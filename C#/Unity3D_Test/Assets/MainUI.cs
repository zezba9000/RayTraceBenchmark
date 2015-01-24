using UnityEngine;
using System.Collections;

public class MainUI : MonoBehaviour
{
	void OnGUI ()
	{
		if (GUI.Button(new Rect(0, 0, 128, 64), "Start"))
		{
			var pixels = RayTraceBenchmark.BenchmarkMain.Start();
			var texture = new Texture2D(1280, 720, TextureFormat.RGBA32, false);
			texture.SetPixels32(convertRGBToColor32(pixels));
			texture.Apply();
			this.GetComponent<Renderer>().material.mainTexture = texture;
		}

		GUI.Label(new Rect(0, 80, 128, 32), RayTraceBenchmark.BenchmarkMain.TimeToComplete);
	}

	private static Color32[] convertRGBToColor32(byte[] rgb)
	{
		var rgba = new Color32[rgb.Length/3];
		for (int i = 0, i2 = 0; i != rgb.Length; i += 3, i2 += 1)
		{
			rgba[i2].r = rgb[i+2];
			rgba[i2].g = rgb[i+1];
			rgba[i2].b = rgb[i];
			rgba[i2].a = 255;
		}

		return rgba;
	}
}
