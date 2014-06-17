/*****************************************************************************/
/* The contents of this file are subject to the Mozilla Public License       */
/* Version 1.0 (the "License"); you may not use this file except in          */
/* compliance with the License.  You may obtain a copy of the License at     */
/* http://www.mozilla.org/MPL/                                               */
/*                                                                           */
/* Software distributed under the License is distributed on an "AS IS"       */
/* basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the  */
/* License for the specific language governing rights and limitations under  */
/* the License.                                                              */
/*                                                                           */
/* The Original Code is the Open H323 Library.                               */
/*                                                                           */
/* The Initial Developer of the Original Code is Matthias Schneider          */
/* Copyright (C) 2007 Matthias Schneider, All Rights Reserved.               */
/*                                                                           */
/* Contributor(s): Matthias Schneider (ma30002000@yahoo.de)                  */
/*                                                                           */
/* Alternatively, the contents of this file may be used under the terms of   */
/* the GNU General Public License Version 2 or later (the "GPL"), in which   */
/* case the provisions of the GPL are applicable instead of those above.  If */
/* you wish to allow use of your version of this file only under the terms   */
/* of the GPL and not to allow others to use your version of this file under */
/* the MPL, indicate your decision by deleting the provisions above and      */
/* replace them with the notice and other provisions required by the GPL.    */
/* If you do not delete the provisions above, a recipient may use your       */
/* version of this file under either the MPL or the GPL.                     */
/*                                                                           */
/* The Original Code was written by Matthias Schneider <ma30002000@yahoo.de> */
/*****************************************************************************/
#include "stdafx.h"

#include "h264frame.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <atlstr.h>
#define MAX_FRAME_SIZE 1024*1024*4

H264Frame::H264Frame ()
{
  _timestamp = 0;
  _maxPayloadSize = 1400;
  _encodedFrame = (uint8_t*)malloc(MAX_FRAME_SIZE);
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
}






bool H264Frame::SetFromRTPFrame(RTPFrame & frame, unsigned int & flags) {
  uint8_t curNALType = *(frame.GetPayloadPtr()) & 0x1f;

  if (curNALType >= H264_NAL_TYPE_NON_IDR_SLICE &&
      curNALType <= H264_NAL_TYPE_FILLER_DATA)
  {
    // regular NAL - put in buffer, adding the header.
    //TRACEI_UP(4, "H264\tDeencap\tDeencapsulating a regular NAL unit NAL of " << frame.GetPayloadSize() - 1 << " bytes (type " << (int) curNALType << ")");
    AddDataToEncodedFrame(frame.GetPayloadPtr() + 1, frame.GetPayloadSize() - 1, *(frame.GetPayloadPtr()), 1);
  } 
  else if (curNALType == 24) 
  {
    // stap-A (single time aggregation packet )
    return DeencapsulateSTAP (frame, flags);
  } 
  else if (curNALType == 28) 
  {
    // Fragmentation Units
    return DeencapsulateFU (frame, flags);
  }
  else
  {
    //TRACEI_UP(4, "H264\tDeencap\tSkipping unsupported NAL unit type " << curNALType);
    return false;
  }
  return true;
}


bool H264Frame::DeencapsulateSTAP (RTPFrame & frame, unsigned int & /*flags*/) {
  uint8_t* curSTAP = frame.GetPayloadPtr() + 1;
  uint32_t curSTAPLen = frame.GetPayloadSize() - 1; 

  //TRACEI_UP(4, "H264\tDeencap\tDeencapsulating a STAP of " << curSTAPLen << " bytes");
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
