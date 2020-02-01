#include "Application.h"
#include "qt_opengl_framework.h"
#include <vector>
#include <cstdlib> /* 亂數相關函數 */
#include <ctime>   /* 時間相關函數 */
#include<algorithm>
#include<math.h>
#include<vector>
using namespace std;
Application::Application()
{

}
Application::~Application()
{

}
//****************************************************************************
//
// * 初始畫面，並顯示Ntust.png圖檔
// 
//============================================================================
void Application::createScene( void )
{
	
	ui_instance = Qt_Opengl_Framework::getInstance();
	
}

//****************************************************************************
//
// * 打開指定圖檔
// 
//============================================================================
void Application::openImage( QString filePath )
{
	mImageSrc.load(filePath);
	mImageDst.load(filePath);

	renew();

	img_data = mImageSrc.bits();
	img_width = mImageSrc.width();
	img_height = mImageSrc.height();

	ui_instance->ui.label->setFixedHeight(img_height);
	ui_instance->ui.label->setFixedWidth(img_width);
}
//****************************************************************************
//
// * 刷新畫面
// 
//============================================================================
void Application::renew()
{
	ui_instance = Qt_Opengl_Framework::getInstance();

	ui_instance->ui.label->clear();
	ui_instance->ui.label->setPixmap(QPixmap::fromImage(mImageDst));
	ui_instance->ui.label->setFixedHeight(img_height);
	ui_instance->ui.label->setFixedWidth(img_width);
	std::cout << "Renew" << std::endl;
}

//****************************************************************************
//
// * 畫面初始化
// 
//============================================================================
void Application::reload()
{
	ui_instance = Qt_Opengl_Framework::getInstance();

	ui_instance->ui.label->clear();
	ui_instance->ui.label->setPixmap(QPixmap::fromImage(mImageSrc));
}

//****************************************************************************
//
// * 儲存圖檔
// 
//============================================================================
void Application::saveImage(QString filePath )
{
	mImageDst.save(filePath);
}

//****************************************************************************
//
// * 將圖檔資料轉換為RGB色彩資料
// 
//============================================================================
unsigned char* Application::To_RGB( void )
{
	unsigned char *rgb = new unsigned char[img_width * img_height * 3];
	int i, j;

	if (! img_data )
		return NULL;

	// Divide out the alpha
	for (i = 0; i < img_height; i++)
	{
		int in_offset = i * img_width * 4;
		int out_offset = i * img_width * 3;

		for (j = 0 ; j < img_width ; j++)
		{
			RGBA_To_RGB(img_data + (in_offset + j*4), rgb + (out_offset + j*3));
		}
	}

	return rgb;
}

void Application::RGBA_To_RGB( unsigned char *rgba, unsigned char *rgb )
{
	const unsigned char	BACKGROUND[3] = { 0, 0, 0 };

	unsigned char  alpha = rgba[3];

	if (alpha == 0)
	{
		rgb[0] = BACKGROUND[0];
		rgb[1] = BACKGROUND[1];
		rgb[2] = BACKGROUND[2];
	}
	else
	{
		float	alpha_scale = (float)255 / (float)alpha;
		int	val;
		int	i;

		for (i = 0 ; i < 3 ; i++)
		{
			val = (int)floor(rgba[i] * alpha_scale);
			if (val < 0)
				rgb[i] = 0;
			else if (val > 255)
				rgb[i] = 255;
			else
				rgb[i] = val;
		}
	}
}
//------------------------Color------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Convert image to grayscale.  Red, green, and blue channels should all 
//  contain grayscale value.  Alpha channel shoould be left unchanged.  Return
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Gray()
{
	unsigned char *rgb = To_RGB();

	for (int i=0; i<img_height; i++)
	{
		for (int j=0; j<img_width; j++)
		{
			int offset_rgb = i*img_width*3+j*3;
			int offset_rgba = i*img_width*4+j*4;
			unsigned char gray = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];

			for (int k=0; k<3; k++)
				img_data[offset_rgba+k] = gray;
			img_data[offset_rgba + aa] = WHITE;
		}
	}
	
	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using uniform quantization.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Quant_Uniform()
{
	unsigned char *rgb = this->To_RGB();
	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			
			rgb[offset_rgb + rr] = (rgb[offset_rgb + rr] / 32) * 32;
		
			rgb[offset_rgb + gg] = (rgb[offset_rgb + gg] / 32) * 32;
		
			rgb[offset_rgb + bb] = (rgb[offset_rgb + bb] / 64 )* 64;
	

			for (int k = 0; k < 3; k++)
				img_data[offset_rgba + k] = rgb[offset_rgb + k];
			img_data[offset_rgba + aa] = WHITE;
		}
	}


	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using populosity quantization.  
//  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////

