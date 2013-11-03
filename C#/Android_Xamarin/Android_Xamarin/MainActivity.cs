using System;
using Android.App;
using Android.Content;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using Android.OS;

using RayTraceBenchmark;

namespace Android_Xamarin
{
	[Activity (Label = "Android_Xamarin", MainLauncher = true)]
	public class MainActivity : Activity
	{
		TextView textView;

		protected override void OnCreate (Bundle bundle)
		{
			base.OnCreate (bundle);
			
			SetContentView (Resource.Layout.Main);
			textView = FindViewById<TextView> (Resource.Id.textView1);
			
			Button button = FindViewById<Button> (Resource.Id.button1);
			button.Click += startClicked;
		}
		
		private void startClicked(object s, EventArgs args)
		{
			RayTraceBenchmark.Console.WriteLineCallback = print;
			BenchmarkMain.SaveImageCallback = drawImage;
			BenchmarkMain.Start();
		}
		
		private void print(string value)
		{
			textView.Text = value;
		}
		
		private void drawImage(byte[] data)
		{
			
		}
	}
}


