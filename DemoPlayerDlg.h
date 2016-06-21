// DemoPlayerDlg.h : ͷ�ļ�
//

#pragma once
#include "d3d_render_interface.h"

// CDemoPlayerDlg �Ի���
class CDemoPlayerDlg : public CDialog
{
// ����
public:
	CDemoPlayerDlg(CWnd* pParent = NULL);	// ��׼���캯��
	CStatic m_view[4];
	Cd3d_render_interface m_render[4];
	HANDLE udpRecvThreadHandle;

	static UINT WINAPI udpRecvThread(PVOID pM);
	
	

// �Ի�������
	enum { IDD = IDD_DEMOPLAYER_DIALOG };

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
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
