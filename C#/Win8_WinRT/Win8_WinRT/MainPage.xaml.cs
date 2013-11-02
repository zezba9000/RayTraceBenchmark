using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

using RayTraceBenchmark;
using Windows.UI.Xaml.Media.Imaging;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace Win8_WinRT
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

		private void Button_Click(object sender, RoutedEventArgs e)
		{
			Console.WriteLineCallback = print;
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
			var stream = bitmap.PixelBuffer.AsStream();
			data = BenchmarkMain.ConvertRGBToBGRA(data);
			stream.Write(data, 0, data.Length);
			image.Source = bitmap;
		}
    }
}
