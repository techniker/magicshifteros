using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MagicshifterGUI
{
    class MagicBitmap
    {
        private int bitPerPixel;
        private int frames;
        private int width, height;
        private SubType subType;
        private int firstChar;
        private int delayMs;
        private const int HEADER_SIZE = 16;

        //private Action<byte [], int, >

        public MagicBitmap(Bitmap bitmap) 
        {
            bitPerPixel = 24;
            frames = 1;
            width = bitmap.Width;
            height = bitmap.Height;
            subType = SubType.Bitmap;
            firstChar = 0;
            delayMs = 0;
        }

        public void WriteFile (string fileName)
        {
            using(FileStream fs = File.Create(fileName))
            {
                byte[] shifterFileData = new byte[HEADER_SIZE];
                int shifterFileSize;
                shifterFileSize = width * height * frames * bitPerPixel;
                // why not multyply widh height
                // write header
	            shifterFileData[0] = 0x23;
	            shifterFileData[1] = (byte)((shifterFileSize & 0xFF0000) >> 16);
	            shifterFileData[2] = (byte)((shifterFileSize & 0xFF00) >> 8);
	            shifterFileData[3] = (byte)((shifterFileSize & 0xFF) >> 0);
	
	            shifterFileData[4] = (byte)(bitPerPixel);
	            shifterFileData[5] = (byte)((frames-1)); // 0 for static images larger for animations and fonts
	            shifterFileData[6] = (byte)(width);
	            shifterFileData[7] = (byte)(height);

	            shifterFileData[8]  =  (byte)(subType);
	            shifterFileData[9]  =  (byte)(firstChar); // >= 1 for fonts/ 0 for animations
	            shifterFileData[10] =  (byte)((delayMs & 0xFF00) >> 8); // 0 for fonts
                shifterFileData[11] = (byte)((delayMs & 0xFF) >> 0);

	            for (var idx = 12; idx < HEADER_SIZE; idx++) {
		            shifterFileData[idx] = 0xFF;	
	            }
                //fs.WriteByte()
            }
        }
            
        /*
	// write image data
	if (bitPerPixel == 24)
	{
		writeGenericPixels(imgData, w, h, function(x, y, c) { SetPixel24Bit(shifterFileData, headerSize, w, h, x, y, c); },
			function(r,g,b) { return Code24Bit(r,g,b,invert); });
	}
	else if (bitPerPixel == 8)
	{
		writeGenericPixels(imgData, w, h, function(x, y, c) { SetPixel8Bit(shifterFileData, headerSize, w, h, x, y, c); }, Code8Bit);
	}
	else if (bitPerPixel == 1)
	{
		writeGenericPixels(imgData, w, h, 
			function(x, y, c) { SetPixel1Bit(shifterFileData, headerSize, w, h, x, y, c); }, 
			function(r,g,b) { return Code1Bit(r,g,b,invert); });
	}

        */
    }

    enum SubType
    {
        font = 0xF0, Bitmap = 0xBA, Text = 0xEE
    }
}
