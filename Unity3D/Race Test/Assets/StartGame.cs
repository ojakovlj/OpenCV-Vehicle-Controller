using UnityEngine;
using System.Collections;

public class StartGame : MonoBehaviour {

	public void StartSimulation(int ID){
		Application.LoadLevel (ID);
	}

	public void Leave(){
		Application.Quit();
	}
}
