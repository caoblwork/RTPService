/*!
/file WASocket.h
/brief 定义WASocket和WASockEvent的头文件
/author	ecop
/date	2006/12/26
*/
#ifndef __WA_PS_SOCKET_BASE__
#define __WA_PS_SOCKET_BASE__

/// 使用在WASockEvent的Wait事件上，用来表示无限等待
#define WASOCK_INFINITE -1

#define WA_SEVENT_ERROR		-1
#define WA_SEVENT_TIMEOUT	0
#define WA_SEVENT_OK		1

///该类是WebAccess PolicyService中Socket操作的基类。
/*!
WASocket封装了标准的socket操作，支持TCP/UDP。
*/

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#include "assert.h"
#define ASSERT assert 

class CWASocket
{
private:

	///socket端口
	int m_port;

	///socket句柄
	SOCKET m_socket;

	///socket类型
	int m_SockType;

	///本定绑定的地址信息。
    struct sockaddr_in m_SockAddr; 

public:
	///构造函数
	CWASocket(int port = 0) {m_socket = INVALID_SOCKET; m_port = port;}

	virtual ~CWASocket() { Close();}

public:
	///初始化Windows Socket Lib的函数。
	static BOOL Initialize()
	{
		WSADATA wsaData;
		return !WSAStartup(0x101, &wsaData);
	}

	///清理Windows socket lib的函数。
	static void Free(){ WSACleanup();}

public:
	
	///将当前的socket设置为阻塞或非模式
	BOOL SetNonBlock(BOOL isnonblock)
	{
		ASSERT(m_socket != INVALID_SOCKET);

		if (ioctlsocket(m_socket,FIONBIO,(unsigned long *)&isnonblock) == -1)
			return FALSE;
		else
			return TRUE;
	}
	
