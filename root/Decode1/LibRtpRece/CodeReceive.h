#pragma once


#include <string>
#include <iostream>
#include <map>
#include <windows.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

using namespace std;


//using namespace boost;

typedef void (*FrameCallback)(AVFrame* frame,int pix_fmt,int srcW,int srcH);
typedef void (*FrameCallback_RGB24)(uint8_t * frame,int pix_fmt,int srcW,int srcH);
class CBaseReceive
{

public:
	
	CBaseReceive(void){}

	virtual ~CBaseReceive(void){}

public:

	virtual int Pix_Fmt() = 0;
	virtual int Width()   = 0;
	virtual int Height()  = 0;
	virtual BOOL StartReceive(string ip,unsigned short port) = 0;

    virtual void StopReceive() = 0;

	//这个画法是使用了SDL画法
	virtual void SetFunction(FrameCallback func) = 0;

	//这个是可以获取数据自己画，后面的版本是要用directshow vmr画法,因此留了这个接口
	virtual void SetFunctionRGB24(FrameCallback_RGB24 func) = 0;

	//这个是内置的画法，普通GDI画，参考OpenCV源代码
	virtual void SetDrawhWnd(HWND hWnd0,HWND hWnd1) = 0;


};



class CReceiveManager
{
	map<string,CBaseReceive*> _ReceContainer;
public:
	CBaseReceive *CreateH264Receive(string name);
	
	BOOL StartReceiveByName(string name,string ip,unsigned short port);

	void SetDrawhWnd(string name,HWND hWnd0,HWND hWnd1);

	void StopReceiveByName(string name);

	void CloseAllReceive();

	void SetFunctionBGR24(string name ,FrameCallback_RGB24 func);

};

