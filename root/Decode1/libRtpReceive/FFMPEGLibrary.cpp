#include "StdAfx.h"
#include "FFMPEGLibrary.h"




FFMPEGLibrary::FFMPEGLibrary()
{
	isLoadedOK = false;
}

FFMPEGLibrary::~FFMPEGLibrary()
{
}




bool FFMPEGLibrary::Load()
{
	WaitAndSignal m(processLock);      
	if (IsLoaded())
		return true;

	av_register_all();//注册库中所有可用的文件格式和编码器
	//avcodec_register_all();
	avformat_network_init();//注册网络接口

	isLoadedOK = true;
	return true;
}


AVCodec *FFMPEGLibrary::AvcodecFindDecoder(enum AVCodecID id)
{
  WaitAndSignal m(processLock);
  return avcodec_find_decoder(id);
}

AVCodecContext *FFMPEGLibrary::AvcodecAllocContext(const AVCodec* avcodec)
{
  WaitAndSignal m(processLock);
  return avcodec_alloc_context3(avcodec);
}

AVFrame *FFMPEGLibrary::AvcodecAllocFrame(void)
{
	return avcodec_alloc_frame();
}

int FFMPEGLibrary::AvcodecOpen(AVCodecContext *ctx, AVCodec *codec)
{
   WaitAndSignal m(processLock);
   return avcodec_open2(ctx, codec,NULL);	
}

int FFMPEGLibrary::AvcodecClose(AVCodecContext *ctx)
{
   WaitAndSignal m(processLock);
   return avcodec_close(ctx);
}



int FFMPEGLibrary::AvcodecDecodeVideo(AVCodecContext *ctx, AVFrame *pict, int *got_picture_ptr, BYTE *buf, int buf_size)
{

#if 1
	
    int bytesDecoded = 0;
	unsigned char *DataPos = buf;
	int DataSize = buf_size;
	while(1)
	{
		AVPacket pkt;
		av_init_packet(&pkt);
		DataPos  = DataPos  + bytesDecoded;
		DataSize = DataSize - bytesDecoded;
		pkt.data = DataPos  ; 
		pkt.size = DataSize ; 
		bytesDecoded = avcodec_decode_video2(ctx,pict,got_picture_ptr,&pkt);
		av_free_packet(&pkt);
		if(bytesDecoded < 0)
		{
			//DP0("decode error");
			return -1; //解码出错
		}
		else
		{
			//DP2("decode %d in %d",bytesDecoded,buf_size);
		}
		if(*got_picture_ptr)
		{
			//DP2("got pic %d in %d",bytesDecoded,buf_size);
			return bytesDecoded;
		}
		if(bytesDecoded == 0)
			return 0;
	}

#endif

}

void FFMPEGLibrary::AvcodecFree(void * ptr)
{

   WaitAndSignal m(processLock);
   av_free(ptr);	
}


void FFMPEGLibrary::AvLogSetLevel(int level)
{
	av_log_set_level(level);
}



bool FFMPEGLibrary::IsLoaded()
{
  return isLoadedOK;
}

