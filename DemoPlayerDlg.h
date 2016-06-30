// DemoPlayerDlg.h : 头文件
//

#pragma once
#include "d3d_render_interface.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "tchart_temperature.h"

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
	afx_msg void OnBnClickedButtonB1cal();
	afx_msg void OnBnClickedButtonB1calof30();
	afx_msg void OnBnClickedButtonB2calof80();
	afx_msg void OnBnClickedButtonfindbp();
	CStatic m_view_tempbar;
	CStatic m_view_tempfloatbar;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	COLORREF m_bluecolor,m_redcolor,m_greencolor,m_blackcolor;
	afx_msg void OnDrawBlock(CDC* pDC,int pXY);
	afx_msg void OnDrawBlock2(CDC* pDC,int posYY);
	CListCtrl m_list_posavr;
	CTchart_temperature m_chart_temperature;
	CTchart_temperature m_chart_frequency;
	CTchart_temperature m_chart_hotcold;
	afx_msg void OnAddisothermInterval();
	afx_msg void OnAddisothermAbove();
	afx_msg void OnAddisothermBelow();
	afx_msg void OnUpdateAddisothermInterval(CCmdUI *pCmdUI);
	afx_msg void OnUpdateAddisothermAbove(CCmdUI *pCmdUI);
	afx_msg void OnUpdateAddisothermBelow(CCmdUI *pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};
