using UnityEngine;
using System.Collections;
using System;
using System.Runtime.InteropServices;
using System.Threading;

public class SignDetector : MonoBehaviour
{
    static float timer = 0;
    Thread detectionThread;

    //Lets make our calls from the Plugin
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?initialiseDetector@SignDetector@CVDetector@@SAHXZ")]
    private static extern int initialiseDetector();
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?detectSigns@SignDetector@CVDetector@@SAXXZ")]
    private static extern void detectSigns();
    [DllImport("CVDetectorDLL", CallingConvention = CallingConvention.Cdecl, EntryPoint = @"?getFoundSign@SignDetector@CVDetector@@SAHXZ")]
    private static extern int getFoundSign();

	void Start(){
        int retval;
        retval = initialiseDetector(); //initialise detector variables
        Debug.Log("Main returns = " + retval);

        detectionThread = new Thread(new ThreadStart(doDetection));
		detectionThread.Start ();
	}

	void doDetection(){
        int signID;
		while(true){
            detectSigns(); //do the opencv function
            signID = getFoundSign();
            if (signID == 1)
            {
				Debug.Log("Found = " + signID);
				//enable autopilot HSV tracking
				//wait 5 sec so that the autopilot doesnt get (de)activated by the same sign
                Thread.Sleep(5000);             
            }
		}
	}

    void Update(){
        if (Input.GetKeyDown ("space")) {
						Debug.Log ("Killing thread");
						detectionThread.Abort ();
				}
    }

}