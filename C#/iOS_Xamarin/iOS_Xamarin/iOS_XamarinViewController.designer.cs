// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using MonoTouch.Foundation;
using System.CodeDom.Compiler;

namespace iOS_Xamarin
{
	[Register ("iOS_XamarinViewController")]
	partial class iOS_XamarinViewController
	{
		[Outlet]
		MonoTouch.UIKit.UILabel printText { get; set; }

		[Outlet]
		MonoTouch.UIKit.UIButton startButton { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (startButton != null) {
				startButton.Dispose ();
				startButton = null;
			}

			if (printText != null) {
				printText.Dispose ();
				printText = null;
			}
		}
	}
}
