#pragma once
#include "base/SimpleThread.h"

#include "h264.h"
#include "DrawRGB24.h"
#include "UdpServer.h"
class DrawPacket:public base::SimpleThread
{
public:
	DrawPacket(void);
	~DrawPacket(void);

	void Run();
	void StartReceive(HWND hWnd,unsigned short port);
	void StopServer();

private:

	AVFrame * _pFrameRGB;
	uint8_t * _RGBBuffer;
	struct SwsContext *_img_convert_ctx;
	HWND _hWnd0;
//	HWND _hWnd1;
	CDrawRGB24 _Draw;
	H264DecoderContext _decoder;

	UdpAgentServer _packetserver;
	//frame 
	int _fps;
};

