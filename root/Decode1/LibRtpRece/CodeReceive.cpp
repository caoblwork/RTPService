#include "StdAfx.h"
#include "CodeReceive.h"
#include "h264head/h264-x264.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <list>
#include "DrawRGB24.h"
using namespace std;
/*template <class T>*/



#if 0
class H264DecodeBuffer {
 
		AVFrame ** qbuframe;
        int qsize;int head;int tail;     // index stop data
        inline void Free()
        {
			for( int i = 0; i < qsize; i++ ) 
			{
				av_free(qbuframe[i]);
				//FFMPEGLibraryInstance.AvcodecFree(qbuframe[i]);
			}
			if (qbuframe != 0)
			{
				delete []qbuframe;
				qbuframe =  0;
			}
			qsize= 1;
			head= tail= 0;
        }

        void CreateMemory(int mn)
		{

			qbuframe = (AVFrame**)malloc(mn*sizeof(qbuframe[0]));
			memset( qbuframe, 0, mn*sizeof(qbuframe[0]));
			for( int i = 0; i < mn; i++ ) 
			{
				qbuframe[i] = avcodec_alloc_frame();
			}
			qsize = mn;
		}

public:
        H264DecodeBuffer(const int size): qsize(1), qbuframe(0), head(0), tail(0)
        {
			CreateMemory(size);
        }
 
        ~H264DecodeBuffer()
        {
                Free();
        }
 
 

		AVFrame *GetWriteBuffer()
		{
             if(IsFull())
			 {
				 return NULL;
			 }
			 return qbuframe[tail];
		}
        bool WriteBufferInc()
		{
			if(IsFull())
			{
				return false;
			}
			//Itemlen = data->mbuf;
		    tail= (tail + 1) & (qsize - 1);
			return true;
		}
        // Retrieve the item from the queue


		AVFrame *GetReadBuffer()
		{
			if(IsEmpty())
				return 0;
			//Itemlen = qbuframe[head]->;
			return qbuframe[head];
		}
		bool ReadBufferInc()
		{
			if(IsEmpty())
				return false;
			head = (head+1)&(qsize-1);
			return true;
		}

        inline void Clear(void) { head= tail= 0; }
        inline int  GetCapacity(void) const { return (qsize - 1); }
        // Count elements
        inline int  GetBusy(void) const   { return ((head > tail) ? qsize : 0) + tail - head; }
        // true - if queue if empty
        inline bool IsEmpty(void) const { return (head == tail); }
        // true - if queue if full
        inline bool IsFull(void) const  { return ( ((tail + 1) & (qsize - 1)) == head ); }
};
//缓冲区类




class H264DecodeBuffer2
{
	/*list<AVFrame*> _con;
	AVFrame* CreateAVFrame()
	{
		av_picture_copy(
	}
    void insert(AVFrame* frame)
	{
		_con.push_back(frame);
	}*/
};


#if 0
while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, 
                           packet.data, packet.size);      
      // Did we get a video frame?
      if(frameFinished) {
        printf("This is frame %d!\n",i++);
        //Save the output.yuv to disk 
		
        img_convert((AVPicture *)pFrameYUV, PIX_FMT_YUV420P, 
                    (AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, 
                    pCodecCtx->height);        
        if(video_st){
         fwrite(pFrameYUV->data[0],1,(pCodecCtx->width*pCodecCtx->height)*3/2,poutFile);          
        }  
       }
            // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }

#endif




//
//static HWND g_hwnd0 = NULL;
//static HWND g_hwnd1 = NULL;


class CCodeReceive;
class H264PackageReceiver
{
public:
	H264PackageReceiver(boost::asio::io_service& io_service,
      const boost::asio::ip::address& listen_address,
      const boost::asio::ip::address& multicast_address,
	  const unsigned short port,
	  CCodeReceive* friendccr
	  ): socket_(io_service),
	  _RGBBuffer(NULL),
	  _pFrameRGB(NULL),
	  _Width(0),
	  _Height(0),
	  _img_convert_ctx(NULL),_friend(friendccr)

	{

		if(_pFrameRGB==NULL)
			_pFrameRGB = avcodec_alloc_frame();
		boost::asio::ip::udp::endpoint listen_endpoint(
			listen_address, port);
		socket_.open(listen_endpoint.protocol());
		socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket_.bind(listen_endpoint);

		// Join the multicast group.
		socket_.set_option(
			boost::asio::ip::multicast::join_group(multicast_address));
		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), sender_endpoint_,
			boost::bind(&H264PackageReceiver::handle_receive_from_direct, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	~H264PackageReceiver()
	{
		av_free(_RGBBuffer);   
		av_free(_pFrameRGB); 
	}

  



  void handle_receive_from_direct(const boost::system::error_code& error,
      size_t bytes_recvd)
  {
    if (!error)
    {
		
		AVFrame * frame =_pDecode->DecodeFrames((const u_char*)data_,bytes_recvd);
		if(frame!=NULL)
		{
            if(_Width == 0)
			{
				_Width  =_pDecode->GetContext()->width;
				_Height =_pDecode->GetContext()->height;
			}
			//if(_Width !=_pDecode->GetContext()->width || _Height != _pDecode->GetContext()->height)
			//{
			//	//影片更换
			//}

#if 0  //如果需要用sdl渲染画面，可以打开这个
			if(_function )
				_function(frame,_pDecode->GetContext()->pix_fmt,
					_pDecode->GetContext()->width,
					_pDecode->GetContext()->height
		
					);
#endif            
			
			if(_RGBBuffer == NULL)
			{
				int numBytes;

				numBytes=avpicture_get_size(
					//PIX_FMT_RGB24, 
					PIX_FMT_BGR24,
					_Width,
					_Height);
				_RGBBuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

				avpicture_fill((AVPicture *)_pFrameRGB, _RGBBuffer, PIX_FMT_BGR24,   _Width, _Height); 

				_img_convert_ctx = sws_getContext(_Width, _Height, 
					_pDecode->GetContext()->pix_fmt,//PIX_FMT_YUV420P, 
					_Width, 
					_Height, 
					PIX_FMT_BGR24, 
					SWS_BICUBIC, 
					NULL, 
					NULL, 
					NULL);
			}

			sws_scale(_img_convert_ctx, frame->data, frame->linesize, 0, _Height, _pFrameRGB->data, _pFrameRGB->linesize);

			_Draw.Draw2(g_hwnd0,g_hwnd1,_pFrameRGB->data[0],_Width,_Height);

	/*		if(_functionRGB24)
			{
				_functionRGB24(_pFrameRGB->data[0],_pDecode->GetContext()->pix_fmt,_Width,_Height);
			}*/
	
		}
	

        socket_.async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&H264PackageReceiver::handle_receive_from_direct, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));

    }
  }


	
private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint sender_endpoint_;
  enum { max_length = 1500 };
  char data_[max_length];
  unsigned short multicast_port;

public:
	//H264DecodeBuffer*   _pDecodeBuffer;
	//解码器
	//H264DecoderContext* _pDecode;

	//FrameCallback _function;
	//FrameCallback_RGB24 _functionRGB24 ;

	AVFrame * _pFrameRGB;
	uint8_t * _RGBBuffer;
	//转换
	struct SwsContext *_img_convert_ctx;

	//同时画两个窗口

    CDrawRGB24 _Draw;

	int _Width ;
	int _Height;

    CCodeReceive * _friend;

};