void Application::Quant_Populosity()
{
	struct  shades
	{
		int Redshade = 0;
		int Greenshade = 0;
		int Blueshade = 0;
		int count = 0;
	}shade[32768];
	unsigned char* rgb = this->To_RGB();
	shades choosed[256];
	vector<shades> memory;
		for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			int red, green, blue;
			red = (rgb[offset_rgb + rr] / 8);

			green = (rgb[offset_rgb + gg] / 8);

			blue = (rgb[offset_rgb + bb] / 8);
			
			shade[red * 1024 + green * 32 + blue].count++;
			shade[red * 1024 + green * 32 + blue].Redshade = red * 8;
			shade[red * 1024 + green * 32 + blue].Blueshade = blue * 8;
			shade[red * 1024 + green * 32 + blue].Greenshade = green * 8;
			
		}
	}
		//sepreate  0
		for (int x = 0; x < 32768; x++)
		{
			if (shade[x].count > 0)
				memory.push_back(shade[x]);
		}
		//sort
		for (int i = 0; i < memory.size(); ++i) {
			for (int j = 0; j < i; ++j) {
				if (memory[j].count < memory[i].count) {
					shades temp = memory[j];
					memory[j] = memory[i];
					memory[i] = temp;
				}
			}
		}
		int size = 0;
		if (memory.size() >= 256)
		{
			for (int k = 0; k < 256; k++)
			{
				choosed[k] = memory[k];
				size = 256;
			}
		}
		else
		{

			for (int k = 0; k < memory.size(); k++)
			{
				choosed[k] = memory[k];
				size = memory.size();
			}
		}
	
	
	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			int min = sqrt((pow((int)(choosed[0].Redshade - rgb[offset_rgb + rr])  , 2) + pow((int)(choosed[0].Greenshade- rgb[offset_rgb + gg])  , 2) + pow((int)(choosed[0].Blueshade - rgb[offset_rgb + bb])  , 2)));
			int minorRed = choosed[0].Redshade;
			int minorBlue = choosed[0].Blueshade;
			int minorGreen = choosed[0].Greenshade;
			for (int u = 1; u < size; u++)
			{
				int sqrtCom = sqrt(pow((int)(choosed[u].Redshade - rgb[offset_rgb + rr]  ), 2) + pow((int)(choosed[u].Greenshade - rgb[offset_rgb + gg])  , 2) + pow((int)(choosed[u].Blueshade - rgb[offset_rgb + bb])  , 2));

				if (min > sqrtCom)
				{
					min = sqrtCom;
					minorRed  = choosed[u].Redshade;
					minorBlue = choosed[u].Blueshade;
					minorGreen = choosed[u].Greenshade;
				}
			}
			rgb[offset_rgb + rr] = minorRed;
			rgb[offset_rgb + gg] = minorGreen;
			rgb[offset_rgb + bb] = minorBlue;

			for (int k = 0; k < 3; k++)
				img_data[offset_rgba + k] = rgb[offset_rgb + k];
			img_data[offset_rgba + aa] = WHITE;
		}
	}


	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
