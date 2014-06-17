#include "StdAfx.h"
#include "DrawPacket.h"


DrawPacket::DrawPacket(void)
{
	_RGBBuffer = NULL;
	_pFrameRGB = NULL;
	_hWnd0 = NULL;
	_fps = 10;
}


DrawPacket::~DrawPacket(void)
{
}


void DrawPacket::StartReceive(HWND hWnd, unsigned short port)
{
	_hWnd0 = hWnd;
	if(!_decoder.Initialize())
	{
		DP0("decoder init error!");
		return ;
	}
	_packetserver.StartReceive(port,&_decoder);
	Start();
}
void DrawPacket::StopServer()
{
	_packetserver.StopServer();
	
	memory_cache * mc = _packetserver.GetBufferToDecode();
	while(mc!=NULL)
	{
		delete []mc->buf;
		delete mc;
		mc = _packetserver.GetBufferToDecode();
	}
	Stop();
}
void DrawPacket::Run()
{
	//第一次解码时间
	unsigned int firstDecodeTime = 0;
	int TickCountStart = 0;
	//画帧计数
	unsigned int FrameCount = 1;
	int fpsTick = 1000/_fps;
	int b =10;
	while(!IsStop())
	{

		memory_cache * mc = _packetserver.GetBufferToDecode();
		if(mc == NULL)
		{
			Sleep(10);
			continue;
		}
		if(TickCountStart == 0)
		{
			firstDecodeTime = timeGetTime();
			TickCountStart = 1;
		}
		AVFrame * frame =_decoder.DecodeFrames_2((const u_char*)mc->buf,mc->size);

		if(frame!=NULL)
		{

			int Width  = _decoder.GetContext()->width;
			int Height = _decoder.GetContext()->height;//_pDecode->GetContext()->height;


			if(_RGBBuffer == NULL)
			{
				int numBytes;

				numBytes=avpicture_get_size(
					//PIX_FMT_RGB24, 
					PIX_FMT_BGR24,
					Width,
					Height);
				_RGBBuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

				if(_pFrameRGB == NULL)
					_pFrameRGB = avcodec_alloc_frame();
				avpicture_fill((AVPicture *)_pFrameRGB, _RGBBuffer, PIX_FMT_BGR24,   Width, Height); 

				_img_convert_ctx = sws_getContext(Width, Height, 
					_decoder.GetContext()->pix_fmt,//PIX_FMT_YUV420P, 
					Width, 
					Height, 
					PIX_FMT_BGR24, 
					SWS_BICUBIC, 
					NULL, 
					NULL, 
					NULL);
			}

			sws_scale(_img_convert_ctx, frame->data, frame->linesize, 0, Height, _pFrameRGB->data, _pFrameRGB->linesize);

			if(_hWnd0!=NULL)
				_Draw.Draw2(_hWnd0,NULL,_pFrameRGB->data[0],Width,Height);
			
			FrameCount++;

			unsigned int x = timeGetTime()-firstDecodeTime;

			unsigned int y = fpsTick * FrameCount;
			//Sleep(25);
			//Sleep(30);
			if(x<y)
			{
			   timeBeginPeriod(1);  
               Sleep(30);  
               timeEndPeriod(1);  
			}

		}
		delete []mc->buf;
		delete mc;
	}
}