	///设置当前的socket可以重用地址功能。
	BOOL SetReuseAddr(BOOL isreuse)
	{
		ASSERT(m_socket != INVALID_SOCKET);
		if ( setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&isreuse,sizeof(int)) == -1 )
			return FALSE;
		else
			return TRUE;
	}
	
	///打开该socket上的keepalive选项,是的tcp协议栈检测keepalive信息。
	BOOL SetKeepAlive(BOOL iskeep)
	{
		ASSERT(m_socket != INVALID_SOCKET);
		if ( setsockopt(m_socket,SOL_SOCKET,SO_KEEPALIVE,(char *)&iskeep,sizeof(int)) == -1 )
			return FALSE;
		else
			return TRUE;
	}
	
	BOOL SetBroadCastEnable(BOOL isBroadCast= TRUE)
	{
		ASSERT(m_SockType == SOCK_DGRAM);
		if(setsockopt(m_socket,SOL_SOCKET,SO_BROADCAST,(const char *)&isBroadCast,sizeof(int)) == SOCKET_ERROR)
			return FALSE;
		return TRUE;
	}

	///根据指定的类型创建udp或tcp socket。
	virtual BOOL Create(int sockettype = SOCK_STREAM , int protocoltype = 0 )
	{
		m_SockType = sockettype;
		m_socket = socket(PF_INET,sockettype,protocoltype);
		if (m_socket == INVALID_SOCKET)
			return FALSE;

		int optVal = 1024*1024*2 ;//2M字节的缓冲
		int optLen = sizeof(int);
		setsockopt(m_socket, SOL_SOCKET,SO_RCVBUF,(char*)&optVal,optLen );

		return TRUE;
	}

	///tcp发送信息。
	virtual BOOL Send(const void *buf,int buflen)
	{
		ASSERT(m_socket != INVALID_SOCKET);
		return (send(m_socket, (LPSTR)buf, buflen, 0) != SOCKET_ERROR);
	}
	
	///udp发送信息。
	virtual int SendTo(const void *buf,int buflen,LPCTSTR destination,short port)
	{
		SOCKADDR_IN sockAddr;

		ASSERT(m_socket != INVALID_SOCKET);

		memset(&sockAddr,0,sizeof(sockAddr));

		LPSTR lpszAscii = (LPTSTR)destination;
		sockAddr.sin_port = htons(port);
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);

		if (sockAddr.sin_addr.s_addr == INADDR_NONE)
		{
			LPHOSTENT lphost;
			lphost = gethostbyname(lpszAscii);
			if (lphost != NULL)
				sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
			else
			{
				WSASetLastError(WSAEINVAL);
				return FALSE;
			}
		}

		return (::sendto(m_socket,(LPSTR)buf,buflen,0,(sockaddr*)&sockAddr,sizeof(sockAddr)) != SOCKET_ERROR);
	}

	///tcp接收信息
	virtual int Recv(void *buf,int buflen)
	{
		ASSERT(m_socket != INVALID_SOCKET);
		return recv(m_socket, (LPSTR)buf, buflen, 0);
	}

	virtual int RecvFrom(void * buf,int buflen)
	{
		sockaddr_in SenderAddr;
		int SenderAddrSize = sizeof(SenderAddr);
		
		return recvfrom(m_socket, (char*)buf, buflen, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
	}
	
	///作为tcp服务器或着使用udp方式通讯时，将socket绑定到端口。
	virtual BOOL Bind(int port = 0)
	{
		SOCKADDR_IN sockAddr;
		ASSERT(m_socket != INVALID_SOCKET);
		memset(&sockAddr,0,sizeof(sockAddr));
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		if(port != 0)
		{
			m_port = port;
			sockAddr.sin_port = htons((u_short)m_port);
		}
		return (SOCKET_ERROR != bind(m_socket, (struct sockaddr*)&sockAddr, sizeof(sockAddr)));
	}

	///为当前WASocket对象的socket句柄赋值
	void Attach(SOCKET soc) { m_socket = soc; }

	///返回当前的socket句柄，并将socket从当前对象中释放。
	SOCKET Dettach(){ 
		SOCKET tmp = m_socket;
        m_socket = INVALID_SOCKET;
		return tmp;
	}

	///得到当前对象的socket句柄。
	SOCKET GetSocket() { return m_socket;}

	///作为服务器时开始监听客户端链接。
	BOOL ListenToClient(int backlog = 5)
	{
		ASSERT(m_socket != INVALID_SOCKET);
		return (SOCKET_ERROR != listen(m_socket, backlog));
	}

	///关闭当前socket。
	virtual void Close()
	{
		if(m_socket != INVALID_SOCKET)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
		m_SockType = 0;
	}

	///接收客户端链接。
	BOOL AcceptClient(CWASocket& client,const void* buf = NULL,int buflen = 0)
	{
		SOCKET soc;
		ASSERT(m_socket != INVALID_SOCKET);

		soc = accept(m_socket,NULL,NULL);
		if(soc == INVALID_SOCKET)
			return FALSE;

		if(buf != NULL && buflen > 0)
			send(soc,(const char*)buf,buflen,0);

		client.Attach(soc);
		return TRUE;
	}

	///作为客户端时，链接到指定的server。
	virtual int Connect(LPCTSTR lpszHostAddress, UINT nHostPort)
	{
		SOCKADDR_IN sockAddr;

		assert(m_socket != INVALID_SOCKET);

		memset(&sockAddr,0,sizeof(sockAddr));

		LPSTR lpszAscii = (LPTSTR)lpszHostAddress;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);

		if (sockAddr.sin_addr.s_addr == INADDR_NONE)
		{
			LPHOSTENT lphost;
			lphost = gethostbyname(lpszAscii);
			if (lphost != NULL)
				sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
			else
			{
				WSASetLastError(WSAEINVAL);
				return FALSE;
			}
		}

		sockAddr.sin_port = htons((u_short)nHostPort);

		return (connect(m_socket,(struct sockaddr*)&sockAddr, sizeof(sockAddr)) != SOCKET_ERROR);
	}

	virtual int Connect(unsigned long HostAddress,UINT nHostPort)
	{
		SOCKADDR_IN sockAddr;

		ASSERT(m_socket != INVALID_SOCKET);

		memset(&sockAddr,0,sizeof(sockAddr));

		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = HostAddress;
		sockAddr.sin_port = htons((u_short)nHostPort);

		return (connect(m_socket,(struct sockaddr*)&sockAddr, sizeof(sockAddr)) != SOCKET_ERROR);
	}
	
	///这个方法是从pj naughter的SMTPSocket中移植过来的，目的是为了能够使用WASocket替换其SMTPSocket。
	BOOL  IsReadible(BOOL& bReadible)
	{
		timeval timeout = {0, 0};
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(m_socket, &fds);
		int nStatus = select(0, &fds, NULL, NULL, &timeout);
		if (nStatus == SOCKET_ERROR)
		{
			return FALSE;
		}
		else
		{
			bReadible = !(nStatus == 0);
			return TRUE;
		}

	}
};

