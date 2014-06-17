

#include "stdafx.h"
#include "QB_RTP.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sys/timeb.h>

static unsigned long our_random32() {
	// Return a 32-bit random number.
	// Because "our_random()" returns a 31-bit random number, we call it a second
	// time, to generate the high bit:
	long random1 = rand();
	long random2 = rand();
	return 0;
	return (unsigned long)((random2<<31) | random1);
}
H264_RTP::H264_RTP(void)
{
	_seq_num = 0;
	_ts_current = 0;
	m_TimestampBase = our_random32();
}

H264_RTP::~H264_RTP(void)
{
	
}
void H264_RTP::Initialize(int fps,CGroupSock *groupsock)
{
    _fps = fps;
	_grousock=groupsock;
	
}
void H264_RTP::Close()
{

}
//void H264_RTP::Initialize(int fps,string ip,int port)
//{
//	_fps = fps;
//	_grousock=new CGroupSock();
//	_grousock->_isConnected=FALSE;
//	_grousock->JoinGroup(ip.c_str(),port);
//
//}
void H264_RTP::SendPakcet(char * buffer, int len)
{
	_grousock->SendTo(buffer,len);
}

static int gettimeofday(struct timeval* tp, int* /*tz*/) {
#if defined(_WIN32_WCE)
	/* FILETIME of Jan 1 1970 00:00:00. */
	static const unsigned __int64 epoch = 116444736000000000LL;

	FILETIME    file_time;
	SYSTEMTIME  system_time;
	ULARGE_INTEGER ularge;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart = file_time.dwLowDateTime;
	ularge.HighPart = file_time.dwHighDateTime;

	tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
#else
	static LARGE_INTEGER tickFrequency, epochOffset;

	// For our first call, use "ftime()", so that we get a time with a proper epoch.
	// For subsequent calls, use "QueryPerformanceCount()", because it's more fine-grain.
	static BOOL isFirstCall = TRUE;

	LARGE_INTEGER tickNow;
	QueryPerformanceCounter(&tickNow);

	if (isFirstCall) {
		struct timeb tb;
		ftime(&tb);
		tp->tv_sec = (long)tb.time;
		tp->tv_usec = 1000*tb.millitm;

		// Also get our counter frequency:
		QueryPerformanceFrequency(&tickFrequency);

		// And compute an offset to add to subsequent counter times, so we get a proper epoch:
		epochOffset.QuadPart
			= tb.time*tickFrequency.QuadPart + (tb.millitm*tickFrequency.QuadPart)/1000 - tickNow.QuadPart;

		isFirstCall = FALSE; // for next time
	} else {
		// Adjust our counter time so that we get a proper epoch:
		tickNow.QuadPart += epochOffset.QuadPart;

		tp->tv_sec = (long) (tickNow.QuadPart / tickFrequency.QuadPart);
		tp->tv_usec = (long) (((tickNow.QuadPart % tickFrequency.QuadPart) * 1000000L) / tickFrequency.QuadPart);
	}
#endif
	return 0;
}


UINT32 H264_RTP::convertToRTPTimestamp(/*struct timeval tv*/)
{
    timeval tv;
	gettimeofday(&tv, NULL);
	UINT32 timestampIncrement = (90000*tv.tv_sec);
	timestampIncrement += (UINT32)((2.0*90000*tv.tv_usec + 1000000.0)/2000000);   

	/*if (m_bNextTimestampHasBeenPreset)  
	{   
		m_TimestampBase -= timestampIncrement;
		m_bNextTimestampHasBeenPreset = FALSE;
	}*/


	UINT32 const rtpTimestamp = m_TimestampBase + timestampIncrement;  

	return rtpTimestamp;
}


