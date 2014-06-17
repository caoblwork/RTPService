#include "StdAfx.h"
#include "h264.h"
#include "FFMPEGLibrary.h"


FFMPEGLibrary FFMPEGLibraryInstance;


#define MAX_FRAME_SIZE 1024*2*1024

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



H264Frame::H264Frame ()
{
	_timestamp = 0;
	_maxPayloadSize = 1400;
	_encodedFrame = (uint8_t*)malloc(MAX_FRAME_SIZE);
	_NALs = NULL;
	_numberOfNALsReserved = 0;

	BeginNewFrame();
}

void H264Frame::BeginNewFrame ()
{
	_encodedFrameLen = 0;

	_numberOfNALsInFrame = 0;
	_currentNAL = 0; 

	_currentNALFURemainingLen = 0;
	_currentNALFURemainingDataPtr = NULL;
	_currentNALFUHeader0 = 0;
	_currentNALFUHeader1 = 0;

	_currentFU = 0;

}

H264Frame::~H264Frame ()
{
	if (_encodedFrame) free (_encodedFrame);
	if (_NALs) free(_NALs);
}

#if 0
bool H264Frame::GetRTPFrame(RTPFrame & frame, unsigned int & flags)
{
	flags = 0;
	flags |= (IsSync()) ? isIFrame : 0;
	if (_currentNAL < _numberOfNALsInFrame) 
	{ 
		uint32_t curNALLen = _NALs[_currentNAL].length;
		const uint8_t *curNALPtr = _encodedFrame + _NALs[_currentNAL].offset;
		/*
		* We have 3 types of packets we can send:
		* fragmentation units - if the NAL is > max_payload_size
		* single nal units - if the NAL is < max_payload_size, and can only fit 1 NAL
		* single time aggregation units - if we can put multiple NALs into one packet
		*
		* We don't send multiple time aggregation units
		*/

		if (curNALLen > _maxPayloadSize)
		{
			// fragmentation unit - break up into max_payload_size size chunks
			return EncapsulateFU(frame, flags);
		} 
		else 
		{
			// it is the last NAL of that frame or doesnt fit into an STAP packet with next nal ?
#ifdef SEND_STAP_PACKETS


			if (((_currentNAL + 1) >= _numberOfNALsInFrame)  ||  
				((curNALLen + _NALs[_currentNAL + 1].length + 5) > _maxPayloadSize)) 
			{ 
#endif
				// single nal unit packet

				frame.SetPayloadSize(curNALLen);
				memcpy(frame.GetPayloadPtr(), curNALPtr, curNALLen);
				frame.SetTimestamp(_timestamp);
				frame.SetMarker((_currentNAL + 1) >= _numberOfNALsInFrame ? 1 : 0);
				flags |= frame.GetMarker() ? isLastFrame : 0;  // marker bit on last frame of video

				//TRACEI_UP(4, "H264\tEncap\tEncapsulating NAL unit #" << _currentNAL << "/" << (_numberOfNALsInFrame-1) << " of " << curNALLen << " bytes as a regular NAL unit");
				_currentNAL++;
				return true;
#ifdef SEND_STAP_PACKETS
			} 
			else
			{
				return EncapsulateSTAP(frame, flags); 
			}
#endif
		}
	} 
	else 
	{
		return false;
	}
}