//------------------------Dithering------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Dither the image using a threshold of 1/2.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Threshold()
{
	unsigned char* rgb = To_RGB();

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			unsigned char gray = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];
			if (gray > 128)gray = 255;
			else gray = 0;
			for (int k = 0; k < 3; k++)
				img_data[offset_rgba + k] = gray;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Dither image using random dithering.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Random()
{
	srand(5);

	unsigned char* rgb = To_RGB();
	int gray[256];
	for (int i = 0; i < 256; i++) gray[i] = 0;
	double u;
	unsigned long long int real = 0;

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			gray[unsigned char(0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb])]++;
		}
	}
	


	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int x = rand() % (51);
			if (rand() % 2 == 0)x = -1 * x;
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			int grayyy= 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb] + x;
			if (grayyy > 255)grayyy = 255;
			else if (grayyy < 0)grayyy = 0;
			unsigned char gray = grayyy;
			if (gray > 127)gray = 255;
			else gray = 0;
			for (int k = 0; k < 3; k++)
				img_data[offset_rgba + k] = gray;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform Floyd-Steinberg dithering on the image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_FS()
{
	unsigned char* rgb = To_RGB();
	
	

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{	
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			int errorColor = 0;
			int err = 0;
			unsigned char gray = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];
			int used = 0;
			if (gray > 128)
			{
				errorColor = (int)gray - 255;
				gray = 255;

			}
			else
			{
				errorColor = gray;
				gray = 0;

			}
			for (int k = 0; k < 3; k++)
				img_data[offset_rgba + k] = gray;
			if (j != img_width - 1)
			{
				
					for (int k = 0; k < 3; k++)
					{
						int test = 0;
						err = (int)rgb[i * img_width * 3 + (j + 1) * 3 + k] + ((errorColor * 7) / 16);
						rgb[i * img_width * 3 + (j + 1) * 3 + k] += ((errorColor * 7) / 16);
						if (err > 255)rgb[i * img_width * 3 + (j + 1) * 3 + k] = 255;
						if (err < 0)rgb[(i)* img_width * 3 + (j + 1) * 3 + k] = 0;
					
					}		
			}

			if (i != img_height - 1)
			{
				for (int k = 0; k < 3; k++)
				{
					err = (int)rgb[(i + 1) * img_width * 3 + (j) * 3 + k] + ((errorColor * 5) / 16);
					rgb[(i + 1) * img_width * 3 + (j) * 3 + k] += ((errorColor * 5) / 16);
					if (err > 255)rgb[(i + 1) * img_width * 3 + (j) * 3 + k] = 255;
					if (err < 0)rgb[(i + 1) * img_width * 3 + (j) * 3 + k] = 0;
				}
				
				
			}
			if (i != img_height - 1 && j != 0)
			{
				for (int k = 0; k < 3; k++)
				{
					err = (int)rgb[(i + 1) * img_width * 3 + (j - 1) * 3 + k] + ((errorColor * 3 )/ 16);
					rgb[(i + 1) * img_width * 3 + (j - 1) * 3 + k] += ((errorColor * 3 )/ 16);
					if (err > 255)rgb[(i + 1) * img_width * 3 + (j - 1) * 3 + k] =255;
					if (err < 0)rgb[(i + 1) * img_width * 3 + (j - 1) * 3 + k] = 0;
				}
			}

				if (i != img_height - 1 && j != img_width - 1)
				{
					for (int k = 0; k < 3; k++)
					{
						err = (int)rgb[(i + 1) * img_width * 3 + (j + 1) * 3 + k] + (errorColor  / 16);
						rgb[(i + 1) * img_width * 3 + (j + 1) * 3 + k] += (errorColor / 16);
						if (err > 255)rgb[(i + 1) * img_width * 3 + (j + 1) * 3 + k] = 255;
						if (err < 0)rgb[(i + 1) * img_width * 3 + (j + 1) * 3 + k] = 0;
					}
					
					
					}
			
		
		
			img_data[offset_rgba + aa] = WHITE;
		}
	}


	delete[] rgb;

	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Dither the image while conserving the average brightness.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Bright()
{
	unsigned char* rgb = To_RGB();
	int gray[256];
	int white = 0;
	for (int i = 0; i < 256; i++) gray[i] = 0;
	double u;
	int mid = 0;
	unsigned long long int real = 0;

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			gray[int(0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb])]++;
		}
	}
	unsigned long long int half = img_height * img_width;
	for (int i = 0; i < 256; i++)
	{
		real = real + gray[i] * i;
	}
	double thepoint = real / half;
	thepoint = thepoint / 256;
	double theusedpoint = half * thepoint;
	for (int q = 255; q >= 0; q--)
	{

		for (int i = 0; i < img_height; i++)
		{
			for (int j = 0; j < img_width; j++)
			{
				int offset_rgb = i * img_width * 3 + j * 3;
				int offset_rgba = i * img_width * 4 + j * 4;
				unsigned char grayy = 0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb];
				if (grayy > q)continue;
				if (white == 1)grayy = 0;
				else if (grayy == q)
				{
					grayy = 255;
					mid++;
				}

				if (mid >= theusedpoint)white = 1;

				for (int k = 0; k < 3; k++)
					img_data[offset_rgba + k] = grayy;
				img_data[offset_rgba + aa] = WHITE;
			}
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform clustered differing of the image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Cluster()
{
	unsigned char* rgb = To_RGB();
	double mask[4][4] =
	{
		0.7059,0.3529,0.5882,0.2353,
		0.0588,0.9412,0.8235,0.4118,
		0.4706,0.7647,0.8824,0.1176,
		0.1765,0.5294,0.2941,0.6471
	};
	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			double gray = (0.3 * rgb[offset_rgb + rr] + 0.59 * rgb[offset_rgb + gg] + 0.11 * rgb[offset_rgb + bb])/255;
		
			if (gray >=mask[i % 4][j % 4])gray = 255;
			else gray = 0;
			for (int k = 0; k < 3; k++)
				img_data[offset_rgba + k] = gray;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Convert the image to an 8 bit image using Floyd-Steinberg dithering over
//  a uniform quantization - the same quantization as in Quant_Uniform.
//  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Dither_Color()
{
	unsigned char* rgb = To_RGB();

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
		
		
			int errorColor[3];
			for (int k = 0; k < 3; k++)errorColor[k] = 0;
				errorColor[2] = rgb[offset_rgb + rr] - (rgb[offset_rgb + rr] / 36) * 36;
				rgb[offset_rgb + rr] = (rgb[offset_rgb + rr] / 36) * 36;
				errorColor[1] = rgb[offset_rgb + gg] - (rgb[offset_rgb + gg] / 36) * 36;
				rgb[offset_rgb + gg] = (rgb[offset_rgb + gg] / 36) * 36;

				errorColor[0] = rgb[offset_rgb + bb] - (rgb[offset_rgb + bb] / 85) * 85;
				rgb[offset_rgb + bb] = (rgb[offset_rgb + bb] / 85) * 85;

			
			
			
				
			if (j != img_width - 1)
			{
				for (int k = 0; k < 3; k++)
				{
					int red = 0;
					
					red= rgb[(i)* img_width * 3 + (j + 1) * 3 + k] + (errorColor[k] * 7 / 16);
					rgb[(i)* img_width * 3 + (j + 1) * 3 + k] += (errorColor[k] * 7 / 16);
					if (red < 0)rgb[(i ) * img_width * 3 + (j + 1) * 3 + k] = 0;
					if (red > 255 )rgb[(i)* img_width * 3 + (j + 1) * 3 + k] = 255;
				}
			}
			if (i != img_height - 1)
			{
				for (int k = 0; k < 3; k++)
				{
					int err = 0;
					err = rgb[(i + 1)* img_width * 3 + (j) * 3 + k] + (errorColor[k] * 5 / 16);
					rgb[(i + 1) * img_width * 3 + (j) * 3 + k] +=  (errorColor[k] * 5 / 16);
					if (err < 0)rgb[(i + 1)* img_width * 3 + (j) * 3 + k] = 0;
					if (err > 255)rgb[( i + 1)* img_width * 3 + (j) * 3 + k] = 255;
					
				}
			}
			if (i != img_height - 1 && j != 0)
			{
				for (int k = 0; k < 3; k++)
				{
					int err = 0;
					err = rgb[(i + 1) * img_width * 3 + (j - 1) * 3 + k] + (errorColor[k] * 3 / 16);
					rgb[(i + 1) * img_width * 3 + (j -1) * 3 + k] += (errorColor[k] * 3 / 16);
					if (err < 0)rgb[(i + 1) * img_width * 3 + (j-1) * 3 + k] = 0;
					if (err > 255)rgb[(i + 1) * img_width * 3 + (j-1) * 3 + k] = 255;
				}
			}

			if (i != img_height - 1 && j != img_width - 1)
			{
				for (int k = 0; k < 3; k++)
				{
					int err = 0;
					err = rgb[(i + 1) * img_width * 3 + (j + 1) * 3 + k] + (errorColor[k] * 1 / 16);
					rgb[(i + 1) * img_width * 3 + (j + 1) * 3 + k] += (errorColor[k] * 1 / 16);
					if (err < 0)rgb[(i + 1) * img_width * 3 + (j + 1) * 3 + k] = 0;
					if (err > 255)rgb[(i + 1) * img_width * 3 + (j + 1) * 3 + k] = 255;
				}
			}

			

			for (int k = 0; k < 3; k++)
				img_data[offset_rgba + k] = rgb[offset_rgb + k];
			img_data[offset_rgba + aa] = WHITE;
		}
	}


	delete[] rgb;

	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}

