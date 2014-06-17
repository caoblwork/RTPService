
#ifndef _H264_RTP_H_
#define _H264_RTP_H_

#include "h264.h"
#include "GroupSock.h"


typedef struct
{
  int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
  unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  unsigned max_size;            //! Nal Unit Buffer size
  int forbidden_bit;            //! should be always FALSE 在 H.264 规范中规定了这一位必须为 0.
  int nal_reference_idc;        //! NALU_PRIORITY_xxxx   

  int nal_unit_type;            //! NALU_TYPE_xxxx    
  char *buf;                    //! contains the first byte followed by the EBSP
  unsigned short lost_packets;  //! true, if packet loss is detected
} NALU_t;



class H264_RTP
{
private:
	int _fps;
	//char buf[100000];
	int  buflen;
	unsigned int _timestamp_increse;
	unsigned int _ts_current;
	unsigned short _seq_num;

	char sendbuf[1500];

public:
	CGroupSock *_grousock;
	H264_RTP(void);
	~H264_RTP(void);
public:
	//sendcontinue 是指这个包和上一个包是否是同一帧，是相同帧则为1，主要解决时间交错模式时间戳是否增加，交错模式可以分为两个包发送
	//第2个包过来时不用增加时间戳
	void SendOneFrame(const unsigned char* framebuffer,int framelen,int SendContinue = 0);
	void Initialize(int fps,CGroupSock *groupsock);
	//void Initialize(int fps,string ip ,int port);
    BOOL SendFile(const char * filename);
	void Close();
protected:
	void SendOneNalunit2(const unsigned char * nalu_start,int len);
	void SendPakcet(char * buffer, int len);

	BOOL InitWinsock();
	UINT32 convertToRTPTimestamp(/*struct timeval tv*/);
private:
	unsigned long m_TimestampBase ;

};
#endif