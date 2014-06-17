
// Decode1Dlg.h : 头文件
//

#pragma once

#include "../librtprece/CodeReceive3.h"
#include "decode.h"
// CDecode1Dlg 对话框
class CDecode1Dlg : public CDialogEx
{
// 构造


private:
	CodeReceive3 _receive;

public:
	//void Draw(AVFrame* frame,int pix_fmt,int srcW,int srcH);
public:
	CDecode1Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_DECODE1_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();


};
