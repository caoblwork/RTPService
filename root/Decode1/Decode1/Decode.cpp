#include "StdAfx.h"
#include "Decode.h"

#if 0
Decode::Decode(void)
{
	fmt_ctx = NULL;
	dec_ctx = NULL;
	dec = NULL;
	stream = NULL;
	src_filename = NULL;
	dst_filename = NULL;
	dst_file = NULL;
	memset(dst_data,0,sizeof(dst_data));
	frame = NULL;
	frame_count = 0;
}


Decode::~Decode(void)
{
}

H264DecoderContext::H264DecoderContext()
{
	_f = NULL;
	FFMPEGLibraryInstance.Load();
  if (!FFMPEGLibraryInstance.IsLoaded()) return;

  _gotIFrame = false;
  _gotAGoodFrame = false;
  _frameCounter = 0; 
  _skippedFrameCounter = 0;
  _rxH264Frame = new H264Frame();

  if ((_codec = FFMPEGLibraryInstance.AvcodecFindDecoder(CODEC_ID_H264)) == NULL) {
    //TRACEI(1, "H264\tDecoder\tCodec not found for decoder");
    return;
  }

  _context = FFMPEGLibraryInstance.AvcodecAllocContext();
  if (_context == NULL) {
    TRACEI(1, "H264\tDecoder\tFailed to allocate context for decoder");
    return;
  }

  _outputFrame = FFMPEGLibraryInstance.AvcodecAllocFrame();
  if (_outputFrame == NULL) {
    TRACEI(1, "H264\tDecoder\tFailed to allocate frame for encoder");
    return;
  }

  if (FFMPEGLibraryInstance.AvcodecOpen(_context, _codec) < 0) {
    TRACEI(1, "H264\tDecoder\tFailed to open H.264 decoder");
    return;
  }
  else
  {
    TRACEI(1, "H264\tDecoder\tDecoder successfully opened");
  }
  //_lpOut = new unsigned char[2500000];//最大2.5M空间不到

  //avpicture_fill((AVPicture *)_pFrameYUV, _lpOut, PIX_FMT_YUV420P,1280, 720);   

	Initialize();



}

H264DecoderContext::~H264DecoderContext()
{
  if (FFMPEGLibraryInstance.IsLoaded())
  {
	  if (_context != NULL)
	  {
		  if (_context->codec != NULL)
		  {
			  FFMPEGLibraryInstance.AvcodecClose(_context);
			  TRACEI(4, "H264\tDecoder\tClosed H.264 decoder, decoded " << _frameCounter << " Frames, skipped " << _skippedFrameCounter << " Frames" );
		  }
	  }

	  FFMPEGLibraryInstance.AvcodecFree(_context);
	  FFMPEGLibraryInstance.AvcodecFree(_outputFrame);
  }
  if (_rxH264Frame) delete _rxH264Frame;


 // sws_freeContext(
}


bool H264DecoderContext::Initialize()
{
	//if (!FFMPEGLibraryInstance.IsLoaded()) return false;
#define _W 1280
#define _H 720
	_img_convert_ctx = NULL;
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
	{  
		//fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());  
		return false;
	}  

	/* the codec gives us the frame size, in samples */   
	_screen = SDL_SetVideoMode(_W,_H, 0, 0); 
	_bmp    = SDL_CreateYUVOverlay(_W, _H,  SDL_YV12_OVERLAY,  _screen);
	_f = fopen("e:\\test.264","wb");
	if(_f == NULL)
		return false;
	return true;
}





