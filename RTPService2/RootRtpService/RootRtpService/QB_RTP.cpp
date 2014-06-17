

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
	RTP_FIXED_HEADER        *rtp_hdr;  //RTPͷ��
	NALU_HEADER		        *nalu_hdr; //NALUͷ��

	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;

	//int len = n->len;
	//�������
	rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 
		//����RTP HEADER��

	    //rtp�̶���ͷ��Ϊ12�ֽ�,�þ佫sendbuf[0]�ĵ�ַ����rtp_hdr���Ժ��rtp_hdr��д�������ֱ��д��sendbuf��
		rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 
		//����RTP HEADER��
		rtp_hdr->payload     = H264;  //�������ͺţ�
		rtp_hdr->version     = 2;  //�汾�ţ��˰汾�̶�Ϊ2
		rtp_hdr->marker      = 0;   //��־λ���ɾ���Э��涨��ֵ��
        rtp_hdr->ssrc        = htonl(10);    //���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ
	//	printf("rtp seqnum=%d\n",_seq_num);

    char FirstByte        = nalu_start[0];
	int F     = FirstByte & 0x80; //1 bit
	int NRI = FirstByte & 0x60; // 2 bit
	int TYPE     = FirstByte & 0x1f;// 5 bit
	    //��һ��NALUС��1400�ֽڵ�ʱ�򣬲���һ����RTP������
		if(len<=1400)
		{	
			//����rtp M λ��
		//	DEBUG("n<1400");
			rtp_hdr->marker=1;
		//	printf("rtp seqnum=%d\n",_seq_num);
			rtp_hdr->seq_no     = htons(_seq_num ++); //���кţ�ÿ����һ��RTP����1
			//����NALU HEADER,�������HEADER����sendbuf[12]
			nalu_hdr =(NALU_HEADER*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����nalu_hdr��֮���nalu_hdr��д��ͽ�д��sendbuf�У�
			nalu_hdr->F=   F;
			nalu_hdr->NRI=NRI>>5;//��Ч������n->nal_reference_idc�ĵ�6��7λ����Ҫ����5λ���ܽ���ֵ����nalu_hdr->NRI��
			nalu_hdr->TYPE=TYPE;

			char *nalu_payload=&sendbuf[13];//ͬ��sendbuf[13]����nalu_payload
			memcpy(nalu_payload,nalu_start+1,len-1);//ȥ��naluͷ��naluʣ������д��sendbuf[13]��ʼ���ַ�����
		//printf("one all nalu\n");
		//	_ts_current=_ts_current+_timestamp_increse;
			rtp_hdr->timestamp=htonl(_ts_current);
			int bytes=len + 12 ;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ�����NALUͷ����ȥ��ʼǰ׺������rtp_header�Ĺ̶�����12�ֽ�
			SendPakcet(sendbuf,bytes);
		}
		
		else if(len>1400)
		{
			//�õ���nalu��Ҫ�ö��ٳ���Ϊ1400�ֽڵ�RTP��������
			int k=0,l=0;
			k=len/1400;//��Ҫk��1400�ֽڵ�RTP��
			l=len%1400;//���һ��RTP������Ҫװ�ص��ֽ���
			int t=0;//����ָʾ��ǰ���͵��ǵڼ�����ƬRTP��
			//_ts_current=_ts_current+_timestamp_increse;
			rtp_hdr->timestamp=htonl(_ts_current);
			while(t<=k)
			{
			//	printf("rtp seqnum=%d\n",_seq_num);
				//rtp_hdr->seq_no = htons(_seq_num ++); //���кţ�ÿ����һ��RTP����1
				if(!t)//����һ����Ҫ��Ƭ��NALU�ĵ�һ����Ƭ����FU HEADER��Sλ
				{
				//	DEBUG("len>1400 t<=k !t");
					//����rtp M λ��
					rtp_hdr->marker=0;
					//����FU INDICATOR,�������HEADER����sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
					fu_ind->F=F;
					fu_ind->NRI=NRI>>5;
					fu_ind->TYPE=28;
					
					//����FU HEADER,�������HEADER����sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=1;
					fu_hdr->TYPE=TYPE;
					rtp_hdr->seq_no = htons(_seq_num ++);
					//	printf("head nalu\n");
					char *nalu_payload=&sendbuf[14];//ͬ��sendbuf[14]����nalu_payload
					memcpy(nalu_payload,nalu_start+1,1400);//ȥ��NALUͷ
					
					int bytes=1400+14;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥ��ʼǰ׺��NALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�
					SendPakcet(sendbuf,bytes);
					t++;
					
				}
				//����һ����Ҫ��Ƭ��NALU�ķǵ�һ����Ƭ������FU HEADER��Sλ������÷�Ƭ�Ǹ�NALU�����һ����Ƭ����FU HEADER��Eλ
				else if(k==t)//���͵������һ����Ƭ��ע�����һ����Ƭ�ĳ��ȿ��ܳ���1400�ֽڣ���l>1386ʱ����
				{
				//	printf("end nalu end \n");
				//	DEBUG("len>1400 t<=k k==t");
					//����rtp M λ����ǰ����������һ����Ƭʱ��λ��1
					rtp_hdr->marker=1;
					//����FU INDICATOR,�������HEADER����sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
					fu_ind->F=F;
					fu_ind->NRI=NRI>>5;
					fu_ind->TYPE=28;
						
					//����FU HEADER,�������HEADER����sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->TYPE=TYPE;
					fu_hdr->E=1;
					if(l-1>=0)
					{

					rtp_hdr->seq_no = htons(_seq_num ++);
					char *nalu_payload=&sendbuf[14];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
					memcpy(nalu_payload,nalu_start+t*1400+1,l-1);//��nalu���ʣ���l-1(ȥ����һ���ֽڵ�NALUͷ)�ֽ�����д��sendbuf[14]��ʼ���ַ�����
					//bytes=l-1+14;		//���sendbuf�ĳ���,Ϊʣ��nalu�ĳ���l-1����rtp_header��FU_INDICATOR,FU_HEADER������ͷ��14�ֽ�
					int bytes = l+14-1;
					SendPakcet(sendbuf,bytes);
						}
					
					t++;

				}
				else if(t<k&&0!=t)
				{
						//printf("body nalu end \n");
				//	DEBUG("len>1400 t<k&&0!=t");
					//����rtp M λ��
					rtp_hdr->marker=0;
					//����FU INDICATOR,�������HEADER����sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
					fu_ind->F=F;
					fu_ind->NRI=NRI>>5;
					fu_ind->TYPE=28;
						
					//����FU HEADER,�������HEADER����sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					//fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->E=0;
					fu_hdr->TYPE=TYPE;
				    rtp_hdr->seq_no = htons(_seq_num ++);
					char *nalu_payload=&sendbuf[14];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
				//	printf("length - %u\n",t*1400+1);
					memcpy(nalu_payload,nalu_start+t*1400+1,1400);//ȥ����ʼǰ׺��naluʣ������д��sendbuf[14]��ʼ���ַ�����
					int bytes=1400+14;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥԭNALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�
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
	if( SendContinue == 0) //������Ǽ������͵İ�����ʱ���Ҫ����
	{
	  //_ts_current=_ts_current+_timestamp_increse;	
	  _ts_current = convertToRTPTimestamp();
	}
	pos = find_startcode(pos,END);//Ѱ����ʼλ�õ�ͷ
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
