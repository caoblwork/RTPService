
#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "stdint.h"
#include "h264-x264.h"
#include <windows.h>
 #include "dyna.h"
 #include "rtpframe.h"


#include <stdlib.h>
#if defined(_WIN32) || defined(_WIN32_WCE)
  #include <malloc.h>
  #define STRCMPI  _strcmpi
#else
  #include <semaphore.h>
  #define STRCMPI  strcasecmp
#endif
#include <string.h>

FFMPEGLibrary FFMPEGLibraryInstance;



class RTPFrame_2 {
public:
  RTPFrame_2(const unsigned char * frame, int frameLen) {
    _frame = (unsigned char*) frame;
    _frameLen = frameLen;
  };

  RTPFrame_2(unsigned char * frame, int frameLen, unsigned char payloadType) {
    _frame = frame;
    _frameLen = frameLen;
    if (_frameLen > 0)
      _frame [0] = 0x80;
    SetPayloadType(payloadType);
  }

  unsigned GetPayloadSize() const {
    return (_frameLen - GetHeaderSize());
  }

  void SetPayloadSize(int size) {
    _frameLen = size + GetHeaderSize();
  }

  int GetFrameLen () const {
    return (_frameLen);
  }

  unsigned char * GetPayloadPtr() const {
    return (_frame + GetHeaderSize());
  }

  int GetHeaderSize() const {
    int size;
    size = 12;
    if (_frameLen < 12) 
      return 0;
    size += (_frame[0] & 0x0f) * 4;
    if (!(_frame[0] & 0x10))
      return size;
    if ((size + 4) < _frameLen) 
      return (size + 4 + (_frame[size + 2] << 8) + _frame[size + 3]);
    return 0;
  }

  bool GetMarker() const {
    if (_frameLen < 2) 
      return false;
    return (_frame[1] & 0x80);
  }

  unsigned GetSequenceNumber() const {
    if (_frameLen < 4)
      return 0;
    return (_frame[2] << 8) + _frame[3];
  }

  void SetMarker(bool set) {
    if (_frameLen < 2) 
      return;
    _frame[1] = _frame[1] & 0x7f;
    if (set) _frame[1] = _frame[1] | 0x80;
  }

  void SetPayloadType(unsigned char type) {
    if (_frameLen < 2) 
      return;
    _frame[1] = _frame [1] & 0x80;
    _frame[1] = _frame [1] | (type & 0x7f);
  }

  unsigned char GetPayloadType() const
  {
    if (_frameLen < 1)
      return 0xff;
    return _frame[1] & 0x7f;
  }

  unsigned long GetTimestamp() const {
    if (_frameLen < 8)
      return 0;
    return ((_frame[4] << 24) + (_frame[5] << 16) + (_frame[6] << 8) + _frame[7]);
  }

  void SetTimestamp(unsigned long timestamp) {
     if (_frameLen < 8)
       return;
     _frame[4] = (unsigned char) ((timestamp >> 24) & 0xff);
     _frame[5] = (unsigned char) ((timestamp >> 16) & 0xff);
     _frame[6] = (unsigned char) ((timestamp >> 8) & 0xff);
     _frame[7] = (unsigned char) (timestamp & 0xff);
  };

protected:
  unsigned char* _frame;
  int _frameLen;
};





H264DecoderContext::H264DecoderContext()
{
  flags = 0;
  
  _gotIFrame = false;
  _gotAGoodFrame = false;
  _frameCounter = 0; 
  _skippedFrameCounter = 0;
  _rxH264Frame = new H264Frame();
  _initflag = false;

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
		  }
	  }

	  FFMPEGLibraryInstance.AvcodecFree(_context);
	  FFMPEGLibraryInstance.AvcodecFree(_outputFrame);
  }
  if (_rxH264Frame) delete _rxH264Frame;

#if defined(_TEST_DRAW_WINDOW)
  sws_freeContext(_img_convert_ctx);
#endif
}

