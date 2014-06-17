/*!
/file WASocket.h
/brief ����WASocket��WASockEvent��ͷ�ļ�
/author	ecop
/date	2006/12/26
*/
#ifndef __WA_PS_SOCKET_BASE__
#define __WA_PS_SOCKET_BASE__

/// ʹ����WASockEvent��Wait�¼��ϣ�������ʾ���޵ȴ�
#define WASOCK_INFINITE -1

#define WA_SEVENT_ERROR		-1
#define WA_SEVENT_TIMEOUT	0
#define WA_SEVENT_OK		1

///������WebAccess PolicyService��Socket�����Ļ��ࡣ
/*!
WASocket��װ�˱�׼��socket������֧��TCP/UDP��
*/

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#include "assert.h"
#define ASSERT assert 

class CWASocket
{
private:

	///socket�˿�
	int m_port;

	///socket���
	SOCKET m_socket;

	///socket����
	int m_SockType;

	///�����󶨵ĵ�ַ��Ϣ��
    struct sockaddr_in m_SockAddr; 

public:
	///���캯��
	CWASocket(int port = 0) {m_socket = INVALID_SOCKET; m_port = port;}

	virtual ~CWASocket() { Close();}

public:
	///��ʼ��Windows Socket Lib�ĺ�����
	static BOOL Initialize()
	{
		WSADATA wsaData;
		return !WSAStartup(0x101, &wsaData);
	}

	///����Windows socket lib�ĺ�����
	static void Free(){ WSACleanup();}

public:
	
	///����ǰ��socket����Ϊ�������ģʽ
	BOOL SetNonBlock(BOOL isnonblock)
	{
		ASSERT(m_socket != INVALID_SOCKET);

		if (ioctlsocket(m_socket,FIONBIO,(unsigned long *)&isnonblock) == -1)
			return FALSE;
		else
			return TRUE;
	}
	
	///���õ�ǰ��socket�������õ�ַ���ܡ�
	BOOL SetReuseAddr(BOOL isreuse)
	{
		ASSERT(m_socket != INVALID_SOCKET);
		if ( setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&isreuse,sizeof(int)) == -1 )
			return FALSE;
		else
			return TRUE;
	}
	
	///�򿪸�socket�ϵ�keepaliveѡ��,�ǵ�tcpЭ��ջ���keepalive��Ϣ��
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

	///����ָ�������ʹ���udp��tcp socket��
	virtual BOOL Create(int sockettype = SOCK_STREAM , int protocoltype = 0 )
	{
		m_SockType = sockettype;
		m_socket = socket(PF_INET,sockettype,protocoltype);
		if (m_socket == INVALID_SOCKET)
			return FALSE;

		int optVal = 1024*1024*2 ;//2M�ֽڵĻ���
		int optLen = sizeof(int);
		setsockopt(m_socket, SOL_SOCKET,SO_RCVBUF,(char*)&optVal,optLen );

		return TRUE;
	}

	///tcp������Ϣ��
	virtual BOOL Send(const void *buf,int buflen)
	{
		ASSERT(m_socket != INVALID_SOCKET);
		return (send(m_socket, (LPSTR)buf, buflen, 0) != SOCKET_ERROR);
	}
	
	///udp������Ϣ��
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

	///tcp������Ϣ
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
	
	///��Ϊtcp����������ʹ��udp��ʽͨѶʱ����socket�󶨵��˿ڡ�
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

	///Ϊ��ǰWASocket�����socket�����ֵ
	void Attach(SOCKET soc) { m_socket = soc; }

	///���ص�ǰ��socket���������socket�ӵ�ǰ�������ͷš�
	SOCKET Dettach(){ 
		SOCKET tmp = m_socket;
        m_socket = INVALID_SOCKET;
		return tmp;
	}

	///�õ���ǰ�����socket�����
	SOCKET GetSocket() { return m_socket;}

	///��Ϊ������ʱ��ʼ�����ͻ������ӡ�
	BOOL ListenToClient(int backlog = 5)
	{
		ASSERT(m_socket != INVALID_SOCKET);
		return (SOCKET_ERROR != listen(m_socket, backlog));
	}

	///�رյ�ǰsocket��
	virtual void Close()
	{
		if(m_socket != INVALID_SOCKET)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
		m_SockType = 0;
	}

	///���տͻ������ӡ�
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

	///��Ϊ�ͻ���ʱ�����ӵ�ָ����server��
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
	
	///��������Ǵ�pj naughter��SMTPSocket����ֲ�����ģ�Ŀ����Ϊ���ܹ�ʹ��WASocket�滻��SMTPSocket��
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

///������ģ��Windows�ں˶��󷽷����ṩ���첽socket�¼��ļ��ӹ���
/*!
ʵ���ϸ�����ͨ����װsocket api�е�FD_SET���ݽṹ��select�����γɵġ�
*/

class CWASocketEvent
{
private:
	///select���ô���Ķ���
	fd_set m_event;

public:
	///���캯��
	CWASocketEvent(){ FD_ZERO(&m_event); }
	~CWASocketEvent() { }

public:
	///��õ�ǰ�����select�����������
	FD_SET* GetEvent() {return &m_event;}

	/*
	��ָ����WASocket��������������󶨵���ǰ�����ϡ�����FD_SET
	\param[in] soc ��Ҫ���õ���ǰEvent�����WASocket��������������������
	\return ��
	\sa IsSomethingHappen
	*/
	void SetSocketEvent( CWASocket &soc)
	{
		ASSERT(soc.GetSocket() != INVALID_SOCKET);

		FD_SET(soc.GetSocket(),&m_event);
	}

	/*!
	���socket�Ϸ������¼�������FD_ISSET
	\param[in] soc WASocket��������������������
	\return ��
	\sa IsSomethingHappen
	*/
	BOOL IsSomethingHappen( CWASocket &soc)
	{
		ASSERT(soc.GetSocket() != INVALID_SOCKET);

		return FD_ISSET(soc.GetSocket(),&m_event);
	}

	///���ô��������������FD_ZERO
	void ResetEvent()
	{
		FD_ZERO(&m_event);
	}

	/*!
	�ȴ�ָ�����¼������ĺ�����
	WaitForSocketEvents����ͬʱ�������WASocketEvent���󣬷ֱ�
	��Ӧsocket����д�¼�����select���á�
	\param[in,out] read	�����˶��¼��Ķ�������
	\param[in,out] write ������д�¼��Ķ�������
	\param[in] millisecond	�ȴ���ʱ���¼�����λΪ΢��,����Ҫ��
	��ʱ��ʹ��WASOCK_INIFINITE.
	\return int �������쳣����ʱ����ֵС��0����ʱ����0�����򷵻ط�0��
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