void H264_RTP::SendOneNalunit2	(const unsigned char * nalu_start,int len)
{   
//	static
	memset(sendbuf,0,12);
	RTP_FIXED_HEADER        *rtp_hdr;  //RTP头部
	NALU_HEADER		        *nalu_hdr; //NALU头部

	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;

	//int len = n->len;
	//二种情况
	rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 
		//设置RTP HEADER，

	    //rtp固定包头，为12字节,该句将sendbuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入sendbuf。
		rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 
		//设置RTP HEADER，
		rtp_hdr->payload     = H264;  //负载类型号，
		rtp_hdr->version     = 2;  //版本号，此版本固定为2
		rtp_hdr->marker      = 0;   //标志位，由具体协议规定其值。
        rtp_hdr->ssrc        = htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一
	//	printf("rtp seqnum=%d\n",_seq_num);

    char FirstByte        = nalu_start[0];
	int F     = FirstByte & 0x80; //1 bit
	int NRI = FirstByte & 0x60; // 2 bit
	int TYPE     = FirstByte & 0x1f;// 5 bit
	    //当一个NALU小于1400字节的时候，采用一个单RTP包发送
		if(len<=1400)
		{	
			//设置rtp M 位；
		//	DEBUG("n<1400");
			rtp_hdr->marker=1;
		//	printf("rtp seqnum=%d\n",_seq_num);
			rtp_hdr->seq_no     = htons(_seq_num ++); //序列号，每发送一个RTP包增1
			//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
			nalu_hdr =(NALU_HEADER*)&sendbuf[12]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
			nalu_hdr->F=   F;
			nalu_hdr->NRI=NRI>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
			nalu_hdr->TYPE=TYPE;

			char *nalu_payload=&sendbuf[13];//同理将sendbuf[13]赋给nalu_payload
			memcpy(nalu_payload,nalu_start+1,len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。
		//printf("one all nalu\n");
		//	_ts_current=_ts_current+_timestamp_increse;
			rtp_hdr->timestamp=htonl(_ts_current);
			int bytes=len + 12 ;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节
			SendPakcet(sendbuf,bytes);
		}
		
		else if(len>1400)
		{
			//得到该nalu需要用多少长度为1400字节的RTP包来发送
			int k=0,l=0;
			k=len/1400;//需要k个1400字节的RTP包
			l=len%1400;//最后一个RTP包的需要装载的字节数
			int t=0;//用于指示当前发送的是第几个分片RTP包
			//_ts_current=_ts_current+_timestamp_increse;
			rtp_hdr->timestamp=htonl(_ts_current);
			while(t<=k)
			{
			//	printf("rtp seqnum=%d\n",_seq_num);
				//rtp_hdr->seq_no = htons(_seq_num ++); //序列号，每发送一个RTP包增1
				if(!t)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
				{
				//	DEBUG("len>1400 t<=k !t");
					//设置rtp M 位；
					rtp_hdr->marker=0;
					//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
					fu_ind->F=F;
					fu_ind->NRI=NRI>>5;
					fu_ind->TYPE=28;
					
					//设置FU HEADER,并将这个HEADER填入sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=1;
					fu_hdr->TYPE=TYPE;
					rtp_hdr->seq_no = htons(_seq_num ++);
					//	printf("head nalu\n");
					char *nalu_payload=&sendbuf[14];//同理将sendbuf[14]赋给nalu_payload
					memcpy(nalu_payload,nalu_start+1,1400);//去掉NALU头
					
					int bytes=1400+14;						//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节
					SendPakcet(sendbuf,bytes);
					t++;
					
				}
				//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
				else if(k==t)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
				{
				//	printf("end nalu end \n");
				//	DEBUG("len>1400 t<=k k==t");
					//设置rtp M 位；当前传输的是最后一个分片时该位置1
					rtp_hdr->marker=1;
					//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
					fu_ind->F=F;
					fu_ind->NRI=NRI>>5;
					fu_ind->TYPE=28;
						
					//设置FU HEADER,并将这个HEADER填入sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->TYPE=TYPE;
					fu_hdr->E=1;
					if(l-1>=0)
					{

					rtp_hdr->seq_no = htons(_seq_num ++);
					char *nalu_payload=&sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload
					memcpy(nalu_payload,nalu_start+t*1400+1,l-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
					//bytes=l-1+14;		//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节
					int bytes = l+14-1;
					SendPakcet(sendbuf,bytes);
						}
					
					t++;

				}
				else if(t<k&&0!=t)
				{
						//printf("body nalu end \n");
				//	DEBUG("len>1400 t<k&&0!=t");
					//设置rtp M 位；
					rtp_hdr->marker=0;
					//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
					fu_ind->F=F;
					fu_ind->NRI=NRI>>5;
					fu_ind->TYPE=28;
						
					//设置FU HEADER,并将这个HEADER填入sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					//fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->E=0;
					fu_hdr->TYPE=TYPE;
				    rtp_hdr->seq_no = htons(_seq_num ++);
					char *nalu_payload=&sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload
				//	printf("length - %u\n",t*1400+1);
					memcpy(nalu_payload,nalu_start+t*1400+1,1400);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
					int bytes=1400+14;						//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节
					SendPakcet(sendbuf,bytes);
					t++;
				}
			}
		}
}

unsigned char * FindStartCode( unsigned char *p,  unsigned char *end)
{

	 unsigned char *a = p + 4 - ((intptr_t)p & 3);

	for (end -= 3; p < a && p < end; p++) {
		if (p[0] == 0 && p[1] == 0 && p[2] == 1)
			return p;
	}
	for (end -= 3; p < end; p += 4) {
		unsigned int x = *(unsigned int*)p;
		if ((x - 0x01010101) & (~x) & 0x80808080) { // generic
			if (p[1] == 0) {
				if (p[0] == 0 && p[2] == 1)
					return p;
				if (p[2] == 0 && p[3] == 1)
					return p+1;
			}
			if (p[3] == 0) {
				if (p[2] == 0 && p[4] == 1)
					return p+2;
				if (p[4] == 0 && p[5] == 1)
					return p+3;
			}
		}
	}

	for (end += 3; p < end; p++) {
		if (p[0] == 0 && p[1] == 0 && p[2] == 1)
			return p;
	}

	return end + 3;
}
 unsigned char * find_startcode( unsigned char *p,  unsigned char *end){
	 unsigned char *out= FindStartCode(p, end);
	if(p<out && out<end && !out[-1]) out--;
	return out;
}

void H264_RTP::SendOneFrame(const unsigned char* framebuffer,int framelen,int SendContinue )
{

	int i = 0;
	unsigned char * pos = (unsigned char*)framebuffer;
	int headnum = 0;
	unsigned char * END = pos + framelen;
	int if_end = 0;
	unsigned char * posend = NULL;
	//_timestamp_increse=(unsigned int)(90000.0 / _fps); 
	if( SendContinue == 0) //如果不是继续发送的包，则时间戳要增加
	{
	  //_ts_current=_ts_current+_timestamp_increse;	
	  _ts_current = convertToRTPTimestamp();
	}
	pos = find_startcode(pos,END);//寻找起始位置的头
	while(pos<END)
	{
		posend = find_startcode(pos+1,END);
		if_end=END-posend;
		if(if_end > 0)
		{
			SendOneNalunit2(pos,posend-pos);
		
			pos = posend;
		}
		else
		{
			SendOneNalunit2(pos,framebuffer+framelen-pos);
			pos=NULL;
			break;
		}
		
	}

}
