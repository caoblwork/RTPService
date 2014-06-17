#include "StdAfx.h"
#include "CodeReceive2.h"



CodeReceive2::CodeReceive2():socket_(io_service_)
{
   _hWnd0 = NULL;
   _hWnd1 = NULL;
   _pDecode = NULL;
   _pFrameRGB= NULL;
   _RGBBuffer = NULL;
   _img_convert_ctx = NULL;
   _functionRGB24 = NULL;
}

CodeReceive2::~CodeReceive2(void)
{
	av_free(_RGBBuffer);   
	av_free(_pFrameRGB); 
}


void CodeReceive2::CreateiocpReceiver(
	  
      const boost::asio::ip::address& listen_address,
      const boost::asio::ip::address& multicast_address,
	  const unsigned short port
	  )
{
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
		boost::bind(&CodeReceive2::handle_receive_from_direct, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}


void CodeReceive2::handle_receive_from_direct(const boost::system::error_code& error,
      size_t bytes_recvd)
{
    if (!error)
    {
		
		AVFrame * frame =_pDecode->DecodeFrames((const u_char*)data_,bytes_recvd);
		if(frame!=NULL)
		{
			int Width  = this->Width();//_pDecode->GetContext()->width;
			int Height = this->Height();//_pDecode->GetContext()->height;

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
					Width,
					Height);
				_RGBBuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

				if(_pFrameRGB == NULL)
					_pFrameRGB = avcodec_alloc_frame();
				avpicture_fill((AVPicture *)_pFrameRGB, _RGBBuffer, PIX_FMT_BGR24,   Width, Height); 

				_img_convert_ctx = sws_getContext(Width, Height, 
					_pDecode->GetContext()->pix_fmt,//PIX_FMT_YUV420P, 
					Width, 
					Height, 
					PIX_FMT_BGR24, 
					SWS_BICUBIC, 
					NULL, 
					NULL, 
					NULL);
			}

			sws_scale(_img_convert_ctx, frame->data, frame->linesize, 0, Height, _pFrameRGB->data, _pFrameRGB->linesize);

			if(_hWnd0!=NULL || _hWnd1!=NULL)
				_Draw.Draw2(_hWnd0,_hWnd1,_pFrameRGB->data[0],Width,Height);

			//Sleep(5);
			if(_functionRGB24)
			{

				_functionRGB24(_pFrameRGB->data[0],_pDecode->GetContext()->pix_fmt,Width,Height);
			}
	
		}
	

        socket_.async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&CodeReceive2::handle_receive_from_direct, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));

    }
  }





int CodeReceive2::Pix_Fmt()
{
	return _pDecode->GetContext()->pix_fmt;
}
int CodeReceive2::Width() 
{
    return _pDecode->GetContext()->width;
}
int CodeReceive2::Height() 
{
	return _pDecode->GetContext()->height;
}
BOOL CodeReceive2::StartReceive(string multip,unsigned short uport) 
{
	_multicast_ip    = multip;
	_multicast_port  = uport;

	if(!CreateDecode())
		return FALSE;

	HANDLE threadHandle =  CreateThread(NULL,0,ThreadProcReceive,this,0,NULL);
	if(threadHandle !=NULL)
		return TRUE;
	return FALSE;
}

void CodeReceive2::StopReceive() 
{
	this->io_service_.stop();
}

	//这个画法是使用了SDL画法
void CodeReceive2::SetFunction(FrameCallback func)
{

}

	//这个是可以获取数据自己画，后面的版本是要用directshow vmr画法
void CodeReceive2::SetFunctionRGB24(FrameCallback_RGB24 func) 
{
	this->_functionRGB24 = func;
}

	//这个是内置的画法，普通GDI画，参考OpenCV源代码
void CodeReceive2::SetDrawhWnd(HWND hWnd0,HWND hWnd1)
{

	_hWnd0 = hWnd0;
	_hWnd1 = hWnd1;
}


DWORD WINAPI ThreadProcReceive(LPVOID param)
{
	CodeReceive2 *ccr = (CodeReceive2 *)param;
	if(ccr==NULL)
		return 0;
		
    boost::asio::io_service& io_service = ccr->io_service_;
	io_service.reset();
	try
	{
		ccr->CreateiocpReceiver(boost::asio::ip::address::from_string("0.0.0.0"),
			boost::asio::ip::address::from_string(ccr->_multicast_ip.c_str()),
			ccr->_multicast_port);
		io_service.run();
	}
	catch (std::exception& e)
	{
		

		//AfxMessageBox(e.what());
		//std::cerr << "Exception: " << e.what() << "\n";
	}
	//AfxMessageBox("exit thread");
	return 0;
}







#if 0 



CBaseReceive * CReceiveManager::CreateH264Receive(string name)
{
	map<string,CBaseReceive*>::iterator iter;
	iter = _ReceContainer.find(name);
	if(iter == _ReceContainer.end())
	{
		CodeReceive2* newrecv = new CodeReceive2();
		_ReceContainer[name] = newrecv;
		return newrecv;
	}
	else
	{
		return _ReceContainer[name] ;
	}
}



BOOL CReceiveManager::StartReceiveByName(string name,string ip,unsigned short port)
{
    map<string,CBaseReceive*>::iterator iter;
	iter = _ReceContainer.find(name);
	if(iter != _ReceContainer.end())
	{
		CodeReceive2* ccr =(CodeReceive2*)iter->second;
		return  ccr->StartReceive(ip,port);
	}
	return FALSE;
}

void CReceiveManager::SetDrawhWnd(string name,HWND hWnd0,HWND hWnd1)
{
   map<string,CBaseReceive*>::iterator iter;
	iter = _ReceContainer.find(name);
	if(iter != _ReceContainer.end())
	{
		CodeReceive2* ccr =(CodeReceive2*)iter->second;
		return  ccr->SetDrawhWnd(hWnd0,hWnd1);
	}
}

void CReceiveManager::StopReceiveByName(string name)
{
	map<string,CBaseReceive*>::iterator iter;
	iter = _ReceContainer.find(name);
	if(iter != _ReceContainer.end())
	{
		CodeReceive2* ccr = (CodeReceive2*)iter->second;
		ccr->StopReceive();
	}
}

void CReceiveManager::CloseAllReceive()
{
	map<string,CBaseReceive*>::iterator iter;
	iter = _ReceContainer.begin();
	while(iter!= _ReceContainer.end())
	{
		CodeReceive2* ccr = (CodeReceive2*)iter->second;
		ccr->StopReceive();
		delete ccr;
	}
}

void CReceiveManager::SetFunctionBGR24(string name ,FrameCallback_RGB24 func)
{
	map<string,CBaseReceive*>::iterator iter;
	iter = _ReceContainer.begin();
	if(iter!= _ReceContainer.end())
	{
		CodeReceive2* ccr = (CodeReceive2*)iter->second;
		ccr->SetFunctionRGB24(func);
	}

}

#endif