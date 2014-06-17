#pragma once
typedef unsigned char u_char;

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

}
enum codecInFlags {
  silenceFrame      = 1,
  forceIFrame       = 2
};
    
enum codecOutFlags {
  isLastFrame     = 1,
  isIFrame        = 2,
  requestIFrame   = 4
};


typedef struct h264_nal_t
{
  uint32_t offset;
  uint32_t length;
  uint8_t  type;
} h264_nal_t;

class RTPFrame {
public:
  RTPFrame(const unsigned char * frame, int frameLen) {
    _frame = (unsigned char*) frame;
    _frameLen = frameLen;
  };

  RTPFrame(unsigned char * frame, int frameLen, unsigned char payloadType) {
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
	{
      return false;
	}
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

class H264Frame
{
public:
  H264Frame();
  ~H264Frame();

  void BeginNewFrame();

  void SetMaxPayloadSize (uint16_t maxPayloadSize) 
  {
    _maxPayloadSize = maxPayloadSize;
  }
  void SetTimestamp (uint64_t timestamp) 
  {
    _timestamp = timestamp;
  }
  bool GetRTPFrame (RTPFrame & frame, unsigned int & flags);
  bool HasRTPFrames ()
  {
    if (_currentNAL < _numberOfNALsInFrame) return true; else return false;
  }

  bool SetFromRTPFrame (RTPFrame & frame, unsigned int & flags);
  uint8_t* GetFramePtr ()
  {
    return (_encodedFrame);
  }
  uint32_t GetFrameSize () {
    return (_encodedFrameLen);
  }
  bool IsSync ();
  
private:
  //bool EncapsulateSTAP  (RTPFrame & frame, unsigned int & flags);
  //bool EncapsulateFU    (RTPFrame & frame, unsigned int & flags);

  bool DeencapsulateFU   (RTPFrame & frame, unsigned int & flags);
  bool DeencapsulateSTAP (RTPFrame & frame, unsigned int & flags);
  void AddDataToEncodedFrame (uint8_t *data, uint32_t dataLen, uint8_t header, bool addHeader);
  bool IsStartCode (const uint8_t *positionInFrame);
    // general stuff
  uint64_t _timestamp;
  uint16_t _maxPayloadSize;
  uint8_t* _encodedFrame;
  uint32_t _encodedFrameLen;

  h264_nal_t* _NALs;
  uint32_t _numberOfNALsInFrame;
  uint32_t _currentNAL; 
  uint32_t _numberOfNALsReserved;
  
  // for encapsulation
  uint32_t _currentNALFURemainingLen;
  uint8_t* _currentNALFURemainingDataPtr;
  uint8_t  _currentNALFUHeader0;
  uint8_t  _currentNALFUHeader1;

  // for deencapsulation
  uint16_t _currentFU;
};

class H264DecoderContext
{
  public:
    H264DecoderContext();
    ~H264DecoderContext();
    AVFrame* DecodeFrames(const u_char * src, unsigned & srcLen);

	char * DecodeFrames_1(const u_char * src, unsigned & srcLen, unsigned int & retlen);
	AVFrame* DecodeFrames_2(const u_char * src, unsigned & srcLen);
	bool Initialize();
    AVCodecContext * GetContext()
	{
		return _context;
	}
  protected:

    AVCodec* _codec;
    AVCodecContext* _context;
    AVFrame* _outputFrame;
    H264Frame* _rxH264Frame;

   
	bool _gotIFrame;
    bool _gotAGoodFrame;
    int _frameCounter;
    int _skippedFrameCounter;
	unsigned int flags;

	bool _initflag;

};