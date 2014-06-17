/*
* author :qianbo
* function：组播类，支持IOCP完成端口读取数据
*/

#ifndef __GROUPSOCK_H_
#define __GROUPSOCK_H_


#include <winsock2.h>
#include <ws2tcpip.h>
#define BUFSIZE 1500

#define SOCKM_REV 0
#define SOCKM_SND 1
	class CGroupSock
	{
	private:
#ifdef USE_VERONE
		SOCKET _hSocket;

		SOCKADDR_IN stLclAddr, stDstAddr;//本地网卡和远程地址
		struct ip_mreq stMreq;        /* 组播 接口 */

		HANDLE _hCompletionPort; //完成端口
		HANDLE _hReadEvent;
		OVERLAPPED Overlapped;
#else //第二版完成端口接收数据


#endif
		////////////////////////////////////////////////////
		//以下为第二版多播通信，

		WSADATA             _wsd;
		struct sockaddr_in  _local,
			_remote,
			_from;
		SOCKET              _sock, _sockM;
		int                 _len ;//= sizeof(struct sockaddr_in),
		int                 _optval;
		int                 _ret;

		int _Receive_Send; //(0,1)
	private:
		BOOL _isConnected;
	protected:
		BOOL InitWinsock2();

		BOOL CreateIOCPPrepare();

		static DWORD WINAPI WorkerThread(LPVOID para);

		BOOL CreateIOCPClientThread(int ThreadNum);

	public:
	
		BOOL Initialize(int RS);

		BOOL JoinGroup(const char* ip = NULL,int port = 0);

		BOOL SendTo(const char *pBuf,int nlen);

	//	void Send(const char* pBuf,int len);

		 int ReceiveData(char *pBuf,int bufferlen);
		void UnInitialize();
		static bool _Initialize;

#ifdef USE_VERONE
#else
		//下面函数为选用，可以创建一个IOCP端口读取数据，并且创建多个线程去维护,默认为一线程
		BOOL StartIOCPClient(int tNum=1)
		{
			if(CreateIOCPPrepare())
			{
				if(CreateIOCPClientThread(tNum))
				{
					//log
					return TRUE;
				}
			}
		//	printf("error start IOCPClient receive!\n");
			return FALSE;
		}
#endif
	public:
		CGroupSock(void);

		~CGroupSock(void);
	};



#endif