bool H264Frame::EncapsulateSTAP (RTPFrame & frame, unsigned int & flags) {
	uint32_t STAPLen = 1;
	uint32_t highestNALNumberInSTAP = _currentNAL;

	// first check how many nals we want to put into the packet
	do {
		STAPLen += 2;
		STAPLen +=  _NALs[highestNALNumberInSTAP].length;
		highestNALNumberInSTAP++;
	} while (highestNALNumberInSTAP < _numberOfNALsInFrame && STAPLen < _maxPayloadSize);

	if (STAPLen > _maxPayloadSize)
	{
		STAPLen -= 2;
		STAPLen -= _NALs[(highestNALNumberInSTAP-1)].length;
		highestNALNumberInSTAP--;
	}

	//TRACEI_UP(4, "H264\tEncap\tEncapsulating NAL units " << _currentNAL << "-"<< (highestNALNumberInSTAP-1) << "/" << (_numberOfNALsInFrame-1) << " as a STAP of " << STAPLen);

	frame.SetPayloadSize(1); // for stap header

	uint32_t curNALLen;
	const uint8_t* curNALPtr;
	uint8_t  maxNRI = 0;
	while (_currentNAL < highestNALNumberInSTAP) {
		curNALLen = _NALs[_currentNAL].length;
		curNALPtr = _encodedFrame + _NALs[_currentNAL].offset;

		// store the nal length information
		frame.SetPayloadSize(frame.GetPayloadSize() + 2);
		*((uint8_t*)frame.GetPayloadPtr() + frame.GetPayloadSize() - 2) = curNALLen >> 8;
		*((uint8_t*)frame.GetPayloadPtr() + frame.GetPayloadSize() - 1) = curNALLen & 0xff;

		// store the nal
		frame.SetPayloadSize(frame.GetPayloadSize() + curNALLen);
		memcpy ((uint8_t*)frame.GetPayloadPtr() + frame.GetPayloadSize() - curNALLen, (void *)curNALPtr, curNALLen);

		if ((*curNALPtr & 0x60) > maxNRI) maxNRI = *curNALPtr & 0x60;
		//TRACEI_UP(4, "H264\tEncap\tAdding NAL unit " << _currentNAL << "/" << (_numberOfNALsInFrame-1) << " of " << curNALLen << " bytes to STAP");
		_currentNAL++;
	}

	// set the nri value in the stap header
	//uint8_t stap = 24 | maxNRI;
	//memcpy (frame.GetPayloadPtr(),&stap,1);
	memset (frame.GetPayloadPtr(), 24 | maxNRI, 1);
	frame.SetTimestamp(_timestamp);
	frame.SetMarker(_currentNAL >= _numberOfNALsInFrame ? 1 : 0);
	flags |= frame.GetMarker() ? isLastFrame : 0;  // marker bit on last frame of video

	return true;
}


bool H264Frame::EncapsulateFU(RTPFrame & frame, unsigned int & flags) {
	uint8_t header[2];
	uint32_t curFULen;

	if ((_currentNALFURemainingLen==0) || (_currentNALFURemainingDataPtr==NULL))
	{
		_currentNALFURemainingLen = _NALs[_currentNAL].length;
		_currentNALFURemainingDataPtr = _encodedFrame + _NALs[_currentNAL].offset;
		_currentNALFUHeader0 = (*_currentNALFURemainingDataPtr & 0x60) | 28;
		_currentNALFUHeader1 = *_currentNALFURemainingDataPtr & 0x1f;
		header[0] = _currentNALFUHeader0;
		header[1] = 0x80 | _currentNALFUHeader1; // s indication
		_currentNALFURemainingDataPtr++; // remove the first byte
		_currentNALFURemainingLen--;
	}
	else
	{
		header[0] = _currentNALFUHeader0;
		header[1] = _currentNALFUHeader1;
	}

	if (_currentNALFURemainingLen > 0)
	{
		bool last = false;
		if ((_currentNALFURemainingLen + 2) <= _maxPayloadSize)
		{
			header[1] |= 0x40;
			curFULen = _currentNALFURemainingLen;
			last = true;
		}
		else
		{
			curFULen = _maxPayloadSize - 2;
		}


		frame.SetPayloadSize(curFULen + 2);
		memcpy ((uint8_t*)frame.GetPayloadPtr(), header, 2);
		memcpy ((uint8_t*)frame.GetPayloadPtr()+2, _currentNALFURemainingDataPtr, curFULen);
		frame.SetTimestamp(_timestamp);
		frame.SetMarker((last && ((_currentNAL+1) >= _numberOfNALsInFrame)) ? 1 : 0);
		flags |= frame.GetMarker() ? isLastFrame : 0;  // marker bit on last frame of video

		_currentNALFURemainingDataPtr += curFULen;
		_currentNALFURemainingLen -= curFULen;
	} 
	if (_currentNALFURemainingLen==0)
	{
		_currentNAL++;
		_currentNALFURemainingDataPtr=NULL;
	}
	return true;
}
#endif
bool H264Frame::SetFromRTPFrame(RTPFrame & frame, unsigned int & flags) {
	uint8_t curNALType = *(frame.GetPayloadPtr()) & 0x1f;

	if (curNALType >= H264_NAL_TYPE_NON_IDR_SLICE &&
		curNALType <= H264_NAL_TYPE_FILLER_DATA)
	{
	
		DP1("the nal type is %d, supported, add",curNALType);
		AddDataToEncodedFrame(frame.GetPayloadPtr() + 1, frame.GetPayloadSize() - 1, *(frame.GetPayloadPtr()), 1);
	} 
	else if (curNALType == 24)  //stap-A
	{
		// stap-A (single time aggregation packet )
		DP1("the nal type is %d, supported",curNALType);
		return DeencapsulateSTAP (frame, flags);
	} 
	else if (curNALType == 28) //FU-A
	{
		// Fragmentation Units
		DP1("the nal type is %d, supported",curNALType);
		return DeencapsulateFU (frame, flags);
	}
	else
	{
		DP1("the nal type is %d, not supported",curNALType);
		return false;
	}

	return true;
}
bool H264Frame::IsSync () {
	uint32_t i;

	for (i=0; i<_numberOfNALsInFrame; i++)
	{
		if ((_NALs[i].type == H264_NAL_TYPE_IDR_SLICE) ||
			(_NALs[i].type == H264_NAL_TYPE_SEQ_PARAM) ||
			(_NALs[i].type == H264_NAL_TYPE_PIC_PARAM))
		{
			return true;
		}
	}
	return false;
}