//------------------------Filter------------------------

///////////////////////////////////////////////////////////////////////////////
//
//     Filtering the img_data array by the filter from the parameters
//
///////////////////////////////////////////////////////////////////////////////
void Application::filtering( double filter[][5] )
{
	unsigned char *rgb = this->To_RGB();



	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

void Application::filtering( double **filter, int n )
{
	unsigned char *rgb = this->To_RGB();



	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 box filter on this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Box()
{
	unsigned char* rgb = To_RGB();

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			unsigned char filterRed[5][5] = { 0 };
			unsigned char filterBlue[5][5] = { 0 };
			unsigned char filterGreen[5][5] = { 0 };
			int sumRed = 0;
			int  sumBlue = 0;
			int  sumGreen = 0;
			int tenpy= i - 2;
			int tenpx = j - 2;
			for (int y = tenpy;  y < i + 3 ; y++)
			{
				for (int x = tenpx; x < j + 3; x++)
				{
					if (y < 0 || x < 0 || y >= img_height || x >= img_width)
					{
						filterRed[y - (i - 2)][x - (j - 2)] = 0;
						filterBlue[y - (i - 2)][x - (j - 2)] = 0;
						filterGreen[y - (i - 2)][x - (j - 2)] = 0;
					}
					else
					{
						filterRed[y - (i - 2)][x - (j - 2)] = rgb[y * img_width * 3 + x * 3 +rr];
						filterBlue[y - (i - 2)][x - (j - 2)] = rgb[y * img_width * 3 + x * 3 + bb];
						filterGreen[y - (i - 2)][x - (j - 2)] = rgb[y * img_width * 3 + x * 3 + gg];
					}
				
				}
			}
			for (int q = 0; q < 5; q++)
			{
				for (int w = 0; w < 5; w++)
				{
					sumRed += filterRed[q][w];
					sumBlue += filterBlue[q][w];
					sumGreen += filterGreen[q][w];
				}
			}
			sumRed = sumRed / 25;
			sumBlue = sumBlue / 25;
			sumGreen = sumGreen / 25;
			
			img_data[offset_rgba+bb] = sumBlue;
			img_data[offset_rgba + gg] = sumGreen;
			img_data[offset_rgba + rr] = sumRed;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 Bartlett filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Bartlett()
{
	unsigned char* rgb = To_RGB();

	int filter[5][5] =
	{
		1,2,3,2,1,
		2,4,6,4,2,
		3,6,9,6,3,
		2,4,6,4,2,
		1,2,3,2,1
	};

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			int filterRed[5][5] = { 0 };
			int filterBlue[5][5] = { 0 };
			int  filterGreen[5][5] = { 0 };
			int sumRed = 0;
			int  sumBlue = 0;
			int  sumGreen = 0;
			int tenpy = i - 2;
			int tenpx = j - 2;
			for (int y = tenpy; y < i + 3; y++)
			{
				for (int x = tenpx; x < j + 3; x++)
				{
					if (y < 0 || x < 0 || y >= img_height || x >= img_width)
					{
						filterRed[y - (i - 2)][x - (j - 2)] = 0;
						filterBlue[y - (i - 2)][x - (j - 2)] = 0;
						filterGreen[y - (i - 2)][x - (j - 2)] = 0;
					}
					else
					{
						filterRed[y - (i - 2)][x - (j - 2)] =  filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + rr];
						filterBlue[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] *  rgb[y * img_width * 3 + x * 3 + bb];
						filterGreen[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] *  rgb[y * img_width * 3 + x * 3 + gg];
					}

				}
			}
			for (int q = 0; q < 5; q++)
			{
				for (int w = 0; w < 5; w++)
				{
					sumRed += filterRed[q][w];
					sumBlue += filterBlue[q][w];
					sumGreen += filterGreen[q][w];
				}
			}
			sumRed = sumRed / 81;
			sumBlue = sumBlue / 81;
			sumGreen = sumGreen / 81;

			img_data[offset_rgba + bb] = sumBlue;
			img_data[offset_rgba + gg] = sumGreen;
			img_data[offset_rgba + rr] = sumRed;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 Gaussian filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Gaussian()
{
	unsigned char* rgb = To_RGB();

	int filter[5][5] =
	{
		1,4,6,4,1,
		4,16,24,16,4,
		6,24,36,24,6,
		4,16,24,16,4,
		1,4,6,4,1
	};

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			int filterRed[5][5] = { 0 };
			int filterBlue[5][5] = { 0 };
			int  filterGreen[5][5] = { 0 };
			int sumRed = 0;
			int  sumBlue = 0;
			int  sumGreen = 0;
			int tenpy = i - 2;
			int tenpx = j - 2;
			for (int y = tenpy; y < i + 3; y++)
			{
				for (int x = tenpx; x < j + 3; x++)
				{
					if (y < 0 || x < 0 || y >= img_height || x >= img_width)
					{
						filterRed[y - (i - 2)][x - (j - 2)] = 0;
						filterBlue[y - (i - 2)][x - (j - 2)] = 0;
						filterGreen[y - (i - 2)][x - (j - 2)] = 0;
					}
					else
					{
						filterRed[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + rr];
						filterBlue[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + bb];
						filterGreen[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + gg];
					}

				}
			}
			for (int q = 0; q < 5; q++)
			{
				for (int w = 0; w < 5; w++)
				{
					sumRed += filterRed[q][w];
					sumBlue += filterBlue[q][w];
					sumGreen += filterGreen[q][w];
				}
			}
			sumRed = sumRed / 256;
			sumBlue = sumBlue / 256;
			sumGreen = sumGreen / 256;

			img_data[offset_rgba + bb] = sumBlue;
			img_data[offset_rgba + gg] = sumGreen;
			img_data[offset_rgba + rr] = sumRed;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform NxN Gaussian filter on this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
int  compose(int n, int k)
{
	unsigned long long int up = 1 , down = 1;
	for (int i = k + 1; i <= n; i++)
	{
		up = up * i;
	}
	for (int i = 1; i <= (n - k); i++)
	{
		down = down * i;
	}
	return (up / down);
}
unsigned long long int powunint(int x, int y)
{
	unsigned long long int test = 1;
	for (int i = 0; i < y; i++)
	{
		test = x * test;
	}
	return test;
}
void Application::Filter_Gaussian_N( unsigned int N )
{
	if (N % 2 == 0)N++;
	unsigned char* rgb = To_RGB();
	vector <vector<double>>filter;
	filter.resize(N);
	for (int i = 0; i < N; i++)
		filter[i].resize(N);
	for (int i = 0; i < N; i++)
	{
		filter[0][i] = compose(N - 1, i);
	}
	for (int i = 1; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			filter[i][j] = filter[0][i] * filter[0][j];
		}
	}
		
	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
		
			vector <vector<double>>filterRed, filterBlue, filterGreen;
			filterRed.resize(N);
			filterBlue.resize(N);
			filterGreen.resize(N);
			for (int i = 0; i < N; i++)
			{
				filterRed[i].resize(N);
				filterBlue[i].resize(N);
				filterGreen[i].resize(N);
			}
				
			
			double sumRed = 0;
			double   sumBlue = 0;
			double   sumGreen = 0;
			int debugy =  ((N - 1) / 2);
			

			int testtt = 0,m=0,count=0;
			for (int y = i -  debugy; y < i + debugy +1; y++)
			{
				for (int x = j - debugy; x < j + debugy+1; x++)
				{
					count++;
					if (y < 0 || x < 0 || y >= img_height || x >= img_width)
					{
						filterRed[y - i + debugy][x - j + debugy] = 0;
						filterBlue[y - i + debugy][x - j + debugy] = 0;
						filterGreen[y - i + debugy][x - j + debugy] = 0;
						testtt = 0;
						if (y ==4)m = 2;
					}
					else
					{
						 m = powunint(2, (2 * N - 2));
						filterRed[y - i + debugy][x - j + debugy] = filter[y - i + debugy][x - j + debugy] * rgb[y * img_width * 3 + x * 3 + rr]/ pow(2,(2*N-2));
						filterBlue[y - i + debugy][x - j + debugy] = filter[y - i + debugy][x - j + debugy] * rgb[y * img_width * 3 + x * 3 + bb] / pow(2, (2 * N - 2));
						filterGreen[y - i + debugy][x - j + debugy] = filter[y - i + debugy][x - j + debugy] * rgb[y * img_width * 3 + x * 3 + gg] / pow(2, (2 * N - 2));
						testtt = 1;
					
					}

				}
			}
		
			for (int q = 0; q < N; q++)
			{
				for (int w = 0; w < N; w++)
				{
					sumRed += filterRed[q][w];
					sumBlue += filterBlue[q][w];
					sumGreen += filterGreen[q][w];
				}
			}
			int t = count;
			

			img_data[offset_rgba + bb] = sumBlue;
			img_data[offset_rgba + gg] = sumGreen;
			img_data[offset_rgba + rr] = sumRed;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();

}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform 5x5 edge detect (high pass) filter on this image.  Return 
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Edge()
{
	unsigned char* rgb = To_RGB();

	int filter[5][5] =
	{
		-1,  -4,  -6,  -4, -1,
		-4, -16, -24, -16, -4,
		-6, -24, 220, -24, -6,
		-4, -16, -24, -16, -4,
		-1,  -4,  -6,  -4, -1
	};

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			float filterRed[5][5] = { 0 };
			float filterBlue[5][5] = { 0 };
			float  filterGreen[5][5] = { 0 };
			float sumRed = 0;
			float  sumBlue = 0;
			float  sumGreen = 0;
			int tenpy = i - 2;
			int tenpx = j - 2;
			for (int y = tenpy; y < i + 3; y++)
			{
				for (int x = tenpx; x < j + 3; x++)
				{
					if (y < 0 || x < 0 || y >= img_height || x >= img_width)
					{
						filterRed[y - (i - 2)][x - (j - 2)] = 0;
						filterBlue[y - (i - 2)][x - (j - 2)] = 0;
						filterGreen[y - (i - 2)][x - (j - 2)] = 0;
					}
					else
					{
						filterRed[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + rr];
						filterBlue[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + bb];
						filterGreen[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + gg];
					}

				}
			}
			for (int q = 0; q < 5; q++)
			{
				for (int w = 0; w < 5; w++)
				{
					sumRed += filterRed[q][w];
					sumBlue += filterBlue[q][w];
					sumGreen += filterGreen[q][w];
				}
			}
			sumRed = sumRed / 256;
			sumBlue = sumBlue / 256;
			sumGreen = sumGreen / 256;
			if (sumRed < 0)sumRed = 0;
			if (sumBlue < 0)sumBlue = 0;
			if (sumGreen < 0)sumGreen = 0;
			img_data[offset_rgba + bb] = sumBlue;
			img_data[offset_rgba + gg] = sumGreen;
			img_data[offset_rgba + rr] = sumRed;
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Perform a 5x5 enhancement filter to this image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Filter_Enhance()
{
	unsigned char* rgb = To_RGB();

	int filter[5][5] =
	{
		-1,  -4,  -6,  -4, -1,
		-4, -16, -24, -16, -4,
		-6, -24, 220, -24, -6,
		-4, -16, -24, -16, -4,
		-1,  -4,  -6,  -4, -1
	};

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			int filterRed[5][5] = { 0 };
			int filterBlue[5][5] = { 0 };
			int  filterGreen[5][5] = { 0 };
			int sumRed = 0;
			int  sumBlue = 0;
			int  sumGreen = 0;
			int tenpy = i - 2;
			int tenpx = j - 2;
			for (int y = tenpy; y < i + 3; y++)
			{
				for (int x = tenpx; x < j + 3; x++)
				{
					if (y < 0 || x < 0 || y >= img_height || x >= img_width)
					{
						filterRed[y - (i - 2)][x - (j - 2)] = 0;
						filterBlue[y - (i - 2)][x - (j - 2)] = 0;
						filterGreen[y - (i - 2)][x - (j - 2)] = 0;
					}
					else
					{
						filterRed[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + rr];
						filterBlue[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + bb];
						filterGreen[y - (i - 2)][x - (j - 2)] = filter[y - (i - 2)][x - (j - 2)] * rgb[y * img_width * 3 + x * 3 + gg];
					}

				}
			}
			for (int q = 0; q < 5; q++)
			{
				for (int w = 0; w < 5; w++)
				{
					sumRed += filterRed[q][w];
					sumBlue += filterBlue[q][w];
					sumGreen += filterGreen[q][w];
				}
			}
			sumRed = sumRed / 256;
			sumBlue = sumBlue / 256;
			sumGreen = sumGreen / 256;
			if (sumRed < 0)sumRed = 0;
			if (sumBlue < 0)sumBlue = 0;
			if (sumGreen < 0)sumGreen = 0;
			int testt = 0;
			if (sumBlue + rgb[offset_rgb + bb] > 255)
			{
				img_data[offset_rgba + bb] = 255;
			
			}
			else
			{
				testt = 2;
				img_data[offset_rgba + bb] = sumBlue + rgb[offset_rgb + bb];
				testt = rgb[offset_rgb + bb]+1;
			}
			if (sumGreen + rgb[offset_rgb + gg] > 255)
			{
				img_data[offset_rgba + gg] = 255;
			
			}
			else
			{
				img_data[offset_rgba + gg] = sumGreen + rgb[offset_rgb + gg];
	
			}

			if (sumRed + rgb[offset_rgb + rr] > 255)
			{
				img_data[offset_rgba + rr] = 255;
			}
			else
			{
				img_data[offset_rgba + rr] = sumRed + rgb[offset_rgb + rr];
			}

		
			img_data[offset_rgba + aa] = WHITE;
		}
	}

	delete[] rgb;
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32);
	renew();
}

//------------------------Size------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  Halve the dimensions of this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Half_Size()
{
	int new_img_height = img_height / 2, new_img_width = img_width / 2;
	unsigned char* rgb = To_RGB();
	unsigned char* newrgb = To_RGB();
	int filter[3][3] =
	{
		1,2,1,   
		2,4,2,
		1,2,1
	};

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			int filterRed[3][3] = { 0 };
			int filterBlue[3][3] = { 0 };
			int  filterGreen[3][3] = { 0 };
			int sumRed = 0;
			int  sumBlue = 0;
			int  sumGreen = 0;
			int tenpy = i - 1;
			int tenpx = j - 1;
			for (int y = tenpy; y < i + 2; y++)
			{
				for (int x = tenpx; x < j + 2; x++)
				{
					if (y < 0 || x < 0 || y >= img_height || x >= img_width)
					{
						filterRed[y - (i - 1)][x - (j - 1)] = 0;
						filterBlue[y - (i - 1)][x - (j - 1)] = 0;
						filterGreen[y - (i - 1)][x - (j - 1)] = 0;
					}
					else
					{
						filterRed[y - (i - 1)][x - (j - 1)] = filter[y - (i - 1)][x - (j - 1)] * rgb[y * img_width * 3 + x * 3 + rr];
						filterBlue[y - (i - 1)][x - (j - 1)] = filter[y - (i - 1)][x - (j - 1)] * rgb[y * img_width * 3 + x * 3 + bb];
						filterGreen[y - (i - 1)][x - (j - 1)] = filter[y - (i - 1)][x - (j - 1)] * rgb[y * img_width * 3 + x * 3 + gg];
					}

				}
			}
			for (int q = 0; q < 3; q++)
			{
				for (int w = 0; w < 3; w++)
				{
					sumRed += filterRed[q][w];
					sumBlue += filterBlue[q][w];
					sumGreen += filterGreen[q][w];
				}
			}
			sumRed = sumRed / 16;
			sumBlue = sumBlue / 16;
			sumGreen = sumGreen / 16;

			rgb[offset_rgb + bb] = sumBlue;
			rgb[offset_rgb + gg] = sumGreen;
			rgb[offset_rgb + rr] = sumRed;
		}
	}
	for(int i = 0; i < new_img_height; i++)
		for (int j = 0; j < new_img_width; j++)
		{
			int new_offset_rgb = i * new_img_width * 3 + j * 3;
			int new_offset_rgba = i * new_img_width * 4 + j * 4;
			int offset_rgb = (i * img_width * 3 + j * 3) * 2;
			int offset_rgba = (i * img_width * 4 + j * 4) * 2;
			img_data[new_offset_rgba + rr] = rgb[offset_rgb + rr];
			img_data[new_offset_rgba + bb] = rgb[offset_rgb + bb];
			img_data[new_offset_rgba + gg] = rgb[offset_rgb + gg];
			img_data[new_offset_rgba + aa] = img_data[offset_rgba + aa];

		}
	delete[] rgb;
	mImageDst = QImage(img_data, new_img_width, new_img_height, QImage::Format_ARGB32 );
	img_height = new_img_height;
	img_width = new_img_width;
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  Double the dimensions of this image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Double_Size()
{
	int new_img_height = img_height * 2, new_img_width = img_width * 2;
	unsigned char* rgb = To_RGB();
	unsigned char* newrgb = To_RGB();
	img_data = new unsigned char[new_img_width * 2 * new_img_height * 2 * 4];
	
	//偶偶
	int filterEvenEven[3][3] =
	{
		1,2,1,
		2,4,2,
		1,2,1
	};
	//奇奇
	int filterOddOdd[4][4] =
	{
		1,3,3,1,
		3,9,9,3,
		3,9,9,3,
		1,3,3,1	
	};
	//偶奇
	int filterEvenOdd[4][3] =
	{
		1,2,1,
		3,6,3,
		3,6,3,
		1,2,1
	};
	//奇偶
	int filterOddEven[3][4] =
	{
		1,3,3,1,
		2,6,6,2,
		1,3,3,1
	};
	int filter[4][4] =
	{
		1,3,3,1,
		3,9,9,3,
		3,9,9,3,
		1,3,3,1
	};

	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			int offset_rgb = i * img_width * 3 + j * 3;
			int offset_rgba = i * img_width * 4 + j * 4;
			int filterRed[4][4] = { 0 };
			int filterBlue[4][4] = { 0 };
			int  filterGreen[4][4] = { 0 };
			int sumRed = 0;
			int  sumBlue = 0;
			int  sumGreen = 0;
			int tenpy = i - 1;
			int tenpx = j - 1;
			for (int y = tenpy; y < i + 3; y++)
			{
				for (int x = tenpx; x < j + 3; x++)
				{
					if (y < 0 || x < 0 || y >= img_height || x >= img_width)
					{
						filterRed[y - (i - 1)][x - (j - 1)] = 0;
						filterBlue[y - (i - 1)][x - (j - 1)] = 0;
						filterGreen[y - (i - 1)][x - (j - 1)] = 0;
					}
					else
					{
						filterRed[y - (i - 1)][x - (j - 1)] = filter[y - (i - 1)][x - (j - 1)] * rgb[y * img_width * 3 + x * 3 + rr];
						filterBlue[y - (i - 1)][x - (j - 1)] = filter[y - (i - 1)][x - (j - 1)] * rgb[y * img_width * 3 + x * 3 + bb];
						filterGreen[y - (i - 1)][x - (j - 1)] = filter[y - (i - 1)][x - (j - 1)] * rgb[y * img_width * 3 + x * 3 + gg];
					}

				}
			}
			for (int q = 0; q < 4; q++)
			{
				for (int w = 0; w < 4; w++)
				{
					sumRed += filterRed[q][w];
					sumBlue += filterBlue[q][w];
					sumGreen += filterGreen[q][w];
				}
			}
			sumRed = sumRed / 64;
			sumBlue = sumBlue / 64;
			sumGreen = sumGreen / 64;

			rgb[offset_rgb + bb] = sumBlue;
			rgb[offset_rgb + gg] = sumGreen;
			rgb[offset_rgb + rr] = sumRed;
		}
	}
	for (int i = 0; i < new_img_height; i++)
		for (int j = 0; j < new_img_width; j++)
		{
			int new_offset_rgb = ( i / 4 ) * new_img_width * 3 + (j / 2) * 3;
			int new_offset_rgba = (i )* new_img_width * 4 + (j) * 4;
			int offset_rgb = ((i / 2) * img_width * 3 + (j / 2) * 3) ;
			int offset_rgba = ((i / 2) * img_width * 4 + (j / 2) * 4) ;
			img_data[new_offset_rgba + rr] = rgb[new_offset_rgb  + rr];
			img_data[new_offset_rgba + bb] = rgb[new_offset_rgb  + bb];
			img_data[new_offset_rgba + gg] = rgb[new_offset_rgb  + gg];
			img_data[new_offset_rgba + aa] = WHITE;

		}
	delete[] rgb;
	mImageDst = QImage(img_data, new_img_width, new_img_height, QImage::Format_ARGB32);
	img_height = new_img_height;
	img_width = new_img_width;
	renew();
}
///////////////////////////////////////////////////////////////////////////////
//
//  resample_src for resize and rotate
//
///////////////////////////////////////////////////////////////////////////////
void Application::resample_src(int u, int v, float ww, unsigned char* rgba)
{

}

