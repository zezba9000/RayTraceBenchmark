using System;
using System.Drawing;
using MonoTouch.Foundation;
using MonoTouch.UIKit;

using RayTraceBenchmark;

namespace iOS_Xamarin
{
	public partial class iOS_XamarinViewController : UIViewController
	{
		static bool UserInterfaceIdiomIsPhone {
			get { return UIDevice.CurrentDevice.UserInterfaceIdiom == UIUserInterfaceIdiom.Phone; }
		}

		public iOS_XamarinViewController ()
			: base (UserInterfaceIdiomIsPhone ? "iOS_XamarinViewController_iPhone" : "iOS_XamarinViewController_iPad", null)
		{
		}

		public override void DidReceiveMemoryWarning ()
		{
			// Releases the view if it doesn't have a superview.
			base.DidReceiveMemoryWarning ();
			
			// Release any cached data, images, etc that aren't in use.
		}

		public override void ViewDidLoad ()
		{
			base.ViewDidLoad ();
			
			startButton.TouchUpInside += startClicked;
		}
		
		private void startClicked(object s, EventArgs args)
		{
			RayTraceBenchmark.Console.WriteLineCallback = print;
			BenchmarkMain.SaveImageCallback = drawImage;
			BenchmarkMain.Start();
		}
		
		private void print(string value)
		{
			printText.Text = value;
		}
		
		private void drawImage(byte[] data)
		{
			
		}

		public override bool ShouldAutorotateToInterfaceOrientation (UIInterfaceOrientation toInterfaceOrientation)
		{
			// Return true for supported orientations
			if (UserInterfaceIdiomIsPhone) {
				return (toInterfaceOrientation != UIInterfaceOrientation.PortraitUpsideDown);
			} else {
				return true;
			}
		}
	}
}

