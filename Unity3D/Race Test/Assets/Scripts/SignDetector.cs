using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;


public class SignDetector : MonoBehaviour
{
    Thread detectionThread, trackingThread;
	UIText uiController;
	Text textLimit;
	static double side = -2.0;
	float maxSpeed = 30.0f;
	int NOTIF_ID = -1;

    // Lets make our calls from the Plugin
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?initialiseDetector@SignDetector@CVDetector@@SAHXZ")]
    private static extern int initialiseDetector();
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?detectSigns@SignDetector@CVDetector@@SAXXZ")]
    private static extern void detectSigns();
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?getFoundSign@SignDetector@CVDetector@@SAHXZ")]
    private static extern int getFoundSign();
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?doTracking@RoadTracker@CVDetector@@SAXXZ")]
    private static extern int doTracking();
	[DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?getHorizontalAxis@RoadTracker@CVDetector@@SANXZ")]
    private static extern double getHorizontalAxis();

	void Start(){
        int retval;
        GameObject canvas = GameObject.Find("canvas");
		textLimit = canvas.transform.FindChild("Limit").GetComponent<Text>();
		textLimit.text = "Speed limit: "+maxSpeed.ToString("0.00")+"km/h";
		
        retval = initialiseDetector(); // Initialise detector variables

        uiController = canvas.AddComponent<UIText>();
        if (retval == 0)
            uiController.showText("System init successful.");
        else
            uiController.showText("System init failed!");          

        detectionThread = new Thread(new ThreadStart(doDetection));
        trackingThread = new Thread(new ThreadStart(doColorTracking));
		// Start scanning for autopilot enable signs
		detectionThread.Start ();
	}

    int signID;
	/// <summary>
	/// Method corresponding to the detectionThread, calls DLL function getFoundSign()
	/// </summary>
	void doDetection(){

        NOTIF_ID = 0;
		while(true){
			Thread.Sleep(20);
            detectSigns(); // Execute the opencv functions
            signID = getFoundSign(); // Acquire function return value
            if (signID == 1) // ENABLE sign is found
            {
				Debug.Log ("Found "+ signID);
				NOTIF_ID = 1;
                Debug.Log("Thread: " + trackingThread.ThreadState);
                // Enable autopilot HSV tracking
                if ((trackingThread.ThreadState == ThreadState.AbortRequested &&
                    trackingThread.ThreadState == ThreadState.Stopped) ||
				    trackingThread.ThreadState == ThreadState.Unstarted)
                {
                    //Debug.Log("Starting tracker thread");
                    NOTIF_ID = 2;
                    trackingThread.Start();
					Debug.Log ("Starting tracking thread");
                    //wait 5 sec so that the autopilot doesnt get (de)activated by the same sign
                    Thread.Sleep(5000);
                }
                /*if (trackingThread.ThreadState == ThreadState.Running)
                {
                    Debug.Log("Killing tracker thread");
                    NOTIF_ID = 3;
                    side = -2;
                    trackingThread.Abort();
                    //wait 5 sec so that the autopilot doesnt get (de)activated by the same sign
                    Thread.Sleep(5000);
                }*/             
            }
		}
	}
    void doColorTracking()
    {        
        while(true){
            Thread.Sleep(20);
            doTracking(); // Do the opencv functions
            side = getHorizontalAxis();
			Debug.Log (getHorizontalAxis());
        }

    }

    void Update(){
        if (Input.GetKey (KeyCode.X)) { // X = kill tracker threads
			Debug.Log("Killing tracker thread");
			uiController.showText("Killing tracker thread");
			trackingThread.Abort();
			trackingThread = new Thread(new ThreadStart(doColorTracking));
			side = -2;
		}
		if (Input.GetKey (KeyCode.Escape)) { // Esc = return to main menu
			Debug.Log("Killing threads");
			uiController.showText("Killing threads");
			trackingThread.Abort();
			detectionThread.Abort();
			UnityEngine.Screen.lockCursor = false;
			UnityEngine.Screen.showCursor = true; 
			UnityEngine.Application.LoadLevel (0);
		}
		if (Input.GetKey (KeyCode.I)) { // I = increase speed by 5 km/h
			if (maxSpeed < 40.0f)
				maxSpeed += 5.0f;
			textLimit.text = "Speed limit: "+maxSpeed.ToString("0.00")+"km/h";
		} 
		if (Input.GetKey (KeyCode.K)){ // D = decrease speed by 5 km/h
			if (maxSpeed > 10.0f)
				maxSpeed -= 5.0f;
			textLimit.text = "Speed limit: "+maxSpeed.ToString("0.00")+"km/h";
		}	
        if (side != -2) // Tracking is not active, do not guard the speed
        {
            CarController controller = GetComponent<CarController>();
            if (controller.speed < maxSpeed-3)
                SendKeys.Send("{UP}");
            if (controller.speed > maxSpeed)
                SendKeys.Send("{DOWN}");
        }

        // Event based UI text output
        switch (NOTIF_ID)
        {
            case 0: uiController.showText("Detector is now working...");
                NOTIF_ID = -1;
                break;
            case 1: uiController.showText("Found sign = " + signID);
                NOTIF_ID = -1;
                break;
            case 2: uiController.showText("Starting tracker thread");
                NOTIF_ID = -1;
                break;
            case 3: uiController.showText("Killing tracker thread");
                NOTIF_ID = -1;
                break;
            default: break;
        }
    }

	/// <summary>
	/// Getter method used by CarController to find out which way to turn the vehicle
	/// </summary>
	/// <returns>The interpolated axis in range [-1, 1] where -1 means turn full left and
	/// +1 means turn full right</returns>
	public static double getAxis(){
		return side;
	}

    void OnApplicationQuit()
    {
        Debug.Log("Killing threads");
        uiController.showText("Killing threads");
        detectionThread.Abort();
        trackingThread.Abort();
        side = -2;
    }
}