
// DrawHwndDlg.h : ͷ�ļ�
//

#pragma once
#include "../libRtpReceive/DrawPacket.h"

// CDrawHwndDlg �Ի���
class CDrawHwndDlg : public CDialogEx
{
// ����
public:
	CDrawHwndDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_DRAWHWND_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

	//UdpAgentServer server1;
	DrawPacket _PacketServer;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
