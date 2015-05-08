using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class CarController : MonoBehaviour {

	public float idealRPM = 500f;
	public float maxRPM = 1000f;

	private static float totalRotation;
	public Transform centerOfGravity;

	public WheelCollider wheelFR;
	public WheelCollider wheelFL;
	public WheelCollider wheelRR;
	public WheelCollider wheelRL;

	public GameObject front_left_wheel;
	public GameObject front_right_wheel;
	public GameObject rear_left_wheel;
	public GameObject rear_right_wheel;

    public Text TextField;
    public double speed;

	public float turnRadius = 6f;
	public float torque = 600f;
	public float brakeTorque = 2000f;

	public float AntiRoll = 20000.0f;

	public enum DriveMode { Front, Rear, All };
	public DriveMode driveMode = DriveMode.Rear;

	//public Text speedText;

	void Start() {
		rigidbody.centerOfMass = centerOfGravity.localPosition;
	}

	public float Speed() {
		return wheelRR.radius * Mathf.PI * wheelRR.rpm * 60f / 1000f;
	}

	public float Rpm() {
		return wheelRL.rpm;
	}

	void FixedUpdate () {

		//if(speedText!=null)
			//speedText.text = "Speed: " + Speed().ToString("f0") + " km/h";
        speed =  -(wheelRR.radius * Mathf.PI * wheelRR.rpm * 60f / 1000f);
		TextField.text = "Speed: " +speed + "km/h    RPM: " + wheelRL.rpm;

		float scaledTorque =-Input.GetAxis("Vertical") * torque;

		if(wheelRL.rpm < idealRPM)
			scaledTorque = Mathf.Lerp(scaledTorque/10f, scaledTorque, wheelRL.rpm / idealRPM );
		else 
			scaledTorque = Mathf.Lerp(scaledTorque, 0,  (wheelRL.rpm-idealRPM) / (maxRPM-idealRPM) );

		DoRollBar(wheelFR, wheelFL);
		DoRollBar(wheelRR, wheelRL);

		wheelFR.steerAngle = Input.GetAxis("Horizontal") * turnRadius;
		wheelFL.steerAngle = Input.GetAxis("Horizontal") * turnRadius;

		wheelFR.motorTorque = driveMode==DriveMode.Rear  ? 0 : scaledTorque;
		wheelFL.motorTorque = driveMode==DriveMode.Rear  ? 0 : scaledTorque;
		wheelRR.motorTorque = driveMode==DriveMode.Front ? 0 : scaledTorque;
		wheelRL.motorTorque = driveMode==DriveMode.Front ? 0 : scaledTorque;

		if(Input.GetButton("Fire1")) {
			wheelFR.brakeTorque = brakeTorque;
			wheelFL.brakeTorque = brakeTorque;
			wheelRR.brakeTorque = brakeTorque;
			wheelRL.brakeTorque = brakeTorque;
		}
		else {
			wheelFR.brakeTorque = 0;
			wheelFL.brakeTorque = 0;
			wheelRR.brakeTorque = 0;
			wheelRL.brakeTorque = 0;
		}

		rotateWheels();
	}

	void rotateWheels(){
		float rotationX = wheelFL.rpm / 6f;
		totalRotation = (totalRotation + rotationX) % 360f;
		Quaternion currRot = front_left_wheel.transform.localRotation;

		/*Debug.Log ("Angles: "+ currRot.x*180/Mathf.PI +"," +
		           currRot.y*180/Mathf.PI+","+
		           currRot.z*180/Mathf.PI);*/

		//front_left_wheel.transform.localRotation = Quaternion.Euler(0, 0, 0);
		//front_left_wheel.transform.Rotate(rotationX+currRot.x*360/Mathf.PI, wheelFL.steerAngle, currRot.z);
		front_left_wheel.transform.localRotation = Quaternion.Euler (totalRotation, wheelFL.steerAngle, 0);

		//front_right_wheel.transform.localEulerAngles = new Vector3(currRot.x, wheelFR.steerAngle, currRot.z);
		front_right_wheel.transform.localRotation = Quaternion.Euler (-totalRotation, wheelFR.steerAngle-180f, 0);

		rear_left_wheel.transform.Rotate(rotationX, 0, 0);

		rear_right_wheel.transform.Rotate(-rotationX, 0, 0);
	}

	void DoRollBar(WheelCollider WheelL, WheelCollider WheelR) {
		WheelHit hit;
		float travelL = 1.0f;
		float travelR = 1.0f;
		
		bool groundedL = WheelL.GetGroundHit(out hit);
		if (groundedL)
			travelL = (-WheelL.transform.InverseTransformPoint(hit.point).y - WheelL.radius) / WheelL.suspensionDistance;
		
		bool groundedR = WheelR.GetGroundHit(out hit);
		if (groundedR)
			travelR = (-WheelR.transform.InverseTransformPoint(hit.point).y - WheelR.radius) / WheelR.suspensionDistance;
		
		float antiRollForce = (travelL - travelR) * AntiRoll;
		
		if (groundedL)
			rigidbody.AddForceAtPosition(WheelL.transform.up * -antiRollForce,
			                             WheelL.transform.position); 
		if (groundedR)
			rigidbody.AddForceAtPosition(WheelR.transform.up * antiRollForce,
			                             WheelR.transform.position); 
	}

}
