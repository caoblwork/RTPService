#pragma once
#include "CodeReceive.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "DrawRGB24.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "h264head/h264-x264.h"

#define INBUF_SIZE 4096



class CodeReceive3:public CBaseReceive
{
	friend DWORD WINAPI ThreadProcReceive(LPVOID param);
public:
	CodeReceive3();
	~CodeReceive3(void);
protected:
	void CreateiocpReceiver(const boost::asio::ip::address& listen_address,
		const boost::asio::ip::address& multicast_address,
		const unsigned short port);

	void handle_receive_from_direct(const boost::system::error_code& error,
		size_t bytes_recvd);



	BOOL CreateDecode()
	{
		if(_pDecode==NULL)
		{
			_pDecode= new H264DecoderContext();

			if(_pDecode == NULL)
				return FALSE;
			if(!_pDecode->Initialize())
				return FALSE;
		}
		return TRUE;
	}
	void DeleteDecode()
	{
		if(_pDecode!=NULL)
		{
			delete _pDecode;
			_pDecode = NULL;
		}
	}


public:
	virtual int Pix_Fmt();
	virtual int Width()  ;
	virtual int Height() ;
	virtual BOOL StartReceive(string ip,unsigned short port) ;

	virtual void StopReceive() ;

	//这个画法是使用了SDL画法
	virtual void SetFunction(FrameCallback func) ;

	//这个是可以获取数据自己画，后面的版本是要用directshow vmr画法
	virtual void SetFunctionRGB24(FrameCallback_RGB24 func) ;

	//这个是内置的画法，普通GDI画，参考OpenCV源代码,预览画像
	virtual void SetDrawhWnd(HWND hWnd0,HWND hWnd1) ;

	// static DWORD ThreadProc_Recv(LPVOID param);

private:

	boost::asio::io_service io_service_;
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	enum { max_length = 1500 };
	char data_[max_length];

	string _multiip;
	unsigned short _port;

private:
	H264DecoderContext* _pDecode;

	AVFrame * _pFrameRGB;
	uint8_t * _RGBBuffer;
	struct SwsContext *_img_convert_ctx;
	//同时画两个窗口
	CDrawRGB24 _Draw;
	HWND _hWnd0;
	HWND _hWnd1;

    FrameCallback _functionSDL;
	FrameCallback_RGB24 _functionRGB24;
    
};



