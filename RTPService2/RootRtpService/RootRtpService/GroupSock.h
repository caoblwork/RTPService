/*
* author :qianbo
* function���鲥�֧࣬��IOCP��ɶ˿ڶ�ȡ����
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

		SOCKADDR_IN stLclAddr, stDstAddr;//����������Զ�̵�ַ
		struct ip_mreq stMreq;        /* �鲥 �ӿ� */

		HANDLE _hCompletionPort; //��ɶ˿�
		HANDLE _hReadEvent;
		OVERLAPPED Overlapped;
#else //�ڶ�����ɶ˿ڽ�������


#endif
		////////////////////////////////////////////////////
		//����Ϊ�ڶ���ಥͨ�ţ�

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
		//���溯��Ϊѡ�ã����Դ���һ��IOCP�˿ڶ�ȡ���ݣ����Ҵ�������߳�ȥά��,Ĭ��Ϊһ�߳�
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