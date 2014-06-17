#pragma once
#include "WASocket.hpp"

#include "base/SimpleThread.h"

#include "h264.h"

#include "DrawRGB24.h"

#include <queue>
using namespace std;


typedef struct _memory_cache
{
   unsigned int size;
   char *buf;
}memory_cache;



class UdpAgentServer:public base::SimpleThread
{

private:
 //  char *_Cache;
   queue<memory_cache*> _memcache;
   
 //  int _member_number;
 //  int _WritePos;
 //  int _ReadPos;
   
private:
   CWASocket _udpserver;
   unsigned int _port;
   H264DecoderContext* _decoder;

   
   	//AVFrame * _pFrameRGB;
	//uint8_t * _RGBBuffer;
	//struct SwsContext *_img_convert_ctx;

	//CDrawRGB24 _Draw;
public:
	UdpAgentServer(void);
	~UdpAgentServer(void);
	void Run();

	void StartReceive(unsigned int port,H264DecoderContext *decoder);
	void StopServer();
	memory_cache * GetBufferToDecode();


protected:
	void HandlePacket(char * buffer, int len);
};