int H264DecoderContext::DecodeFrames(const u_char * src, unsigned & srcLen, char * dst, unsigned & dstLen, unsigned int & flags)
{
 // if (!FFMPEGLibraryInstance.IsLoaded()) return 0;
  //CString str;
  // create RTP frame from source buffer
  static int iiflag = 0;
  RTPFrame srcRTP(src, srcLen);
  
  // create RTP frame from destination buffer
 // RTPFrame dstRTP(dst, dstLen, 0);
 // dstLen = 0;

  if (!_rxH264Frame->SetFromRTPFrame(srcRTP, flags)) {
	  _rxH264Frame->BeginNewFrame();
	  sprintf(dst,"%s\n","setfromrtpframe is not ok!");
	  flags = (_gotAGoodFrame ? requestIFrame : 0);
	  _gotAGoodFrame = false;
	  return 1;
  }

  if (srcRTP.GetMarker()==0)
  {

		return 1;
  } 

  if (_rxH264Frame->GetFrameSize()==0)
  {
	  _rxH264Frame->BeginNewFrame();
	   sprintf(dst,"GetFrameSize==0! skip frame counter is %d",_skippedFrameCounter);
	  _skippedFrameCounter++;
	  flags = (_gotAGoodFrame ? requestIFrame : 0);
	  _gotAGoodFrame = false;
	  return 1;
  }


//  str.Format("H264 Decoder %d bytes \n ",_rxH264Frame->GetFrameSize() );
//  TRACEI_UP(4,  str);

  // look and see if we have read an I frame.
  if (_gotIFrame == 0)
  {
	  //下面的代码暂时屏蔽
    //if (!_rxH264Frame->IsSync())
    //{
    //  TRACEI(1, "H264 Decoder Waiting for an I-Frame\n");
    //  _rxH264Frame->BeginNewFrame();
    //  flags = (_gotAGoodFrame ? requestIFrame : 0);
    //  _gotAGoodFrame = false;
    //  return 1;
    //}
    _gotIFrame = 1;
  }
 

  int gotPicture = 0;
  uint32_t bytesUsed = 0;
  int bytesDecoded = FFMPEGLibraryInstance.AvcodecDecodeVideo(_context, _outputFrame, &gotPicture, _rxH264Frame->GetFramePtr() + bytesUsed, _rxH264Frame->GetFrameSize() - bytesUsed);

  _rxH264Frame->BeginNewFrame();
  if (!gotPicture) 
  {
//	  str.Format("H264 Decoder Decoded bytes without getting a Picture...%d \n", bytesDecoded); 

//    TRACEI(1, str); 
	   sprintf(dst,"%s\n","!gotPicture");
    _skippedFrameCounter++;
    flags = (_gotAGoodFrame ? requestIFrame : 0);

    _gotAGoodFrame = false;
    return 1;
  }

  //if(iiflag<1000)
  //{
	 // const int x = 1;
	 // char naluhead[4] = {0x00,0x00,0x00,0x01};   

	 // fwrite(&naluhead[0],4,1,_f);
	 // fwrite(_rxH264Frame->GetFramePtr(),_rxH264Frame->GetFrameSize(),1,_f);
	 // iiflag++;
  //}
  //if(iiflag ==1000)
  //{
	 // fclose(_f);
	 // _f = NULL;
  //}
  //得到一帧
  int w = 640;//_context->width;//1280;
  int h = 480;//_context->height;

  if (!_img_convert_ctx)
  {
	  _img_convert_ctx = sws_getContext(
		  w, 
		  h,
		  _context->pix_fmt,
		  w, h,
		  //PIX_FMT_RGB24,
		  PIX_FMT_YUV420P,
		  SWS_BICUBIC, NULL, NULL, NULL);
  }

  SDL_LockYUVOverlay(_bmp);
  AVPicture pict;
  pict.data[0] = _bmp->pixels[0];
  pict.data[1] = _bmp->pixels[2];
  pict.data[2] = _bmp->pixels[1];

  pict.linesize[0] = _bmp->pitches[0];
  pict.linesize[1] = _bmp->pitches[2];
  pict.linesize[2] = _bmp->pitches[1];

  // Convert the image into YUV format that SDL uses
  int ret = sws_scale(_img_convert_ctx,
	  _outputFrame->data, 
	  _outputFrame->linesize, 
	  0, 
	  h,
	  pict.data, 
	  pict.linesize);
  SDL_UnlockYUVOverlay(_bmp);
  _rect.x = 0;
  _rect.y = 0;                  
  _rect.w = w;                  
  _rect.h = h;                  
  SDL_DisplayYUVOverlay(_bmp, &_rect);  
  flags = PluginCodec_ReturnCoderLastFrame;
  _frameCounter++;
  _gotAGoodFrame = true;
  return 1;

  
	

}


#endif