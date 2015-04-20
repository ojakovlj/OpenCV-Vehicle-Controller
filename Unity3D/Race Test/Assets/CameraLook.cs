using UnityEngine;
using System.Collections;

public class CameraLook : MonoBehaviour {
		
	// Use this for initialization
	void Start () {
		Screen.lockCursor = true;
	}
		
	// Update is called once per frame
	void Update () {
		// Rotation
		float rotLR = Input.GetAxis ("Mouse X") * 3;
		transform.Rotate (0, rotLR, 0);
	}
}
