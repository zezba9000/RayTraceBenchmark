﻿using UnityEngine;
using System.Collections;
using System;
using System.Reflection;
using RayTraceBenchmark;

public class MainUI : MonoBehaviour
{
	private bool didPrint;
	private void PrintMonoVersion()
	{
		if (didPrint) return;
		didPrint = true;

		var type = Type.GetType("Mono.Runtime");
		if (type != null)
		{
			var displayName = type.GetMethod("GetDisplayName", BindingFlags.NonPublic | BindingFlags.Static);
			if (displayName != null) Debug.Log("Mono version: " + displayName.Invoke(null, null));
		}
	}

	void OnGUI ()
	{
		PrintMonoVersion();

		if (GUI.Button(new Rect(0, 0, 128, 64), "Start"))
		{
			var pixels = BenchmarkMain.Start();
			var texture = new Texture2D(Benchmark.Width, Benchmark.Height, TextureFormat.RGBA32, false);
			texture.SetPixels32(convertRGBToColor32(pixels));
			texture.Apply();
			this.GetComponent<Renderer>().material.mainTexture = texture;
		}

		GUI.Label(new Rect(0, 80, 128, 32), BenchmarkMain.TimeToComplete);
	}

	private static Color32[] convertRGBToColor32(byte[] rgb)
	{
		// copy
		var rgba = new Color32[rgb.Length/3];
		for (int i = 0, i2 = 0; i != rgb.Length; i += 3, i2 += 1)
		{
			rgba[i2].r = rgb[i+0];
			rgba[i2].g = rgb[i+1];
			rgba[i2].b = rgb[i+2];
			rgba[i2].a = 255;
		}

		// flip
		for (int y = 0; y != Benchmark.Height; ++y)
		for (int x = 0; x != Benchmark.Width / 2; ++x)
		{
			int i = x + (y * Benchmark.Width);

			int x2 = (Benchmark.Width - 1) - x;
			int i2 = x2 + (y * Benchmark.Width);

			var prev = rgba[i2];
			rgba[i2] = rgba[i];
			rgba[i] = prev;
		}

		return rgba;
	}
}
