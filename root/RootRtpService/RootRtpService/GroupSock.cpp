#include "stdafx.h"
#include "GroupSock.h"
#include <stdio.h>


#pragma comment(lib,"ws2_32.lib")


	static const BOOL      bLoopBack = FALSE;    // 屏蔽回播
  
	static HANDLE g_hCompletionPort; //完成端口
	static HANDLE g_hReadEvent;

	//测试使用
	DWORD               dwFlag = 0 ;

	bool CGroupSock ::_Initialize = false;


	CGroupSock::CGroupSock(void)
	{
		_len		 = sizeof(struct sockaddr_in);
	}

	CGroupSock::~CGroupSock(void)
	{
	}


	BOOL CGroupSock::InitWinsock2()
	{
#ifdef USE_VERONE
		WSADATA data;
		WORD version;
		int ret = 0;	

		version = (MAKEWORD(2, 2));
		ret = WSAStartup(version, &data);
		if (ret != 0)
		{
			ret = WSAGetLastError();
			if (ret == WSANOTINITIALISED)
			{
				//printf("not initialised");
				//LOG
				return FALSE;
			}
		}
#else
		if (WSAStartup(MAKEWORD(2, 2), &_wsd) != 0)
		{
			printf("WSAStartup() failed\n");
			return FALSE;
		}
#endif
		return TRUE;

	}

	BOOL CGroupSock::Initialize(int RS)
	{
		_Receive_Send = RS;
		if(_Initialize==FALSE)
		{
			_isConnected = FALSE;
			_Initialize=true;
			return InitWinsock2();
		}
		
		else return TRUE;
	}

	void CGroupSock::UnInitialize()
	{
#ifdef USE_VERONE
		closesocket(_hSocket);
#else
		closesocket(_sock);
#endif
	//	WSACleanup();
	}

	BOOL CGroupSock::JoinGroup(const char* ip,int port)
	{

		//////////////////////////////////////////////////////////
		//以下为第二版windows多播通信,加入了根通信
		if ((_sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0,
			WSA_FLAG_MULTIPOINT_C_LEAF 
			| WSA_FLAG_MULTIPOINT_D_LEAF 
			| WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		{
			//printf("socket failed with: %d\n", WSAGetLastError());
			WSACleanup();
			return FALSE;
		}
		// Bind to the local interface. This is done to receive data.
		_local.sin_family = AF_INET;
		//_local.sin_port   = 0;//htons(port);
		_local.sin_port   = 0;
		if(_Receive_Send == SOCKM_REV)
			_local.sin_port   = htons(port);

		_local.sin_addr.s_addr = INADDR_ANY;

		char optval = 1;
		if(setsockopt(_sock,SOL_SOCKET,SO_REUSEADDR,(const char *)&optval,sizeof(int))<0)
		{
			return FALSE; 
		}

		if (bind(_sock, (struct sockaddr *)&_local, 
			sizeof(_local)) == SOCKET_ERROR)
		{
			//printf("bind failed with: %d\n", WSAGetLastError());
			closesocket(_sock);
			WSACleanup();
			return FALSE;
		}
		// Setup the SOCKADDR_IN structure describing the multicast 
		// group we want to join
		//
		_remote.sin_family             = AF_INET;
		_remote.sin_port                = htons(port);
		_remote.sin_addr.s_addr     = inet_addr(ip);
		//
		// Change the TTL to something more appropriate
		//
		_optval = 32;
		if (setsockopt(_sock, IPPROTO_IP, IP_MULTICAST_TTL, 
			(char *)&_optval, sizeof(int)) == SOCKET_ERROR)
		{
			//printf("setsockopt(IP_MULTICAST_TTL) failed: %d\n",
			//	WSAGetLastError());
			closesocket(_sock);
			WSACleanup();
			return FALSE;
		}
		// Disable loopback if needed
		//
		_optval = 0;
		if (setsockopt(_sock, IPPROTO_IP, IP_MULTICAST_LOOP,
			(char *)&_optval, sizeof(_optval)) == SOCKET_ERROR)
		{
		//	printf("setsockopt(IP_MULTICAST_LOOP) failed: %d\n",
			//	WSAGetLastError());
			closesocket(_sock);
			WSACleanup();
			return FALSE;
		}
		// 加入多播组.  
		// 
		// 
		// 
		//
		if ((_sockM = WSAJoinLeaf(_sock, (SOCKADDR *)&_remote, 
			sizeof(_remote), NULL, NULL, NULL, NULL, 
			JL_BOTH)) == INVALID_SOCKET)
		{
			//printf("WSAJoinLeaf() failed: %d\n", WSAGetLastError());
			closesocket(_sock);
			WSACleanup();
			_isConnected = FALSE;
			return FALSE;
		}
		_isConnected = TRUE;

		return TRUE;



	}

	BOOL CGroupSock::SendTo(const char *pBuf, int len)
	{
		////测试使用
		////sprintf(_sendbuf, "CodePlayerServer 1: This is a test: %d", dwFlag++);
		if (sendto(_sock, pBuf, len, 0,
			(struct sockaddr *)&_remote, 
			sizeof(_remote)) == SOCKET_ERROR)
		{
			printf("sendto failed with: %d\n",WSAGetLastError());
			closesocket(_sockM);
			closesocket(_sock);
			WSACleanup();
			return FALSE;
		}
		//else
		//{
			//printf("send this is a test %s\n",pBuf);
	//	}
		//printf ("send data is %s\n",pBuf);
		return TRUE;
	}


//	void CGroupSock::Send(const char* pBuf,int len)
//	{
//#ifdef USE_VERONE
//		int nRet = sendto(_hSocket, pBuf, len, 0,(struct sockaddr*)&stDstAddr, sizeof(stDstAddr));
//		if (nRet < 0) 
//		{
//			printf ("sendto() failed, Error: %d\n", WSAGetLastError());
//			return ;
//		} 
//#else
//#endif
//	}

	int CGroupSock::ReceiveData(char *pBuf,int BufLen)
	{

		int ret = 0;
		if ((ret = recvfrom(_sock, pBuf, BufLen, 0,
			(struct sockaddr *)&_from, &_len)) == SOCKET_ERROR)
		{
			//printf("recvfrom failed with: %d\n", WSAGetLastError());
			closesocket(_sockM);
			closesocket(_sock);
			WSACleanup();
			return -1;
		}

		return ret;
	}
	

#ifdef USE_VERONE
#else
	BOOL CGroupSock::CreateIOCPPrepare()
	{
		g_hReadEvent      = CreateEvent(NULL,TRUE,TRUE,NULL); 
		g_hCompletionPort = CreateIoCompletionPort (INVALID_HANDLE_VALUE,NULL,0,10);
		if (!g_hCompletionPort)
		{
			//log
			return FALSE;
		}

		CreateIoCompletionPort ((HANDLE)_sock,g_hCompletionPort,(DWORD)_sock,10);

		OVERLAPPED Overlapped;
		Overlapped.hEvent = g_hReadEvent;
		Overlapped.Internal		= 0;
		Overlapped.InternalHigh = 0;
		Overlapped.Offset		= 0;
		Overlapped.OffsetHigh	= 0;

		char			achInBuf [BUFSIZE];
		DWORD nbytes;
		DWORD Flags = 0;

		WSABUF DataBuf;
		DataBuf.len = BUFSIZE;
		DataBuf.buf = &achInBuf[0];

		sockaddr_in SenderAddr;
		int SenderAddrSize = sizeof(SenderAddr);

		int nRet = WSARecvFrom(_sock,&DataBuf,1,&nbytes,&Flags,(SOCKADDR *)&SenderAddr,&SenderAddrSize,&Overlapped,NULL);
		if(nRet == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
		{
			//printf("error first post!\n");
		}
		return TRUE;	

	}


	BOOL HandleIncomingData(char * pBuf,int DataLen)
	{
		pBuf[DataLen]='\0';
	//	printf("receive %s %d\n",pBuf,DataLen);
		return TRUE;
	}

	DWORD WINAPI CGroupSock::WorkerThread (LPVOID WorkContext)
	{
		DWORD nSocket;
		BOOL b;
		OVERLAPPED ovl;
		LPOVERLAPPED lpo=&ovl;
		DWORD nBytesRead=0;
		DWORD nBytesToBeRead;
		DWORD dwErrCode ;
		char ReadBuffer[BUFSIZE];

		WSABUF DataBuf;
		DataBuf.len =  BUFSIZE;
		DataBuf.buf =  ReadBuffer;
		DWORD BytesRecv = 0, Flags = 0, BytesTransferred = 0;
		WSAOVERLAPPED Overlapped;
		//sockaddr_in RecvAddr;
		sockaddr_in SenderAddr;
		int SenderAddrSize = sizeof(SenderAddr);

		memset(&ReadBuffer,0,BUFSIZE);

		CGroupSock * gs = (CGroupSock *)WorkContext;
		for (;;)
		{
			b = GetQueuedCompletionStatus (g_hCompletionPort,&nBytesToBeRead,&nSocket,&lpo,INFINITE);
			if (b || lpo)
			{
				if (b)
				{

					OVERLAPPED ol;
					ol.hEvent		=	g_hReadEvent;
					ol.Offset		=	0;
					ol.OffsetHigh	=	0;
					//WSARecvFrom(nSocket,&DataBuf,1,&nBytesToBeRead,&Flags,(SOCKADDR *)&SenderAddr,&SenderAddrSize,&Overlapped,NULL);
					//HandleIncomingData(DataBuf.buf,nBytesRead);
					b = ReadFile ((HANDLE)nSocket,&ReadBuffer,nBytesToBeRead,&nBytesRead,&ol);
					if (!b ) 
					{
						dwErrCode = GetLastError();
						if( dwErrCode  != ERROR_IO_PENDING )
						{
							//OutputDebugString((LPCTSTR)"error read!");
							//printf("error read!\n");
						}
						else if( dwErrCode  == ERROR_IO_PENDING )
						{
							WaitForSingleObject(ol.hEvent,INFINITE);

							HandleIncomingData(ReadBuffer,nBytesToBeRead);
						}
					}
					else
					{
						HandleIncomingData(ReadBuffer,nBytesToBeRead);
					}

					continue;
				}
				else
				{
					dwErrCode = GetLastError();
					LPVOID lpMsgBuf;
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						dwErrCode ,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &lpMsgBuf,
						0,
						NULL 
						);

					//OutputDebugString((LPCTSTR)lpMsgBuf);
				//	printf("%s\n",lpMsgBuf);

					// Free the buffer.
					LocalFree( lpMsgBuf );
					continue;
				}
			}
			else
			{
				//printf("error!\n");
				continue;
			}
			return 1;
		}
	}

	//-----------------------------------------------------------------------------


	BOOL CGroupSock::CreateIOCPClientThread(int ThreadNum )
	{

		DWORD ThreadId;
		HANDLE ThreadHandle;

		for (int i = 0; i < ThreadNum; i++)
		{
			ThreadHandle = CreateThread (NULL,0,WorkerThread,NULL,0,&ThreadId);
			if (!ThreadHandle)
			{
				//LOG
				return FALSE;
			}

			CloseHandle (ThreadHandle);
		}
		return TRUE;
	}

#endif