#pragma once



extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/avutil.h>
}
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "Decode/critsect.h"
//#include "Decode/h264frame.h"
#include <sdl/SDL.h>  
#include <sdl/SDL_thread.h> 



class SDLDraw
{
public:
	SDLDraw()
	{
		_Init = false;
	}
	~SDLDraw()
	{
	  sws_freeContext(_img_convert_ctx);
	}
private:
	SDL_Overlay     * _bmp;
	SDL_Surface     * _screen;
	SDL_Rect        _rect;
	SDL_Event       _event;
	struct SwsContext *_img_convert_ctx;




public:	
	bool _Init ;
	bool Initialize(HWND hWnd ,int w , int h,int pix_fmt )
	{
		_Init = true;
		char variable[256];
		sprintf(variable, "SDL_WINDOWID=0x%lx", hWnd);
		SDL_putenv(variable);

		//_img_convert_ctx = NULL;
		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
		{  
			_Init = false;
			return false;
		}  

		/* the codec gives us the frame size, in samples */   
		_screen = SDL_SetVideoMode(w,h, 0, 0); 
		_bmp    = SDL_CreateYUVOverlay(w, h,  SDL_YV12_OVERLAY,  _screen);
	

		if (!_img_convert_ctx)
		{ 
			_img_convert_ctx = sws_getContext(
				w, 
				h,
				(AVPixelFormat)pix_fmt,
				w, h,
				//PIX_FMT_RGB24,
				PIX_FMT_YUV420P,
				SWS_BICUBIC, NULL, NULL, NULL);
		}

		return true;
	}

	void Draw(AVFrame * _outputFrame,int w,int h)
	{


		SDL_LockYUVOverlay(_bmp);
		AVPicture pict;
		pict.data[0] = _bmp->pixels[0];
		pict.data[1] = _bmp->pixels[2];
		pict.data[2] = _bmp->pixels[1];

		pict.linesize[0] = _bmp->pitches[0];
		pict.linesize[1] = _bmp->pitches[2];
		pict.linesize[2] = _bmp->pitches[1];

		// Convert the image into YUV format that SDL uses

		int ret = sws_scale(_img_convert_ctx,
			_outputFrame->data, 
			_outputFrame->linesize, 
			0, 
			h,
			pict.data, 
			pict.linesize); 
		SDL_UnlockYUVOverlay(_bmp);
		_rect.x = 0;
		_rect.y = 0;                  
		_rect.w = w;                  
		_rect.h = h;                  
		SDL_DisplayYUVOverlay(_bmp, &_rect);  

	}



};
#if 0
class H264DecoderContext
{
  public:
    H264DecoderContext();
    ~H264DecoderContext();
    int DecodeFrames(const u_char * src, unsigned & srcLen, char * dst, unsigned & dstLen, unsigned int & flags);
	int DecodeFrames_test(const u_char * src, unsigned & srcLen, u_char * dst, unsigned & dstLen, unsigned int & flags);

	bool Initialize();
  protected:
    CriticalSection _mutex;

    AVCodec* _codec;
    AVCodecContext* _context;
    AVFrame* _outputFrame;
    H264Frame* _rxH264Frame;

	unsigned char * _lpOut;
	AVPicture* _pFrameYUV;
   
    struct SwsContext *_img_convert_ctx;


	bool _gotIFrame;
    bool _gotAGoodFrame;
    int _frameCounter;
    int _skippedFrameCounter;
private:
    SDL_Overlay     * _bmp;
	SDL_Surface     * _screen;
	SDL_Rect        _rect;
	SDL_Event       _event;
	FILE * _f;
};

#endif