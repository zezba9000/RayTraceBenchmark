using System;
using System.IO;

namespace ImageBitCompare
{
	class Program
	{
		static void Main(string[] args)
		{
			if (args == null || args.Length != 2)
			{
				Console.WriteLine("Pass in two images for arguments...");
				return;
			}

			try
			{
				using (var file1 = new FileStream(args[0], FileMode.Open, FileAccess.Read))
				using (var file2 = new FileStream(args[1], FileMode.Open, FileAccess.Read))
				{
					if (file1.Length != file2.Length) throw new Exception("Image File sizes do not match!");

					var data1 = new byte[file1.Length];
					var data2 = new byte[file2.Length];
					file1.Read(data1, 0, data1.Length);
					file2.Read(data2, 0, data2.Length);
					for (int i = 0; i != data1.Length; ++i)
					{
						if (data1[i] != data2[i]) throw new Exception(string.Format("ERROR: Image1 bit: {0} does not equal Image2 bit: {1} at index: {2}", data1[i], data2[i], i));
					}
				}

				Console.WriteLine("Success: Files Match!");
			}
			catch (Exception e)
			{
				Console.WriteLine(e.Message);
			}
		}
	}
}