//单时间聚合包(STAP)包含的是同一帧的数据
bool H264Frame::DeencapsulateSTAP (RTPFrame & frame, unsigned int & /*flags*/) {
	uint8_t* curSTAP = frame.GetPayloadPtr() + 1;
	uint32_t curSTAPLen = frame.GetPayloadSize() - 1; 

	while (curSTAPLen > 0)
	{
		// first, theres a 2 byte length field
		uint32_t len = (curSTAP[0] << 8) | curSTAP[1];
		curSTAP += 2;
		// then the header, followed by the body.  We'll add the header
		// in the AddDataToEncodedFrame - that's why the nal body is dptr + 1
		//TRACEI_UP(4, "H264\tDeencap\tDeencapsulating an NAL unit of " << len << " bytes (type " << (int)(*curSTAP && 0x1f) << ") from STAP");
		AddDataToEncodedFrame(curSTAP + 1,  len - 1, *curSTAP, 1);
		curSTAP += len;
		if ((len + 2) > curSTAPLen)
		{
			curSTAPLen = 0;
			//TRACEI(1, "H264\tDeencap\tError deencapsulating STAP, STAP header says its " << len + 2 << " bytes long but there are only " << curSTAPLen << " bytes left of the packet");
			return false;
		}
		else
		{
			curSTAPLen -= (len + 2);
		}
	}
	return true;
}

bool H264Frame::DeencapsulateFU (RTPFrame & frame, unsigned int & /*flags*/) 
{
	//FU指示字节有以下格式：
	//     +---------------+
	//     |0|1|2|3|4|5|6|7|
	//     +-+-+-+-+-+-+-+-+
	//     |F|NRI|  Type   |
	//     +---------------+
	//  FU指示字节的类型域的28，29表示FU-A和FU-B。F的使用在5。3描述。NRI域的值必须根据分片NAL单元的NRI域的值设置。
	//  FU头的格式如下：
	//     +---------------+
	//     |0|1|2|3|4|5|6|7|
	//     +-+-+-+-+-+-+-+-+
	//     |S|E|R|  Type   |
	//     +---------------+

	//  S: 1 bit
	//     当设置成1,开始位指示分片NAL单元的开始。当跟随的FU荷载不是分片NAL单元荷载的开始，开始位设为0。
	//  E: 1 bit
	//     当设置成1, 结束位指示分片NAL单元的结束，即, 荷载的最后字节也是分片NAL单元的最后一个字节。当跟随的
	//     FU荷载不是分片NAL单元的最后分片,结束位设置为0。
	//  R: 1 bit
	//     保留位必须设置为0，接收者必须忽略该位。
	//     
	//  Type: 5 bits
	//     NAL单元荷载类型定义在[1]的表7-1.
	uint8_t* curFUPtr = frame.GetPayloadPtr();
	uint32_t curFULen = frame.GetPayloadSize(); 
	uint8_t header;
	//S为1 E不为1 为开始第一个分片
	if ((curFUPtr[1] & 0x80) && !(curFUPtr[1] & 0x40))
	{
		//TRACEI_UP(4, "H264\tDeencap\tDeencapsulating a FU of " << frame.GetPayloadSize() - 1 << " bytes (_Startbit_, !Endbit)");
		if (_currentFU) 
		{
			_currentFU=1;
		}
		else
		{
			_currentFU++;
			//0xe0 11100000 0x1f 00011111
			header = (curFUPtr[0] & 0xe0) | (curFUPtr[1] & 0x1f);
			AddDataToEncodedFrame(curFUPtr + 2, curFULen - 2, header,  1);
		}
	} 
	else if (!(curFUPtr[1] & 0x80) && !(curFUPtr[1] & 0x40))
	{
		//TRACEI_UP(4, "H264\tDeencap\tDeencapsulating a FU of " << frame.GetPayloadSize() - 1 << " bytes (!Startbit, !Endbit)");
		if (_currentFU)
		{
			_currentFU++;
			AddDataToEncodedFrame(curFUPtr + 2, curFULen - 2,  0, 0);
		}
		else
		{
			_currentFU=0;
			//TRACEI(1, "H264\tDeencap\tReceived an intermediate FU without getting the first - dropping!");
			return false;
		}
	} 
	else if (!(curFUPtr[1] & 0x80) && (curFUPtr[1] & 0x40))
	{
		//TRACEI_UP(4, "H264\tDeencap\tDeencapsulating a FU of " << frame.GetPayloadSize() - 1 << " bytes (!Startbit, _Endbit_)");
		if (_currentFU) {
			_currentFU=0;
			AddDataToEncodedFrame( curFUPtr + 2, curFULen - 2, 0, 0);
		}
		else
		{
			_currentFU=0;
			//TRACEI(1, "H264\tDeencap\tReceived a last FU without getting the first - dropping!");
			return false;
		}
	} 
	else if ((curFUPtr[1] & 0x80) && (curFUPtr[1] & 0x40))
	{
		//TRACEI_UP(4, "H264\tDeencap\tDeencapsulating a FU of " << frame.GetPayloadSize() - 1 << " bytes (_Startbit_, _Endbit_)");
		//TRACEI(1, "H264\tDeencap\tReceived a FU with both Starbit and Endbit set - This MUST NOT happen!");
		_currentFU=0;
		return false;
	} 
	return true;
}