bool H264DecoderContext::Initialize()
{

	_rxH264Frame->BeginNewFrame();
	if(_initflag == true)
		return true;
	FFMPEGLibraryInstance.Load();
	if (!FFMPEGLibraryInstance.IsLoaded()) return false;

	//_codec =  avcodec_find_decoder(CODEC_ID_H264);
	if ((_codec = FFMPEGLibraryInstance.AvcodecFindDecoder(AV_CODEC_ID_H264)) == NULL) {
		return false;
	}

	//_context =   avcodec_alloc_context();
	_context = FFMPEGLibraryInstance.AvcodecAllocContext(_codec);
	if (_context == NULL) {
		return false;
	}

	//_outputFrame  = avcodec_alloc_frame();
	_outputFrame = FFMPEGLibraryInstance.AvcodecAllocFrame();
	if (_outputFrame == NULL) {
		return false;
	}

	//if(avcodec_open(_context, _codec)<0)
	//	return false;
	if (FFMPEGLibraryInstance.AvcodecOpen(_context, _codec) < 0) {
		return false;
	}
	_initflag = true;
	return true;
}


//传入AVFrame 
#if 0
AVFrame* H264DecoderContext::DecodeFrames(const u_char * src, unsigned & srcLen,AVFrame * Frame)
{
  RTPFrame srcRTP(src, srcLen);
  if (!_rxH264Frame->SetFromRTPFrame(srcRTP, flags)) {
	  _rxH264Frame->BeginNewFrame();
	  //sprintf(dst,"%s\n","setfromrtpframe is not ok!");
	  flags = (_gotAGoodFrame ? requestIFrame : 0);
	  _gotAGoodFrame = false;
	  return NULL;
  }

  if (srcRTP.GetMarker()==0)
  {
		return NULL;
  } 

  if (_rxH264Frame->GetFrameSize()==0)
  {
	  _rxH264Frame->BeginNewFrame();
	  _skippedFrameCounter++;
	  flags = (_gotAGoodFrame ? requestIFrame : 0);
	  _gotAGoodFrame = false;
	  return NULL;
  }
  // look and see if we have read an I frame.


  int gotPicture = 0;
 // uint32_t bytesUsed = 0;
//   int bytesDecoded = avcodec_decode_video(_context,Frame,&gotPicture,_rxH264Frame->GetFramePtr(),_rxH264Frame->GetFrameSize());
  int bytesDecoded = FFMPEGLibraryInstance.AvcodecDecodeVideo(_context, Frame, &gotPicture, _rxH264Frame->GetFramePtr(), _rxH264Frame->GetFrameSize());
  _rxH264Frame->BeginNewFrame();
  if (!gotPicture) 
  {
	  _skippedFrameCounter++;
	  flags = (_gotAGoodFrame ? requestIFrame : 0);

	  _gotAGoodFrame = false;
	  return NULL;
  }

 //得到了一帧
  // w  = _context->width;
  // h  = _context->height;
  flags = 1;
  _frameCounter++;
  _gotAGoodFrame = true;

  return Frame;
}
#endif
AVFrame* H264DecoderContext::DecodeFrames(const u_char * src, unsigned & srcLen)
{

  RTPFrame srcRTP(src, srcLen);
  if (!_rxH264Frame->SetFromRTPFrame(srcRTP, flags)) {
	  _rxH264Frame->BeginNewFrame();
	  //sprintf(dst,"%s\n","setfromrtpframe is not ok!");
	  flags = (_gotAGoodFrame ? requestIFrame : 0);
	  _gotAGoodFrame = false;
	  return NULL;
  }

 if (srcRTP.GetMarker()==0)
  {
		return NULL;
  } 

  if (_rxH264Frame->GetFrameSize()==0)
  {
	   DP0("drop packet now framesize is zero");
	  _rxH264Frame->BeginNewFrame();
	  _skippedFrameCounter++;
	  flags = (_gotAGoodFrame ? requestIFrame : 0);
	  _gotAGoodFrame = false;
	  return NULL;
  }
  // look and see if we have read an I frame.
  int gotPicture = 0;

  int bytesDecoded = FFMPEGLibraryInstance.AvcodecDecodeVideo(_context, _outputFrame, &gotPicture, _rxH264Frame->GetFramePtr(), _rxH264Frame->GetFrameSize());
  _rxH264Frame->BeginNewFrame();
  if (!gotPicture) 
  {
	  _skippedFrameCounter++;
	  flags = (_gotAGoodFrame ? requestIFrame : 0);
	  DP0("not get picture");
	  _gotAGoodFrame = false;
	  return NULL;
  }

 //得到了一帧
  // w  = _context->width;
  // h  = _context->height;
  flags = 1;
  _frameCounter++;
  _gotAGoodFrame = true;


#if defined(_TEST_DRAW_WINDOW) 
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
#endif

  return _outputFrame;
}




