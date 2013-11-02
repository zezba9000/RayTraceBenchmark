using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using WP8_WinRT.Resources;
using System.IO;

using RayTraceBenchmark;
using System.Windows.Media.Imaging;

namespace WP8_WinRT
{
	public partial class MainPage : PhoneApplicationPage
	{
		// Constructor
		public MainPage()
		{
			InitializeComponent();

		}

		private void Button_Click(object sender, RoutedEventArgs e)
		{
			RayTraceBenchmark.Console.WriteLineCallback = print;
			BenchmarkMain.SaveImageCallback = showImage;
			BenchmarkMain.Start();
		}

		private void print(string value)
		{
			secText.Text = value;
		}

		private void showImage(byte[] data)
		{
			var bitmap = new WriteableBitmap(1280, 720);
			var data2 = BenchmarkMain.ConvertRGBToBGRAInt(data);
			for (int i = 0; i != data2.Length; ++i)
			{
				bitmap.Pixels[i] = data2[i];
			}
			image.Source = bitmap;
		}
	}
}