//这个类还可以进一步
class CCodeReceive:public CBaseReceive
{
friend DWORD WINAPI ThreadProcReceive(LPVOID param);
friend class H264PackageReceiver;
private:
     int Pix_Fmt()
	 {
		 return _pDecode->GetContext()->pix_fmt;
	 }
	 int Width()
	 {
		 return _pDecode->GetContext()->width;
	 }
	 int Height()
	 {
		 return _pDecode->GetContext()->height;
	 }




	 HANDLE _ThreadHandle  ;
	 boost::asio::io_service io_service;
public:
	CCodeReceive::CCodeReceive(void):_ThreadHandle(NULL)/*_pDecodeBuffer(NULL)*/,
		_pDecode(NULL),
		_function(NULL),
		_functionRGB24(NULL),_hWnd0(NULL),_hWnd1(NULL)
	{
	}

	CCodeReceive::~CCodeReceive(void)
	{
	}
public:
	BOOL StartReceive(string multip ,unsigned short uport)
	{
		ip    = multip;
		port  = uport;

		if(!CreateDecode())
			return FALSE;

		_ThreadHandle  = CreateThread(NULL,0,ThreadProcReceive,this,0,NULL);
		if(_ThreadHandle !=NULL)
			return TRUE;
		return FALSE;
	}

    void StopReceive()
	{
		io_service.stop();
	}


public:
	//解码器
	H264DecoderContext* _pDecode;
	
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
	FrameCallback _function;
    FrameCallback_RGB24 _functionRGB24;
	void SetFunction(FrameCallback func)
	{
		_function = func;
	}

    void SetFunctionRGB24(FrameCallback_RGB24 func) 
	{
		_functionRGB24 = func;
	}
	void SetDrawhWnd(HWND hWnd0,HWND hWnd1)
	{
		_hWnd0 = hWnd0;
		_hWnd1 = hWnd1;
	}
private:
	HWND _hWnd0;
	HWND _hWnd1;
};


DWORD WINAPI ThreadProcReceive(LPVOID param)
{
	CCodeReceive *ccr = (CCodeReceive *)param;
	if(ccr==NULL)
		return 0;
		
    boost::asio::io_service& io_service = ccr->io_service;
	io_service.reset();
	try
	{
		H264PackageReceiver r(io_service,
			boost::asio::ip::address::from_string("0.0.0.0"),
			boost::asio::ip::address::from_string(ccr->ip.c_str()),
			ccr->port,
			ccr
			);
		//r._pDecode       = ccr->_pDecode;
		//r._pDecodeBuffer = ccr->_pDecodeBuffer;
		//回调函数
		//r._function = NULL;
		//r._function      = ccr->_function;
		//r._functionRGB24 = ccr->_functionRGB24;
		io_service.run();
	}
	catch (std::exception& e)
	{

		//AfxMessageBox(e.what());
		std::cerr << "Exception: " << e.what() << "\n";
	}
	//AfxMessageBox("exit thread");
	return 0;
}


#endif


/*
使用IFilterGraph::ConnectDirect进行连接 

HRESULT   ConnectDirect( 
    IPin   *ppinOut,   //输出PIN 
    IPin   *ppinIn,     //输入PIN 
    const   AM_MEDIA_TYPE   *pmt   //指定媒体类型，在这里指定YV12格式的媒体类型进行连接即可 
); 
*/