void H264Frame::AddDataToEncodedFrame (uint8_t *data, uint32_t dataLen, uint8_t header, bool addHeader) {
	uint8_t headerLen= addHeader ? 5 : 0;
	uint8_t* currentPositionInFrame = _encodedFrame + _encodedFrameLen;

	// add 00 00 00 01 [headerbyte] header
	if (addHeader)
	{
		*currentPositionInFrame++ = 0;
		*currentPositionInFrame++ = 0;
		*currentPositionInFrame++ = 0;
		*currentPositionInFrame++ = 1;

		if (_numberOfNALsInFrame + 1 >(_numberOfNALsReserved))
		{
			_NALs = (h264_nal_t *)realloc(_NALs, (_numberOfNALsReserved + 1) * sizeof(h264_nal_t));
			_numberOfNALsReserved++;
		}
		if (_NALs)
		{
			_NALs[_numberOfNALsInFrame].offset = _encodedFrameLen + 4;
			_NALs[_numberOfNALsInFrame].length = dataLen + 1;
			_NALs[_numberOfNALsInFrame].type = header & 0x1f;


			_numberOfNALsInFrame++;
		}

		*currentPositionInFrame++ = header;
	}
	else
	{
		if (_NALs) _NALs[_numberOfNALsInFrame - 1].length += dataLen;
	}

	memcpy(currentPositionInFrame, data, dataLen);
	_encodedFrameLen += dataLen + headerLen;
}

bool H264Frame::IsStartCode (const uint8_t *positionInFrame)
{
	if (positionInFrame[0] == 0 &&
		positionInFrame[1] == 0 &&
		((positionInFrame[2] == 1) ||
		((positionInFrame[2] == 0) && positionInFrame[3] == 1))) 
	{
		return true;
	}
	return false;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

//retlen返回长度
char * H264DecoderContext::DecodeFrames_1(const u_char * src, unsigned & srcLen, unsigned int & retlen)
{
	RTPFrame srcRTP(src, srcLen);
	if (!_rxH264Frame->SetFromRTPFrame(srcRTP, flags)) {
		_rxH264Frame->BeginNewFrame();
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
	retlen = _rxH264Frame->GetFrameSize();

	//接收完了一帧
	_rxH264Frame->BeginNewFrame();

	flags = 1;
	_frameCounter++;
	_gotAGoodFrame = true;

	return (char*)_rxH264Frame->GetFramePtr();
	//返回一帧的数据
}


AVFrame* H264DecoderContext::DecodeFrames_2(const u_char * src, unsigned & srcLen)
{
	int gotPicture = 0;
	int bytesDecoded = FFMPEGLibraryInstance.AvcodecDecodeVideo(_context, _outputFrame, &gotPicture, (BYTE*)src, srcLen);
	if (!gotPicture) 
	{
		_skippedFrameCounter++;
		flags = (_gotAGoodFrame ? requestIFrame : 0);
		DP0("not get picture");
		_gotAGoodFrame = false;
		return NULL;
  }

  flags = 1;
  _frameCounter++;
  _gotAGoodFrame = true;

  return _outputFrame;
}

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

  return _outputFrame;
}


