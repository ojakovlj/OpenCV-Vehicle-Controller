using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using System;

public class UIText : MonoBehaviour{

    public Text text1, text2, text3;
    private List<string> texts;

    void Awake()
    {
		// Find text components to add text to them
        GameObject canvas = GameObject.Find("canvas");
        text1 = canvas.transform.FindChild("Text1").GetComponent<Text>();
        text2 = canvas.transform.FindChild("Text2").GetComponent<Text>();
        text3 = canvas.transform.FindChild("Text3").GetComponent<Text>();
        texts = new List<string>(3);
        texts.Add("");
        texts.Add("");
        texts.Add("");
        text1.text = texts[0];
        text2.text = texts[1];
        text3.text = texts[2];
    }

	/// <summary>
	/// Called by SignDetector, this displays a new message to the screen
	/// </summary>
	/// <param name="text">Text string to display on the UI</param>
    public void showText(string text)
    {
		string tx1 = String.Copy(texts[0]), tx2 = String.Copy(texts[1]);
        texts.Insert(2, tx2);
        texts.Insert(1, tx1);
        texts.Insert(0, String.Copy (text));
        text3.text = texts[2];       
        text2.text = texts[1];
        text1.text = texts[0];
    }
}
