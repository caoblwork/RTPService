#pragma once



#include <windows.h>
#include <malloc.h>

#define STRCMPI  _strcmpi
#include <string.h>
#include "critsect.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}	  



class FFMPEGLibrary 
{
  public:
    FFMPEGLibrary();
    ~FFMPEGLibrary();

    bool Load();

    AVCodec *AvcodecFindDecoder(enum AVCodecID id);

	AVCodecContext *AvcodecAllocContext(const AVCodec* avcodec);
    
	AVFrame *AvcodecAllocFrame(void);
    
	int AvcodecOpen(AVCodecContext *ctx, AVCodec *codec);
    
	int AvcodecClose(AVCodecContext *ctx);
    
	int AvcodecDecodeVideo(AVCodecContext *ctx, AVFrame *pict, int *got_picture_ptr, BYTE *buf, int buf_size);
    
	void AvcodecFree(void * ptr);


    void AvLogSetLevel(int level);


    bool IsLoaded();
    CriticalSection processLock;

  protected:
    bool isLoadedOK;
};

