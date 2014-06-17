#include "StdAfx.h"
#include "UdpServer.h"

//typedef struct _memory_cache
//{
//   int size;
//   char *buf;
//}memory_cache;

UdpAgentServer::UdpAgentServer(void)
{
	_port = 9200;
	//_WritePos = 1;
	//_ReadPos  = 0;
}


UdpAgentServer::~UdpAgentServer(void)
{
}

void UdpAgentServer::StartReceive(unsigned int port,H264DecoderContext *decoder)
{
	_port = port;
	_decoder = decoder;
	//_hWnd0 = hWnd;
	//_Cache = new char[4*1024*1024];
	Start();
}


void UdpAgentServer::StopServer()
{
	_udpserver.Close();
	Stop();
}

memory_cache * UdpAgentServer::GetBufferToDecode()
{
	if(!_memcache.empty())
	{
		memory_cache * mc = _memcache.front();
		_memcache.pop();
		return mc;
	}
	return NULL;
}


void UdpAgentServer::HandlePacket(char * buffer, int len)
{
	unsigned int bufLen = len;


	unsigned int retLen = 0;
	char * retbuffer = _decoder->DecodeFrames_1((const u_char*)buffer,bufLen,retLen);

	if(retbuffer!=NULL)
	{
		memory_cache *mc = new memory_cache() ;
		mc->buf = new char[retLen+12];
		memset(mc->buf+retLen,0,12);
		mc->size = retLen;
		memcpy(mc->buf,retbuffer,retLen);
		_memcache.push(mc);
	}

	//AVFrame * frame =_decoder.DecodeFrames((const u_char*)buffer,bufLen);
	//if(frame!=NULL)
	//{
	//	int Width  = _decoder.GetContext()->width;
	//	int Height = _decoder.GetContext()->height;//_pDecode->GetContext()->height;


	//	if(_RGBBuffer == NULL)
	//	{
	//		int numBytes;

	//		numBytes=avpicture_get_size(
	//			//PIX_FMT_RGB24, 
	//			PIX_FMT_BGR24,
	//			Width,
	//			Height);
	//		_RGBBuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	//		if(_pFrameRGB == NULL)
	//			_pFrameRGB = avcodec_alloc_frame();
	//		avpicture_fill((AVPicture *)_pFrameRGB, _RGBBuffer, PIX_FMT_BGR24,   Width, Height); 

	//		_img_convert_ctx = sws_getContext(Width, Height, 
	//			_decoder.GetContext()->pix_fmt,//PIX_FMT_YUV420P, 
	//			Width, 
	//			Height, 
	//			PIX_FMT_BGR24, 
	//			SWS_BICUBIC, 
	//			NULL, 
	//			NULL, 
	//			NULL);
	//	}

	//	sws_scale(_img_convert_ctx, frame->data, frame->linesize, 0, Height, _pFrameRGB->data, _pFrameRGB->linesize);

	//	if(_hWnd0!=NULL)
	//		_Draw.Draw2(_hWnd0,NULL,_pFrameRGB->data[0],Width,Height);
		//Sleep(100);
}

void UdpAgentServer::Run()
{
	if(!_udpserver.Create(SOCK_DGRAM,0))
	{
		DP0("server create error!");
		return ;
	}
	_udpserver.SetReuseAddr(TRUE);
	_udpserver.Bind(_port);


	//char buffer[1600];
	//int recvlen = 0;


	char buffer[1500];
	while(!this->IsStop())
	{
		//int wpos = _WritePos; 
		//memory_cache * mc = _memcache[wpos];
		//char * buffer = mc->buf;

		

		int recvlen = _udpserver.Recv(buffer,1500);
		//_memcache[wpos]->size = recvlen;
		//_memcache[wpos]->flag = 1; //Ð´Èë

		if(recvlen>0)
		{
			HandlePacket(buffer,recvlen);
			//if(wpos+1 < _member_number )
			//{
			//	if(wpos +1 != _ReadPos ) 
			//	_WritePos ++;
			//}
			//else
			//{
			//	if(_ReadPos != 0)
			//		_WritePos = 0;
			//}
			//
		}
		else
		{
			DP0("udp receive error!");
		}
	}
}
