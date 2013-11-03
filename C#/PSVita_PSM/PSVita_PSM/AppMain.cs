using System;
using System.Collections.Generic;

using Sce.PlayStation.Core;
using Sce.PlayStation.Core.Environment;
using Sce.PlayStation.Core.Graphics;
using Sce.PlayStation.Core.Input;

using Sce.PlayStation.HighLevel.GameEngine2D;
using Sce.PlayStation.HighLevel.GameEngine2D.Base;
using Sce.PlayStation.Core.Imaging;

namespace PSVita_PSM
{
	public class AppMain
	{
		private static GraphicsContext graphics;
		private static string printValue = "???";
		
		public static void Main (string[] args)
		{
			RayTraceBenchmark.Console.WriteLineCallback = print;
			RayTraceBenchmark.BenchmarkMain.SaveImageCallback = drawImage;
			RayTraceBenchmark.BenchmarkMain.Start();
		
			// Render results
			Director.Initialize();
			
			Scene scene = new Scene();
			scene.Camera.SetViewFromViewport();
			
			var width = Director.Instance.GL.Context.GetViewport().Width;
			var height = Director.Instance.GL.Context.GetViewport().Height;
			
			Image img = new Image(ImageMode.Rgba, new ImageSize(width,height),
			                     new ImageColor(255,0,0,0));
			img.DrawText(printValue, 
			            new ImageColor(255,0,0,255),
			            new Font(FontAlias.System,170,FontStyle.Regular),
			            new ImagePosition(0,150));
			
			Texture2D texture = new Texture2D(width,height,false,
			                                 PixelFormat.Rgba);
			texture.SetPixels(0,img.ToBuffer());
			img.Dispose();                                  
			
			TextureInfo ti = new TextureInfo();
			ti.Texture = texture;
			
			SpriteUV sprite = new SpriteUV();
			sprite.TextureInfo = ti;
			
			sprite.Quad.S = ti.TextureSizef;
			sprite.CenterSprite();
			sprite.Position = scene.Camera.CalcBounds().Center;
			
			scene.AddChild(sprite);
			
			Director.Instance.RunWithScene(scene);
		}
		
		private static void print(string value)
		{
			printValue = value;
		}
		
		private static void drawImage(byte[] data)
		{
			
		}
	}
}
