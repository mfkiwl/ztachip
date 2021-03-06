#include <stdio.h>
#include <stdlib.h>
#include "tensor.h"
#include "bitmap.h"

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER
{
    uint16_t bfType;  //specifies the file type
    uint32_t bfSize;  //specifies the size in bytes of the bitmap file
    uint16_t bfReserved1;  //reserved; must be 0
    uint16_t bfReserved2;  //reserved; must be 0
    uint32_t bOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER
{
    uint32_t biSize;  //specifies the number of bytes required by the struct
    uint32_t biWidth;  //specifies width in pixels
    uint32_t biHeight;  //species height in pixels
    uint16_t biPlanes; //specifies the number of color planes, must be 1
    uint16_t biBitCount; //specifies the number of bit per pixel
    uint32_t biCompression;//spcifies the type of compression
    uint32_t biSizeImage;  //size of image in bytes
    uint32_t biXPelsPerMeter;  //number of pixels per meter in x axis
    uint32_t biYPelsPerMeter;  //number of pixels per meter in y axis
    uint32_t biClrUsed;  //number of colors used by th ebitmap
    uint32_t biClrImportant;  //number of colors that are important
}BITMAPINFOHEADER;
#pragma pack(pop)

// Read 24bit BMP image file

static uint8_t *bmpRead(const char *filename,int *h,int *w) {
   FILE *filePtr;
   BITMAPFILEHEADER bitmapFileHeader;
   uint8_t *bitmapImage;
   int readsize;
   int bisizeImage;
   BITMAPINFOHEADER bitmapInfoHeader;

   //open filename in read binary mode
   filePtr = fopen(filename, "rb");
   if (filePtr == NULL) {
      printf("Image file not found\n");
      return 0;
   }
   //read the bitmap file header
   fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

   //verify that this is a bmp file by check bitmap id
   if (bitmapFileHeader.bfType != 0x4D42) {
      printf("Image file is not 24-bit BMP format\n");
      fclose(filePtr);
      return 0;
   }

   //read the bitmap info header

   fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr); // small edit. forgot to add the closing bracket at sizeof

   *w=bitmapInfoHeader.biWidth;
   *h=bitmapInfoHeader.biHeight;

   //move file point to the begging of bitmap data
   fseek(filePtr, bitmapFileHeader.bOffBits, SEEK_SET);
   bisizeImage=(((bitmapInfoHeader.biWidth+3)/4)*4)*(int)bitmapInfoHeader.biHeight*3;

   //allocate enough memory for the bitmap image data
   bitmapImage = (unsigned char*)malloc(bisizeImage);

   //read in the bitmap image data
   readsize=fread(bitmapImage, 1, bisizeImage, filePtr);
   if(readsize != bisizeImage) {
      printf("Error open image file\n");
      free(bitmapImage);
      fclose(filePtr);
      return 0;
   }
   //close file and return bitmap iamge data
   fclose(filePtr);
   return bitmapImage;
}

// Read 24bit BMP file and put content in a tensor object

ZtaStatus BitmapRead(const char *bmpFile,TENSOR *outputTensor)
{
   uint8_t *pict;
   int bmp_w,bmp_h;
   int bmpActualWidth;
   int r,c;
   int w,h;
   int dx,dy;
   uint8_t *output;

   pict = bmpRead(bmpFile,&bmp_h,&bmp_w);
   if(!pict) {
      return ZtaStatusFail;
   }
   std::vector<int> dim={3,bmp_h,bmp_w};
   outputTensor->Create(TensorDataTypeUint8,TensorFormatSplit,TensorSemanticRGB,dim);
   output=(uint8_t *)outputTensor->GetBuf();

   w=bmp_w;
   h=bmp_h;
   dx=0;
   dy=0;
   bmpActualWidth=((bmp_w*3+3)/4)*4;
   uint8_t red, blue, green;
   for (r = 0; r < h; r++) {
      for (c = 0; c < w; c++) {
         blue = (pict[((bmp_h-1)-(r+dy))*bmpActualWidth+3*(c+dx)+0]);
         green = (pict[((bmp_h-1)-(r+dy))*bmpActualWidth+3*(c+dx)+1]);
         red = (pict[((bmp_h-1)-(r+dy))*bmpActualWidth+3*(c+dx)+2]);
         output[0*w*h+r*w+c] = red;
         output[1*w*h+r*w+c] = green;
         output[2*w*h+r*w+c] = blue;
      }
   }
   free(pict);
   return ZtaStatusOk;
}

// Write tensor object to a file in 24bit BMP format

ZtaStatus BitmapWrite(const char *fileName,TENSOR *image) {
   int r,c;
   // open the file if we can
   FILE *file;
   int w,w2,h;
   uint8_t *image_p;
   uint8_t *pict;
   uint8_t red,green,blue;
   int nChannel;

   file = fopen(fileName, "wb");
   if (!file)
      return ZtaStatusFail;

   w=image->GetDimension(2);
   h=image->GetDimension(1);
   nChannel=image->GetDimension(0);
   w2=((w*3+3)/4)*4;
   image_p=(uint8_t *)image->GetBuf();

   // make the header info
   BITMAPFILEHEADER header;
   BITMAPINFOHEADER infoHeader;

   header.bfType = 0x4D42;
   header.bfReserved1 = 0;
   header.bfReserved2 = 0;
   header.bOffBits = 54;

   infoHeader.biSize = 40;
   infoHeader.biWidth = w;
   infoHeader.biHeight = h;
   infoHeader.biPlanes = 1;
   infoHeader.biBitCount = 24;
   infoHeader.biCompression = 0;
   infoHeader.biSizeImage = w2*h*3;
   infoHeader.biXPelsPerMeter = 0;
   infoHeader.biYPelsPerMeter = 0;
   infoHeader.biClrUsed = 0;
   infoHeader.biClrImportant = 0;

   header.bfSize = infoHeader.biSizeImage + header.bOffBits;

   fwrite(&header,sizeof(header),1,file);
   fwrite(&infoHeader,sizeof(infoHeader),1,file);

   pict=(uint8_t *)malloc(w2*h*3);
   memset(pict,0,w2*h*3);

   for (r = 0; r < h; r++) {
      for (c = 0; c < w; c++) {
         if(nChannel==1) {
            red=green=blue=image_p[r*w+c];
         } else {
            red=image_p[0*w*h+r*w+c];
            green=image_p[1*w*h+r*w+c];
            blue=image_p[2*w*h+r*w+c];
         }
         pict[((h-1)-r)*w2+3*c+0]=blue;
         pict[((h-1)-r)*w2+3*c+1]=green;
         pict[((h-1)-r)*w2+3*c+2]=red;
      }
   }
   fwrite(pict,w2*h*3,1,file);
   free(pict);
   fclose(file);
   return ZtaStatusOk;
}