///该类是模拟Windows内核对象方法，提供对异步socket事件的监视功能
/*!
实际上该类是通过封装socket api中的FD_SET数据结构和select调用形成的。
*/

class CWASocketEvent
{
private:
	///select调用处理的对象。
	fd_set m_event;

public:
	///构造函数
	CWASocketEvent(){ FD_ZERO(&m_event); }
	~CWASocketEvent() { }

public:
	///获得当前对象的select处理对象句柄。
	FD_SET* GetEvent() {return &m_event;}

	/*
	将指定的WASocket或者其派生对象绑定到当前对象上。类似FD_SET
	\param[in] soc 需要设置到当前Event对象的WASocket对象或其派生对象的引用
	\return 无
	\sa IsSomethingHappen
	*/
	void SetSocketEvent( CWASocket &soc)
	{
		ASSERT(soc.GetSocket() != INVALID_SOCKET);

		FD_SET(soc.GetSocket(),&m_event);
	}

	/*!
	检测socket上否发生了事件。类似FD_ISSET
	\param[in] soc WASocket对象或其派生对象的引用
	\return 无
	\sa IsSomethingHappen
	*/
	BOOL IsSomethingHappen( CWASocket &soc)
	{
		ASSERT(soc.GetSocket() != INVALID_SOCKET);

		return FD_ISSET(soc.GetSocket(),&m_event);
	}

	///重置处理对象句柄，类似FD_ZERO
	void ResetEvent()
	{
		FD_ZERO(&m_event);
	}

	/*!
	等待指定的事件发生的函数。
	WaitForSocketEvents可以同时检测两个WASocketEvent对象，分别
	对应socket读、写事件，与select调用。
	\param[in,out] read	设置了读事件的对象引用
	\param[in,out] write 设置了写事件的对象引用
	\param[in] millisecond	等待超时的事件，单位为微秒,当需要阻
	塞时，使用WASOCK_INIFINITE.
	\return int 当出现异常错误时返回值小于0，超时返回0，否则返回非0。
	\sa IsSomethingHappen
	*/
	static int WaitForSocketEvents(CWASocketEvent *read,CWASocketEvent *write = NULL,int millisecond = 0)
	{
		return WaitForSocketEvents(read,write,NULL,millisecond);
	}

	static int WaitForSocketEvents(CWASocketEvent *read,CWASocketEvent *write , CWASocketEvent *except,int millisecond )
	{
		struct timeval t;
		struct timeval *tp;
		int result = -1;
		FD_SET *wfd ,*efd ;

		wfd = NULL;
		efd = NULL;
		tp = NULL;

		if(millisecond !=-1)
		{
			t.tv_sec = millisecond/1000;
			t.tv_usec = millisecond;
			tp = &t;
		}

		if(write)
			wfd = write->GetEvent();

		if(except)
			efd = except->GetEvent();

		result = select(0,read->GetEvent(),wfd,efd,tp);

		if(result == SOCKET_ERROR)
			return WA_SEVENT_ERROR;
		else
			return result;
	}

};
#endif