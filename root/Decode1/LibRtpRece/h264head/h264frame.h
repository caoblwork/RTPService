#ifndef __H264FRAME_H__
#define __H264FRAME_H__ 1

#include <stdint.h>
#include "rtpframe.h"

#define H264_NAL_TYPE_NON_IDR_SLICE 1
#define H264_NAL_TYPE_DP_A_SLICE 2
#define H264_NAL_TYPE_DP_B_SLICE 3
#define H264_NAL_TYPE_DP_C_SLICE 0x4
#define H264_NAL_TYPE_IDR_SLICE 0x5
#define H264_NAL_TYPE_SEI 0x6
#define H264_NAL_TYPE_SEQ_PARAM 0x7
#define H264_NAL_TYPE_PIC_PARAM 0x8
#define H264_NAL_TYPE_ACCESS_UNIT 0x9
#define H264_NAL_TYPE_END_OF_SEQ 0xa
#define H264_NAL_TYPE_END_OF_STREAM 0xb
#define H264_NAL_TYPE_FILLER_DATA 0xc
#define H264_NAL_TYPE_SEQ_EXTENSION 0xd






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

class H264Frame
{
public:
  H264Frame();
  ~H264Frame();

  void BeginNewFrame();
//#ifndef LICENCE_MPL
//  void SetFromFrame (x264_nal_t *NALs, int numberOfNALs);
//#endif
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
  bool EncapsulateSTAP  (RTPFrame & frame, unsigned int & flags);
  bool EncapsulateFU    (RTPFrame & frame, unsigned int & flags);

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

#endif /* __H264FRAME_H__ */
