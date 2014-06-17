#pragma once

#include <list>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
using namespace std;

class BufferManager
{
	list<AVPacket*> mem;
public:
	BufferManager(void);
	~BufferManager(void);
};

