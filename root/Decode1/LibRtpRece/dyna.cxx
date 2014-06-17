/*
 * Common Plugin code for OpenH323/OPAL
 *
 * This code is based on the following files from the OPAL project which
 * have been removed from the current build and distributions but are still
 * available in the CVS "attic"
 * 
 *    src/codecs/h263codec.cxx 
 *    include/codecs/h263codec.h 

 * The original files, and this version of the original code, are released under the same 
 * MPL 1.0 license. Substantial portions of the original code were contributed
 * by Salyens and March Networks and their right to be identified as copyright holders
 * of the original code portions and any parts now included in this new copy is asserted through 
 * their inclusion in the copyright notices below.
 *
 * Copyright (C) 2006 Post Increment
 * Copyright (C) 2005 Salyens
 * Copyright (C) 2001 March Networks Corporation
 * Copyright (C) 1999-2000 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Open H323 Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): qianbo (418511899@qq.com)
 *                 钱波
 *                 
 */
#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "dyna.h"


#include <string.h>
#if defined(__WIN32__) || defined(_WIN32)
#define snprintf _snprintf
#endif

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

AVCodec *FFMPEGLibrary::AvcodecFindEncoder(enum AVCodecID id)
{
	return avcodec_find_encoder(id);
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

int FFMPEGLibrary::AvcodecEncodeVideo(AVCodecContext *ctx, BYTE *buf, int buf_size, const AVFrame *pict)
{
 	return avcodec_encode_video(ctx,buf,buf_size,pict);
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
			DP0("decode error");
			return -1; //解码出错
		}
		else
		{
			//DP2("decode %d in %d",bytesDecoded,buf_size);
		}
		if(*got_picture_ptr)
		{
			DP2("got pic %d in %d",bytesDecoded,buf_size);
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

void FFMPEGLibrary::AvSetDimensions(AVCodecContext *s, int width, int height)
{
   WaitAndSignal m(processLock);
   avcodec_set_dimensions(s,width,height);
}
  

void FFMPEGLibrary::AvLogSetLevel(int level)
{
	av_log_set_level(level);
}

void FFMPEGLibrary::AvLogSetCallback(void (*callback)(void*, int, const char*, va_list))
{
	av_log_set_callback(callback);
}

int FFMPEGLibrary::FFCheckAlignment(void)
{
	return 1;
	//ff_check_alignment();
}

bool FFMPEGLibrary::IsLoaded()
{
  return isLoadedOK;
}

