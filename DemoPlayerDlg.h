// DemoPlayerDlg.h : 头文件
//

#pragma once
#include "d3d_render_interface.h"

// CDemoPlayerDlg 对话框
class CDemoPlayerDlg : public CDialog
{
// 构造
public:
	CDemoPlayerDlg(CWnd* pParent = NULL);	// 标准构造函数
	CStatic m_view[4];
	Cd3d_render_interface m_render[4];
	HANDLE udpRecvThreadHandle;

	static UINT WINAPI udpRecvThread(PVOID pM);
	
	

// 对话框数据
	enum { IDD = IDD_DEMOPLAYER_DIALOG };

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
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
