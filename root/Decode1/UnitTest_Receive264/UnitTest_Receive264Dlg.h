
// UnitTest_Receive264Dlg.h : ͷ�ļ�
//

#pragma once


#include "CodeReceive.h"
// CUnitTest_Receive264Dlg �Ի���
class CUnitTest_Receive264Dlg : public CDialog
{
// ����
public:
	CUnitTest_Receive264Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_UNITTEST_RECEIVE264_DIALOG };

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
private:

	CCodeReceive _Receive;
public:
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnBnClickedBtnStop();


	afx_msg void OnBnClickedBtnTest();
};
