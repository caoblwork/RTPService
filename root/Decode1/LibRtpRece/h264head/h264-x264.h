
#ifndef __H264_X264_H__
#define __H264_X264_H__ 1


#include <stdarg.h>
#include <stdint.h>
#include "critsect.h"
#include "h264frame.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

}
#if 0
#include <SDL.h>  
#include <SDL_thread.h> 

#define P720_WIDTH 720
#define P720_HEIGHT 480

#define CIF4_WIDTH 704
#define CIF4_HEIGHT 576

#define CIF_WIDTH 352
#define CIF_HEIGHT 288

#define QCIF_WIDTH 176
#define QCIF_HEIGHT 144

#define SQCIF_WIDTH 128
#define SQCIF_HEIGHT 96
#define IT_QCIF 0
#define IT_CIF 1
#endif
typedef unsigned char u_char;

class H264DecoderContext
{
  public:
    H264DecoderContext();
    ~H264DecoderContext();
	AVFrame* DecodeFrames(const u_char * src, unsigned & srcLen,AVFrame * Frame);
    AVFrame* DecodeFrames(const u_char * src, unsigned & srcLen);
	bool Initialize();
    AVCodecContext * GetContext()
	{
		return _context;
	}
  protected:

    AVCodec* _codec;
    AVCodecContext* _context;
    AVFrame* _outputFrame;
    H264Frame* _rxH264Frame;

   
	bool _gotIFrame;
    bool _gotAGoodFrame;
    int _frameCounter;
    int _skippedFrameCounter;
	unsigned int flags;

	bool _initflag;
private: 
#if defined(_TEST_DRAW_WINDOW) 
	struct SwsContext *_img_convert_ctx;
	SDL_Overlay     * _bmp;
	SDL_Surface     * _screen;
	SDL_Rect        _rect;
	SDL_Event       _event;
#endif
};

#define H264_CLOCKRATE        90000
#define H264_BITRATE         768000
#define H264_PAYLOAD_SIZE      1400
#define H264_FRAME_RATE          25
#define H264_KEY_FRAME_INTERVAL  75

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////




#endif /* __H264-X264_H__ */
