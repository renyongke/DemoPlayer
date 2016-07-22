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
	HANDLE CalcThreadHandle;
	static UINT WINAPI udpRecvThread(PVOID pM);
	static UINT WINAPI calcThread(PVOID pM);
	
	

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
	COLORREF m_bluecolor,m_bluecolor2,m_redcolor,m_greencolor,m_blackcolor,m_yellowcolor,m_whitecolor,m_magentacolor,m_cyancolor,m_tapecolor;
	afx_msg void OnDrawTempBar(CDC* pDC,int pXY);
	afx_msg void OnDrawFloatTempBar(CDC* pDC,int posYY);
	afx_msg void OnDrawHistogram(CDC* pDC);
	afx_msg void OnDrawSpot(CDC* pDC,int posXX,int posYY,int sport);
	afx_msg void OnDrawArea(CDC* pDC,int startPosXX,int startPosYY,int posXX,int posYY,int area);
	afx_msg void OnDrawHotColdPlot(CDC* pDC,int startPosXX,int startPosYY,int posXX,int posYY,int size,bool Mode);
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
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonSetspot();
	afx_msg void OnBnClickedButtonSetarea();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonHotcoldplot();
	afx_msg void OnBnClickedButtonHistogram();
	afx_msg void OnNMClickListPosavr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnToAddListControl(int sport);
	afx_msg void OnToUpdateListControl(int row,int column,int sport);
	CComboBox m_comboColor;
	CComboBox m_combobox;
	afx_msg void OnCbnSelchangeComboColor();
	afx_msg void toChangeColor(CString color);
	afx_msg void toChangeBarColor(CString color);
	afx_msg void OnDrawBarColor(CDC* pDC);
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnClose();
	afx_msg void toMarkPotColor(CDC* pDC,unsigned short *Image_in);
};