///////////////////////////////////////////////////////////////////////////////
//
//  Scale the image dimensions by the given factor.  The given factor is 
//	assumed to be greater than one.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Resize( float scale )
{
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

//////////////////////////////////////////////////////////////////////////////
//
//  Rotate the image clockwise by the given angle.  Do not resize the 
//  image.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Rotate( float angleDegrees )
{
	unsigned char* rgb = To_RGB();
	unsigned char* rgb2 = To_RGB();

	float cos_t = cosf(angleDegrees * (float)(M_PI / 180));
	float sin_t = sinf(angleDegrees * (float)(M_PI / 180));
	for (int i = 0; i < img_height; i++)
	{
		for (int j = 0; j < img_width; j++)
		{
			
		}
	}
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

//------------------------Composing------------------------


void Application::loadSecondaryImge( QString filePath )
{
	mImageSrcSecond.load(filePath);

	renew();

	img_data2 = mImageSrcSecond.bits();
	img_width2 = mImageSrcSecond.width();
	img_height2 = mImageSrcSecond.height();
}

//////////////////////////////////////////////////////////////////////////
//
//	Composite the image A and image B by Over, In, Out, Xor and Atom. 
//
//////////////////////////////////////////////////////////////////////////
void Application::Comp_image( int tMethod )
{
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite the current image over the given image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Over()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image "in" the given image.  See lecture notes for 
//  details.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_In()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image "out" the given image.  See lecture notes for 
//  details.  Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Out()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite current image "atop" given image.  Return success of 
//  operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Atop()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//      Composite this image with given image using exclusive or (XOR).  Return
//  success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::Comp_Xor()
{
	if (img_height == img_height2 && img_width == img_width2)
	{

	}
	else
	{
		std::cout << "Images not the same size" << std::endl;
	}
}

//------------------------NPR------------------------

///////////////////////////////////////////////////////////////////////////////
//
//      Run simplified version of Hertzmann's painterly image filter.
//      You probably will want to use the Draw_Stroke funciton and the
//      Stroke class to help.
// Return success of operation.
//
///////////////////////////////////////////////////////////////////////////////
void Application::NPR_Paint()
{
	mImageDst = QImage(img_data, img_width, img_height, QImage::Format_ARGB32 );
	renew();
}

void Application::NPR_Paint_Layer( unsigned char *tCanvas, unsigned char *tReferenceImage, int tBrushSize )
{

}

///////////////////////////////////////////////////////////////////////////////
//
//      Helper function for the painterly filter; paint a stroke at
// the given location
//
///////////////////////////////////////////////////////////////////////////////
void Application::Paint_Stroke( const Stroke& s )
{
	int radius_squared = (int)s.radius * (int)s.radius;
	for (int x_off = -((int)s.radius); x_off <= (int)s.radius; x_off++) 
	{
		for (int y_off = -((int)s.radius); y_off <= (int)s.radius; y_off++) 
		{
			int x_loc = (int)s.x + x_off;
			int y_loc = (int)s.y + y_off;

			// are we inside the circle, and inside the image?
			if ((x_loc >= 0 && x_loc < img_width && y_loc >= 0 && y_loc < img_height)) 
			{
				int dist_squared = x_off * x_off + y_off * y_off;
				int offset_rgba = (y_loc * img_width + x_loc) * 4;

				if (dist_squared <= radius_squared) 
				{
					img_data[offset_rgba + rr] = s.r;
					img_data[offset_rgba + gg] = s.g;
					img_data[offset_rgba + bb] = s.b;
					img_data[offset_rgba + aa] = s.a;
				} 
				else if (dist_squared == radius_squared + 1) 
				{
					img_data[offset_rgba + rr] = (img_data[offset_rgba + rr] + s.r) / 2;
					img_data[offset_rgba + gg] = (img_data[offset_rgba + gg] + s.g) / 2;
					img_data[offset_rgba + bb] = (img_data[offset_rgba + bb] + s.b) / 2;
					img_data[offset_rgba + aa] = (img_data[offset_rgba + aa] + s.a) / 2;
				}
			}
		}
	}
}





///////////////////////////////////////////////////////////////////////////////
//
//      Build a Stroke
//
///////////////////////////////////////////////////////////////////////////////
Stroke::Stroke() {}

///////////////////////////////////////////////////////////////////////////////
//
//      Build a Stroke
//
///////////////////////////////////////////////////////////////////////////////
Stroke::Stroke(unsigned int iradius, unsigned int ix, unsigned int iy,
	unsigned char ir, unsigned char ig, unsigned char ib, unsigned char ia) :
radius(iradius),x(ix),y(iy),r(ir),g(ig),b(ib),a(ia)
{
}



