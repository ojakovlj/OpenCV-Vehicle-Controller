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
    int side = -2, NOTIF_ID = -1;
    //Lets make our calls from the Plugin
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?initialiseDetector@SignDetector@CVDetector@@SAHXZ")]
    private static extern int initialiseDetector();
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?detectSigns@SignDetector@CVDetector@@SAXXZ")]
    private static extern void detectSigns();
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?getFoundSign@SignDetector@CVDetector@@SAHXZ")]
    private static extern int getFoundSign();
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?doTracking@RoadTracker@CVDetector@@SAXXZ")]
    private static extern int doTracking();
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?getHorizontalAxis@RoadTracker@CVDetector@@SAHXZ")]
    private static extern int getHorizontalAxis();

	void Start(){
        int retval;
        GameObject canvas = GameObject.Find("canvas");
        retval = initialiseDetector(); //initialise detector variables

        uiController = canvas.AddComponent<UIText>();
        if (retval == 0)
        {
            uiController.showText("System init successful.");
            //Debug.Log("System init successful.");
        }
        else
        {
            uiController.showText("System init failed!");
            //Debug.Log("System init failed!");
        }

        detectionThread = new Thread(new ThreadStart(doDetection));
        trackingThread = new Thread(new ThreadStart(doColorTracking));
		detectionThread.Start ();
	}

    int signID;
	void doDetection(){

        NOTIF_ID = 0;
        //Debug.Log("Detector is now working...");
		while(true){
            detectSigns(); //do the opencv function
            signID = getFoundSign();
            if (signID == 1)
            {
                NOTIF_ID = 1;
                Debug.Log("Thread: " + trackingThread.ThreadState);
                //Debug.Log("Found = " + signID);
                //enable autopilot HSV tracking
                if (trackingThread.ThreadState == ThreadState.Unstarted ||
                    trackingThread.ThreadState == ThreadState.Stopped)
                {
                    //Debug.Log("Starting tracker thread");
                    NOTIF_ID = 2;
                    trackingThread.Start();
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
        //Debug.Log("About to do tracking...");
        while(true){
            Thread.Sleep(2);
            doTracking(); //do the opencv function
            side = getHorizontalAxis();
            if (side == -1)
            {
                //Debug.Log("Turning left");
            }
            else if (side == 1)
            {
                //Debug.Log("Turning right");
            }
            else if(side == 0)
            {
               // Debug.Log("Going straight");
            }
        }

    }

    void Update(){
        if (Input.GetKey(KeyCode.X))
            OnApplicationQuit();

        if (side == -1)
        {
            SendKeys.Send("{LEFT}");
        }
        else if (side == 1)
        {
            SendKeys.Send("{RIGHT}");
        }

        if (side != -2)
        {
            CarController controller = GetComponent<CarController>();
            if (controller.speed < 17.0)
                SendKeys.Send("{UP}");
            if (controller.speed > 20.0)
                SendKeys.Send("{DOWN}");
        }

        //Debug.Log("Thread: " + trackingThread.ThreadState);
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

    void OnApplicationQuit()
    {
        Debug.Log("Killing threads");
        uiController.showText("Killing threads");
        detectionThread.Abort();
        trackingThread.Abort();
        side = -2;
    }
}