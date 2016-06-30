// DemoPlayerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DemoPlayer.h"
#include "DemoPlayerDlg.h"

#include <process.h>
#include <Winsock2.h>
#include<stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>  
#include <iomanip>  
#include <fstream>  

#include "CAxes.h"
#include "CAxis.h"
#include "CScroll.h"
#include "CSeries.h"
#include "CTChart.h"

#include "YUV/Arcus_YUV.h"
#include "YUV/Inferno_YUV.h"
#include "YUV/iron_YUV.h"
#include "YUV/Memoriam_YUV.h"
#include "YUV/RHot_YUV.h"
#include "YUV/sunset_YUV.h"
#include "YUV/Whot_YUV.h"

#define Image_Width 320
#define Image_Height 240//256
#define Tempbar_Width 100
#define Tempfloatbar_width 70
#define ZoomSize 2

using namespace std; 

#pragma comment(lib,"WS2_32")
//服务器端口号为5050
#define DEFAULT_PORT 8500
//缓冲区长度
#define DATA_BUFFER  1024*200//1024
#define DATA_LANGTH 1500


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef struct __ThreadParams{
	int ch;
	CDemoPlayerDlg *dlg; 
}ThreadParams;

ThreadParams gPM[4];
WSADATA wsaData;
SOCKET sClient;
int iPort=DEFAULT_PORT;
//服务器地址长度
int iLen;
//接收数据的缓冲
int iSend;
int   iRecv=0;
long savenum=0;
const char iaddr[100]="127.0.0.1";
//要发送给服务器的信息
char send_buf[]="Hello!I am a client.";
//接收数据的缓冲区
unsigned char recv_buf[DATA_LANGTH];
unsigned char savebuf[DATA_BUFFER*2];
unsigned short savebuf16bit[DATA_BUFFER];
unsigned char udprecvvideobuf[DATA_BUFFER];
//服务器端地址
struct sockaddr_in ser,saclient;
CPoint   point;
int mouseonvideo=0;
long XX,YY;
unsigned char nowmousecolor[10]; 

int Hist_max,Hist_min,Hist_sum_5low,Hist_sum_5hign=0;
unsigned char videoRecvBuf[4][1024*200];
unsigned char yuvBuf[4][1920*1080*3/2];
unsigned char yuvBuf1[Image_Width*Image_Height*3/2];

float  average[10];
float  aver_screen=0;
unsigned char ave_cnt=0;

unsigned long Buffer_Space[Image_Width*Image_Height]={0,};

unsigned char Bp_Space[Image_Width*Image_Height]={0,};
unsigned short X_Space[Image_Width*Image_Height]={0,};
unsigned short B_Space[Image_Width*Image_Height]={0,};
unsigned short B2_Space[Image_Width*Image_Height]={0,};
unsigned short K_Space[Image_Width*Image_Height]={0,};
unsigned short B_aver=0;
unsigned short B_aver_30=0;
unsigned short B2_aver=0;

unsigned char B1_Cal = 0;
unsigned char B2_Cal = 0;
unsigned char B1_Cal_30 = 0;
unsigned char Find_Bp_en = 0;

unsigned char Cal_cnt = 0;
unsigned char tmpcolor[768];

CDC *pDC;
long posYY=200;
long barXX =Image_Width*ZoomSize;
long barYY = 200;

bool m_IntervalChecked = FALSE;
bool m_AboveChecked = FALSE;
bool m_BelowChecked = FALSE;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CDemoPlayerDlg 对话框

void Data_Space_Init(void)
{
	FILE *fp_Bp,*fp_B,*fp_K,*fp_color= NULL;
	char Image_B[Image_Width*Image_Height*2]={0,};
	char Image_K[Image_Width*Image_Height*2+2]={0,};
	unsigned double B_aver_buf=0;
	unsigned int i;

	fp_B = fopen("B1.dat","rb");
	if (fp_B != NULL)
	{
		fread(Image_B,1,Image_Width*Image_Height*2,fp_B);
		for (i = 0;i<Image_Width*Image_Height;i++)
		{
			B_Space[i] = (unsigned short)((unsigned char)Image_B[2*i] + (Image_B[2*i+1]<<8));
			B_aver_buf = B_aver_buf + B_Space[i];
		}
		B_aver = B_aver_buf/(Image_Width*Image_Height);
		B_aver_30 = B_aver;
	}
	else
	{
		memset (B_Space,0x00,Image_Width*Image_Height);
	}

	fp_K = fopen("K.dat","rb");
	if (fp_K != NULL)
	{
		fread(Image_K,1,Image_Width*Image_Height*2+2,fp_K);
		for (i = 0;i<Image_Width*Image_Height;i++)
		{
			K_Space[i] = (unsigned short)((unsigned char)Image_K[2*i] + (Image_K[2*i + 1]<<8));
		}
		B2_aver = (unsigned short)((unsigned char)Image_K[2*i] + (Image_K[2*i + 1]<<8));

	}
	else
	{
		memset (K_Space,0x0400,Image_Width*Image_Height);
	}

	fp_Bp = fopen("Bp.dat","rb");
	if (fp_Bp != NULL)
		fread((char*)Bp_Space,1,Image_Width*Image_Height,fp_Bp);
	else
		memset (Bp_Space,0,Image_Width*Image_Height);

	fp_color = fopen("YUV-Memoriam.raw","rb");
	fread((char*)tmpcolor,1,768,fp_color);

}



CDemoPlayerDlg::CDemoPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDemoPlayerDlg::IDD, pParent)
	//, m_chart_temperature(0)
	//, m_chart_frequency(0)
	//, m_chart_hotcold(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDemoPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_v0, m_view[0]);
	DDX_Control(pDX, IDC_STATIC_TEMP_BAR, m_view_tempbar);
	DDX_Control(pDX, IDC_STATIC_TEMP_FLOATBAR, m_view_tempfloatbar);
	DDX_Control(pDX, IDC_LIST_POSAVR, m_list_posavr);
	DDX_Control(pDX, IDC_TCHART_TEMPERATURE, m_chart_temperature);
	DDX_Control(pDX, IDC_TCHART_FREQUENCY, m_chart_frequency);
	DDX_Control(pDX, IDC_TCHART_HOTCOLD, m_chart_hotcold);
}

BEGIN_MESSAGE_MAP(CDemoPlayerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON_B1Cal, &CDemoPlayerDlg::OnBnClickedButtonB1cal)
	ON_BN_CLICKED(IDC_BUTTON_B1Calof30, &CDemoPlayerDlg::OnBnClickedButtonB1calof30)
	ON_BN_CLICKED(IDC_BUTTON_B2calof80, &CDemoPlayerDlg::OnBnClickedButtonB2calof80)
	ON_BN_CLICKED(IDC_BUTTON_findBP, &CDemoPlayerDlg::OnBnClickedButtonfindbp)
	ON_WM_CTLCOLOR()
	ON_WM_INITMENUPOPUP()
	ON_WM_TIMER()
	ON_COMMAND(ID_ADDISOTHERM_INTERVAL, &CDemoPlayerDlg::OnAddisothermInterval)
	ON_COMMAND(ID_ADDISOTHERM_ABOVE, &CDemoPlayerDlg::OnAddisothermAbove)
	ON_COMMAND(ID_ADDISOTHERM_BELOW, &CDemoPlayerDlg::OnAddisothermBelow)
	ON_UPDATE_COMMAND_UI(ID_ADDISOTHERM_INTERVAL, &CDemoPlayerDlg::OnUpdateAddisothermInterval)
	ON_UPDATE_COMMAND_UI(ID_ADDISOTHERM_ABOVE, &CDemoPlayerDlg::OnUpdateAddisothermAbove)
	ON_UPDATE_COMMAND_UI(ID_ADDISOTHERM_BELOW, &CDemoPlayerDlg::OnUpdateAddisothermBelow)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


// CDemoPlayerDlg 消息处理程序

BOOL CDemoPlayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_bluecolor = RGB(0,209,206);
	m_redcolor = RGB(255,0,0);
	m_greencolor= RGB(0,255,0);
	m_blackcolor = RGB(0,0,0);
	
	SetTimer(1,1000,NULL);

	int tmpsize  ;
	tmpsize = Image_Width*2/9;
	m_list_posavr.InsertColumn(0,_T("Lable"),LVCFMT_LEFT,tmpsize,0);
	m_list_posavr.InsertColumn(1,_T("ROI"),LVCFMT_LEFT,tmpsize,1);
	m_list_posavr.InsertColumn(2,_T("Avg."),LVCFMT_RIGHT,tmpsize,2);
	m_list_posavr.InsertColumn(3,_T("MIN."),LVCFMT_RIGHT,tmpsize,3);
	m_list_posavr.InsertColumn(4,_T("MAX."),LVCFMT_RIGHT,tmpsize,4);
	m_list_posavr.InsertColumn(5,_T("std."),LVCFMT_RIGHT,tmpsize,5);
	m_list_posavr.InsertColumn(6,_T("Emissivity"),LVCFMT_RIGHT,tmpsize,6);
	m_list_posavr.InsertColumn(7,_T("Atm.Temp."),LVCFMT_RIGHT,tmpsize,7);
	m_list_posavr.InsertColumn(8,_T("Atm.Trans."),LVCFMT_RIGHT,tmpsize,8);



	CSeries ser_temperature = (CSeries)m_chart_temperature.Series(0);
	ser_temperature.FillSampleValues(50);
	
	m_chart_temperature.AddSeries(0);
	CSeries ser_temperature2 = (CSeries)m_chart_temperature.Series(1);
	ser_temperature2.FillSampleValues(50);
	ser_temperature.put_Title(_T("sp1(Temperature)"));
	ser_temperature2.put_Title(_T("sp2(Temperature)"));

	CAxes axes = (CAxes)m_chart_temperature.get_Axis();
	CAxis axis = axes.get_Left();
	CAxis axis2 = axis.get_Title();


	CSeries ser_frequency = (CSeries)m_chart_frequency.Series(0);
	ser_frequency.FillSampleValues(50);

	//m_chart_frequency.AddSeries(0);
	CSeries ser_frequency2 = (CSeries)m_chart_frequency.Series(1);
	//ser_frequency2.FillSampleValues(50);
	ser_frequency.put_Title(_T("sp1(Temperature)"));
	//ser_frequency2.put_Title(_T("sp2(Temperature)"));

	CAxes axesf = (CAxes)m_chart_frequency.get_Axis();
	CAxis axisf = axesf.get_Left();
	CAxis axisf2 = axisf.get_Title();


	CSeries ser_hotcold = (CSeries)m_chart_hotcold.Series(0);
	ser_hotcold.FillSampleValues(50);
	ser_hotcold.put_Title(_T("sp1(Temperature)"));
	
	CAxes axesh = (CAxes)m_chart_hotcold.get_Axis();
	CAxis axish = axesh.get_Left();
	CAxis axisfh = axish.get_Title();

	Data_Space_Init();
	memset(recv_buf,0,sizeof(recv_buf));

	if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
	{
		AfxMessageBox(_T("Failed to load Winsock"));
	}

	//建立服务器端地址
	ser.sin_family=AF_INET;
	ser.sin_port=htons(iPort);
	ser.sin_addr.s_addr=htonl(INADDR_ANY);//inet_addr(iaddr);//inet_addr("127.0.0.1");
	//建立客户端数据报套接口
	sClient=socket(AF_INET,SOCK_DGRAM,0);
	if(sClient==INVALID_SOCKET)
	{
		printf("socket()Failed：%d\n",WSAGetLastError());
		AfxMessageBox(_T("socket Failed"));

	}   

	if(bind( sClient, (SOCKADDR FAR *)&ser, sizeof(ser))!=0)
	{
		printf("Can't bind socket to local port!Program stop.\n");//初始化失败返回-1
		return -1;
	}

	iLen = sizeof(saclient);
	unsigned int videoThreadId;
	gPM[0].ch  = 0;
	gPM[0].dlg = this;
	udpRecvThreadHandle =  (HANDLE)_beginthreadex(NULL, 0, udpRecvThread, PVOID(&gPM[0]), 0, &videoThreadId);



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDemoPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDemoPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}


	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_STATIC_v0);
	pWnd ->MoveWindow(CRect(0,0,Image_Width*ZoomSize,Image_Height*ZoomSize));

	CWnd *pwnd2;
	pwnd2 = GetDlgItem(IDC_STATIC_TEMP_BAR);
	pwnd2->MoveWindow(CRect(Image_Width*ZoomSize,0,Image_Width*ZoomSize+Tempbar_Width,Image_Height*ZoomSize));

	CWnd *pwnd3;
	pwnd3 = GetDlgItem(IDC_STATIC_TEMP_FLOATBAR);
	pwnd3->MoveWindow(CRect(Image_Width*ZoomSize+Tempbar_Width,0,Image_Width*ZoomSize+Tempbar_Width+Tempfloatbar_width,Image_Height*ZoomSize));


	CWnd *pWnd4;
	pWnd4 = GetDlgItem(IDC_LIST_POSAVR);
	pWnd4 ->MoveWindow(CRect(0,Image_Height*ZoomSize,Image_Width*ZoomSize,Image_Height*ZoomSize+300));

	CWnd *pWnd5;
	pWnd5 = GetDlgItem(IDC_TCHART_TEMPERATURE);
	pWnd5 ->MoveWindow(CRect(Image_Width*ZoomSize+Tempbar_Width+Tempfloatbar_width,0,Image_Width*ZoomSize+Tempbar_Width+Tempfloatbar_width+600,Image_Height*ZoomSize));

	CWnd *pWnd6;
	pWnd6 = GetDlgItem(IDC_TCHART_FREQUENCY);
	pWnd6 ->MoveWindow(CRect(Image_Width*ZoomSize,Image_Height*ZoomSize,Image_Width*ZoomSize+400,Image_Height*ZoomSize+300));

	CWnd *pWnd7;
	pWnd7 = GetDlgItem(IDC_TCHART_HOTCOLD);
	pWnd7 ->MoveWindow(CRect(Image_Width*ZoomSize+400,Image_Height*ZoomSize,Image_Width*ZoomSize+400+400,Image_Height*ZoomSize+300));

	pDC= this->GetDC();
	OnDrawBlock(pDC,2);

	pDC= this->GetDC();
	OnDrawBlock2(pDC,posYY);

}

void Bp_replace(unsigned short *Image_in,unsigned char *Bp,unsigned short *Image_out)
{
	bool Bp_data = 0;
	unsigned char data_buf;
	int i;

	for (i=0;i<Image_Width*Image_Height;i++)
	{
		Bp_data = *Bp++;
		if (Bp_data)
		{
			*Image_out++ = *(Image_in -1);
			*Image_in++;
		}
		else
			*Image_out++ = *Image_in++;
	}
}
void filter(unsigned short *Image_now,unsigned short *Image_last,unsigned int Value_Th)
{
	int Data_now,Data_last;

	for(int i = 0; i < Image_Width*Image_Height;i++)
	{
		Data_now = *Image_now;
		Data_last = *Image_last;
		if (abs(Data_now - Data_last) < Value_Th)
		{
			if (Data_now > Data_last)
				*Image_now = Data_last + 2;
			else if (Data_now < Data_last)
				*Image_now = Data_last - 2;
		}
		*Image_last ++  = *Image_now++;

	}
}
void smooth (unsigned short *Image_in,unsigned short *Image_out)
{
	unsigned short Data1,Data2,Data3,Data4,Data5,Data6,Data7,Data8,Data9;
	int i,k;

	for (i = 0;i <Image_Height;i++)
	{
		for (k=0;k<Image_Width;k++)
		{
			Data1 = *(Image_in-Image_Width-1);
			Data2 = *(Image_in-Image_Width);
			Data3 = *(Image_in-Image_Width+1);
			Data4 = *(Image_in-1);
			Data5 = *Image_in;
			Data6 = *(Image_in+1);
			Data7 = *(Image_in+Image_Width-1);
			Data8 = *(Image_in+Image_Width);
			Data9 = *(Image_in+Image_Width+1);
			*Image_in++;
			if (k==0)
			{
				if (i==0)
					*Image_out++ =unsigned short(( Data5 +Data6 + Data8+Data9)/4);
				else if (i==255)
					*Image_out++ =unsigned short(( Data5 +Data6 + Data2+Data3)/4);
				else
					*Image_out++ =unsigned short((Data2+ Data5 +Data6 +Data9+ Data8+Data3)/6);
			}
			else if (k==319)
			{
				if (i==0)
					*Image_out++ =unsigned short((Data6+ Data5 +Data4 +Data9+ Data8+Data7)/4);
				else if (i==255)
					*Image_out++ =unsigned short((Data6+ Data5 +Data4 +Data3+ Data2+Data1)/4);
				else
					*Image_out++ =unsigned short((Data2+ Data1 +Data5 +Data4+ Data8+Data7)/6);
			}
			else
			{
				if (i==0)
					*Image_out++ =unsigned short(( Data5 +Data4 + Data8+Data7)/4);
				else if (i==255)
					*Image_out++ =unsigned short(( Data5 +Data4 + Data2+Data1)/4);
				else
					*Image_out++ =unsigned short((Data1+Data2+Data3+Data4+Data5 +Data6 +Data7+ Data8+Data9)/9);
			}
		}
	}
}
void twopointcal(unsigned short *Image_in,unsigned short *Image_B,unsigned short *Image_K,unsigned short B_aver,unsigned short *Image_out)
{
	unsigned short Data_X,Data_B,Data_K=0;
	signed short data_buf = 0;
	int i;

	for (i = 0;i<Image_Width*Image_Height;i++)
	{
		Data_X = *Image_in++;
		Data_B = *Image_B++;
		Data_K = *Image_K++;
		data_buf = (signed short)((((Data_K * (signed short)(Data_X - Data_B))>>12) + B_aver));
		if (data_buf > 16383)
			*Image_out = 16383;
		else if (data_buf < 0)
			*Image_out = 0;
		else
			*Image_out++ = data_buf;
	}
}
void DDE (unsigned short *Image_in,unsigned short *Image_out)
{
	unsigned short Data1,Data2,Data3,Data4,Data5,Data6,Data7,Data8,Data9;
	int i,k;


	for (i = 0;i <Image_Height;i++)
	{
		for (k=0;k<Image_Width;k++)
		{
			Data1 = *(Image_in-Image_Width-1);
			Data2 = *(Image_in-Image_Width);
			Data3 = *(Image_in-Image_Width+1);
			Data4 = *(Image_in-1);
			Data5 = *Image_in;
			Data6 = *(Image_in+1);
			Data7 = *(Image_in+Image_Width-1);
			Data8 = *(Image_in+Image_Width);
			Data9 = *(Image_in+Image_Width+1);
			*Image_in++;
			if (k==0)
			{
				if (i==0)
					*Image_out++ =unsigned short(Data5 + (Data6 + Data8 + Data9 - 3 * Data5)/6);
				else if (i==255)
					*Image_out++ =unsigned short( Data5 +(Data6 + Data2 + Data3- 3 * Data5)/6);
				else
					*Image_out++ =unsigned short(Data5 +(Data6 + Data9 + Data8 + Data2 + Data3 - 5 * Data5)/10);
			}
			else if (k==319)
			{
				if (i==0)
					*Image_out++ =unsigned short(Data5 +(Data4 + Data8 + Data7 - 3 * Data5)/6);
				else if (i==255)
					*Image_out++ =unsigned short(Data5 +(Data4 + Data2 + Data1 - 3 * Data5)/6);
				else
					*Image_out++ =unsigned short(Data5 +(Data2 + Data1 + Data4 + Data8 + Data7 - 5 * Data5)/10);
			}
			else
			{
				if (i==0)
					*Image_out++ =unsigned short(Data5 +(Data6 + Data4  + Data9 + Data8 + Data7 - 5 * Data5)/10);
				else if (i==255)
					*Image_out++ =unsigned short(Data5 +(Data6 + Data4 +Data3 + Data2 + Data1 - 5 * Data5)/10);
				else
					*Image_out++ =unsigned short(Data5 +(Data1 + Data2 + Data3 + Data4 + Data6 + Data7 + Data8 + Data9 - 8 * Data5)/16);

			}
		}
	}
}
void autobc(unsigned short *Image_in, unsigned char *Image_out)
{
	int Hist_sum=0;
	double Hist_sum_buf=0;
	unsigned short data_buf = 0;
	float data_buf2=0;
	int Hist[16384]={0,};
	int Equal_Hist[16384]={0,};
	int Hist_max_5,Hist_min_5= 0;

	Hist_max = 0;
	Hist_min = 16384;
	Hist_sum = 0;

	for (int i = 0;i < Image_Width * Image_Height; i++)
	{
		data_buf = 0;
		data_buf = (*Image_in++);
		if (data_buf < Hist_min)
		{
			Hist_min = data_buf;

		}
		if (data_buf > Hist_max)
		{
			Hist_max = data_buf;
		}
		Hist[data_buf] = Hist[data_buf] + 1;
	}

	for(int i = 0; i < 16384; i++)
	{
		Hist_sum_buf += Hist[i];
		Equal_Hist[i] = Hist_sum_buf;

		if (Hist_sum_buf < Image_Width*Image_Height * 0.05)
		{
			Hist_min_5 = i;
		}
		if (Hist_sum_buf < Image_Width*Image_Height * 0.95)
		{
			Hist_max_5 = i;
		}
	}

	Image_in = (Image_in - Image_Width*Image_Height);

	for (int i=0;i<Image_Width*Image_Height;i++)
	{
		data_buf = (*Image_in++);
		//data_buf2 = 255*Equal_Hist[data_buf]/(Image_Width*Image_Height);
		//if (data_buf > Hist_max_5)
		//	data_buf2 = 255;
		//else if (data_buf < Hist_min_5)
		//	data_buf2 = 0;
		//else
		{

			if ((Hist_max_5 - Hist_min_5) < 64)
				data_buf2 = (float)(0.50 * 32 * (data_buf - Hist_min_5)/(Hist_max_5 - Hist_min_5) + 256 * 0.5);
			else if ((Hist_max_5 - Hist_min_5) < 128)
				data_buf2 = (float)(0.50 * 64 * (data_buf - Hist_min_5)/(Hist_max_5 - Hist_min_5) + 256 * 0.5);
			else if ((Hist_max_5 - Hist_min_5) < 192)
				data_buf2 = (float)(0.50 * 128 * (data_buf - Hist_min_5)/(Hist_max_5 - Hist_min_5) + 256 * 0.5);
			else
				data_buf2 = (float)(0.50 * 256 * (data_buf - Hist_min_5)/(Hist_max_5 - Hist_min_5) + 256 * 0.5);
		}
		if (data_buf2 > 255)
		{
			*Image_out++ = 255;
		}
		else if(data_buf2 < 0 )
		{
			*Image_out++ = 0;
		}
		else
			*Image_out++ = (unsigned char)data_buf2;

	}

}

void Cal_K(unsigned short *Image_B2,unsigned short *Image_B1,unsigned short *Image_K)
{
	unsigned int B2,B1,K=0;
	FILE *fp_Bp,*fp_B,*fp_K= NULL;

	for (int i = 0;i<Image_Width * Image_Height;i++)
	{
		B2 = *Image_B2++;
		B1 = *Image_B1++;
		if (B2 - B1 > 200)
			*Image_K++ = 4096*(B2_aver - B_aver)/(B2 - B1);
		else
		{
			*Image_K++ = 4096;
			Bp_Space[i] = 1;
		}

	}
	*Image_K = B2_aver;
	Image_K = Image_K - Image_Width * Image_Height;
	fp_K = fopen("E:\\K.dat","wb");
	fwrite(Image_K,Image_Width * Image_Height*2+2,1,fp_K);
	fp_Bp = fopen("E:\\Bp.dat","wb");
	fwrite(Bp_Space,Image_Width * Image_Height,1,fp_Bp);
}

void Calculate_B1(unsigned short *Image_in)
{
	unsigned long long aver_buf=0;
	Cal_cnt++;
	for (int i = 0;i<Image_Width * Image_Height;i++)
	{	
		Buffer_Space[i] += *Image_in++;
		if (Cal_cnt == 16)
		{
			B_Space[i] = unsigned int (Buffer_Space[i] / 16);
			aver_buf += B_Space[i];
			Buffer_Space[i] = 0;
		}
	}

	if (Cal_cnt == 16)
	{
		aver_buf = aver_buf / (Image_Width * Image_Height);
		B_aver = unsigned short (aver_buf);
		if (B1_Cal_30 == 1)
			B_aver_30 = unsigned short (aver_buf);
		B1_Cal = 0;
		B1_Cal_30=0;
		Cal_cnt = 0;
	}

}

void Calculate_B2(unsigned short *Image_in)
{
	unsigned long long aver_buf=0;
	Cal_cnt++;
	for (int i = 0;i<Image_Width * Image_Height;i++)
	{	
		Buffer_Space[i] += *Image_in++;
		if (Cal_cnt == 16)
		{
			B2_Space[i] = unsigned int (Buffer_Space[i] / 16);
			aver_buf += B2_Space[i];
			Buffer_Space[i] = 0;
		}
	}

	if (Cal_cnt == 16)
	{
		B2_aver = aver_buf / (Image_Width * Image_Height);
		Cal_K(B2_Space,B_Space,K_Space);
		B2_Cal = 0;
		Cal_cnt = 0;
	}

}

void Cal_Frame_Aver(unsigned short *Image_in)
{

	for (int i = 0;i<Image_Width * Image_Height;i++)
	{
		average[ave_cnt] += *Image_in++;
	}
	average[ave_cnt] = average[ave_cnt]/(Image_Width*Image_Height);

	ave_cnt++;
	if (ave_cnt == 10)
	{
		ave_cnt = 0;
		aver_screen = 0;
		for (int i=0;i<10;i++)
			aver_screen += average[i];
		aver_screen /= 10;
	}
	Image_in =  (Image_in - Image_Width*Image_Height); 


}

void Cal_Color(unsigned char *Image_in,unsigned char *Image_out)
{
	unsigned char *Y_space,*U_space,*V_space;
	unsigned char Y_data_buf;

	Y_space = Image_out;
	U_space = Image_out + Image_Width*Image_Height;
	V_space = Image_out + Image_Width*(Image_Height+60);

	for (int heit =0;heit<Image_Height;heit++)
	{
		for (int wid=0;wid<Image_Width;wid++)
		{	
			Y_data_buf =  *Image_in++;

			*Y_space++ = tmpcolor[Y_data_buf * 3];

			if ((heit %2 == 0) && (wid %2 == 0))
			{
				*U_space++ = tmpcolor[Y_data_buf * 3 + 2];
				*V_space++ = tmpcolor[Y_data_buf * 3 + 1];
			}
		}
	}



}


UINT WINAPI CDemoPlayerDlg::udpRecvThread(PVOID pM){
	ThreadParams *pObj = (ThreadParams*)pM;
	CDemoPlayerDlg *Dlg = pObj->dlg;
	int ch = pObj->ch;
	int i=0,j=0,m=0,n=0;
	int ret,size,isGotFrame;
	int k=0,sendflag=0;
	long framenum=1,serialnum=1;
	char tmpchar[DATA_BUFFER*2];
	struct timeval tv;
	long beforframenum,beforserialnum;
	int _init_render = 0,tt=0,saveflag=0,recverrorflag=0,firstserialflag=0	;
	FILE *pFile = NULL;
	pFile = fopen("WWWWWWWWWWWWWWWWWW.yuv", "wb");
	char filename[] ="TTTTTTTTTTTTTTTTTTTTTT.txt";
	ofstream fout(filename);
	iLen = sizeof(saclient);
	float nowtemp;
	float  average;
	
	while(1)
	{

		iRecv = recvfrom(sClient,(char *)recv_buf,sizeof(recv_buf),0,(struct sockaddr*)&saclient,&iLen);

		if (iRecv==SOCKET_ERROR)
		{
			//TRACE("SOCKET_ERROR\n");
			//Sleep(2);
			continue;
		}
		else if (iRecv == 0)
		{
			TRACE("recv 0\n");
			//Sleep(2);
			continue;
		}
		else 
		{

			//TRACE("ch %d : %x %x %x %x %x %x\n", ch, recv_buf[0], recv_buf[1], recv_buf[2],recv_buf[3],recv_buf[4],recv_buf[5]);
			//TRACE("__iRecv = %d\n",iRecv);

			framenum = (unsigned char)recv_buf[1]<<8;
			framenum = framenum + (unsigned char)recv_buf[0];
			//	fout<<framenum<<":";
			serialnum = recv_buf[3]<<8;
			serialnum = serialnum + recv_buf[2];
			//	fout<<serialnum<<"_"<<'\n';

			if (tt==0)
			{
				beforserialnum= serialnum;
				beforframenum = framenum;
				tt = 1;
			}

			if ((abs(framenum-beforframenum) == 0)&&((serialnum-beforserialnum)!=1))
			{
				TRACE("________error! framenum:%d\n", framenum);
				recverrorflag =1;

			}

			if (framenum-beforframenum != 0)
			{

				TRACE("____framenum =%d\n",framenum);
			}

			if (framenum-beforframenum != 0 && framenum-beforframenum != 1 )
			{
				TRACE("lostframe,  %d %d\n", framenum, beforframenum);
			}

		
		}
#if 1	//display	


		if ((serialnum ==1 )&&(firstserialflag ==0))
		{
			firstserialflag =1;
		}

		if (firstserialflag)
		{
			fout<<framenum<<":";
			fout<<serialnum<<"_"<<'\n';

			if (abs(framenum-beforframenum) != 0 )
			{
				if (recverrorflag==1)
				{
					memset(udprecvvideobuf,0,j);
					memset(savebuf16bit,0,j);
					savenum =0;
					j= 0;
					
					
					TRACE(" throw away, framenum:%d \n", framenum);

				}
				else 
				{

					memset(yuvBuf1,0x80,Image_Width*Image_Height*3/2);
					
					Bp_replace(savebuf16bit,Bp_Space,savebuf16bit);

					if (B1_Cal == 1)
						Calculate_B1(savebuf16bit);

					if (B2_Cal == 1)
						Calculate_B2(savebuf16bit);

					if (B1_Cal_30 == 1)
						Calculate_B1(savebuf16bit);

					Cal_Frame_Aver(savebuf16bit);
					twopointcal(savebuf16bit,B_Space,K_Space,B_aver,savebuf16bit);

					

					if (mouseonvideo == 1)
					{
						//TRACE("*** X = %ld Y = %ld  :  T=  %.2f\n ",XX,YY,nowtemp);

						//nowtemp = savebuf16bit[(((int)YY)/ZoomSize*Image_Width+((int)XX)/ZoomSize)];
						//nowtemp = 0.3138*pow((int)nowtemp,0.6491);

						nowtemp = (unsigned int)(savebuf16bit[(((unsigned int)YY)/ZoomSize*320+((unsigned int)XX)/ZoomSize)]);
						nowtemp = 30 + float(nowtemp - B_aver_30)*50/(B2_aver - B_aver_30);

						TRACE("********** X = %ld Y = %d  :  T=  %.2f  average = %f\n ",XX,YY,nowtemp,aver_screen);
					}

					DDE(savebuf16bit,savebuf16bit);
					autobc(savebuf16bit,yuvBuf1);
					Cal_Color(yuvBuf1,yuvBuf1);
					if(!_init_render){
						Dlg->m_render[ch].createRender(Dlg->m_view[ch].GetSafeHwnd(), VIDEO_YV12,Image_Width, Image_Height, 1);

						_init_render = 1;
					}
					
					fwrite(yuvBuf1,Image_Width*Image_Height*3/2, 1, pFile);
					Dlg->m_render[ch].display(yuvBuf1);


					memset(udprecvvideobuf,0,j);
					memset(savebuf,0,Image_Width*Image_Height*ZoomSize);
					memset(savebuf16bit,0,j);
					
					savenum =0;
					j=0;
					
				}
				
				recverrorflag =0;




			}
			else if (abs(framenum-beforframenum) > 1)
			{
				TRACE("___________________frame drop\n");
			}



		
			unsigned short tmp;
			for (i=0;i<iRecv-4;i++)
			{
				tmp = recv_buf[i+4 +1];
				tmp = tmp << 8;

				tmp +=  (unsigned char)recv_buf[i+4];
				savebuf16bit[j]= tmp;

				udprecvvideobuf[j]=tmp;

				j++;
				i++;
			}


		}


		beforserialnum= serialnum;
		beforframenum = framenum;



#endif
	}

	return 0;
}






//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDemoPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDemoPlayerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnMouseMove(nFlags, point);
	CRect rect_IDC_STATIC_v0;
	GetDlgItem(IDC_STATIC_v0)->GetWindowRect(rect_IDC_STATIC_v0);

	ScreenToClient(rect_IDC_STATIC_v0);

	if (PtInRect(&rect_IDC_STATIC_v0,point))
	{
		mouseonvideo = 1;
		XX=point.x;
		YY=point.y;
	}
	else
	{
		mouseonvideo = 0;
	}

	CRect rect_IDC_STATIC_TEMP_BAR;
	GetDlgItem(IDC_STATIC_TEMP_BAR)->GetWindowRect(rect_IDC_STATIC_TEMP_BAR);
	ScreenToClient(rect_IDC_STATIC_TEMP_BAR);
	if (PtInRect(&rect_IDC_STATIC_TEMP_BAR,point))
	{
		barXX = point.x;
		barYY = point.y;

		pDC= this->GetDC();
		OnDrawBlock(pDC,2);

	}

	//TRACE("___mouseonvideo=%d\n",mouseonvideo);
	//TRACE("_____________________________ X = %ld Y = %d\n ",point.x,point.y);


}

void CDemoPlayerDlg::OnBnClickedButtonB1cal()
{
	// TODO: 在此添加控件通知处理程序代码
	B1_Cal = 1;
}

void CDemoPlayerDlg::OnBnClickedButtonB1calof30()
{
	// TODO: 在此添加控件通知处理程序代码
	B1_Cal_30 = 1;
}

void CDemoPlayerDlg::OnBnClickedButtonB2calof80()
{
	// TODO: 在此添加控件通知处理程序代码
	B2_Cal = 1;
}

void CDemoPlayerDlg::OnBnClickedButtonfindbp()
{
	// TODO: 在此添加控件通知处理程序代码
	Find_Bp_en = 1;
}

HBRUSH CDemoPlayerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性
	if (pWnd->GetDlgCtrlID()==IDC_STATIC_TEMP_BAR)
	{
		pDC->SetBkColor(m_bluecolor);

		HBRUSH b = CreateSolidBrush(m_bluecolor);
		return b;


	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CDemoPlayerDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: 在此处添加消息处理程序代码

	if (!bSysMenu && pPopupMenu)
	{
		CCmdUI cmdUI;
		cmdUI.m_pOther =NULL;
		cmdUI.m_pMenu = pPopupMenu;
		cmdUI.m_pSubMenu = NULL;

		UINT count = pPopupMenu->GetMenuItemCount();
		cmdUI.m_nIndex = count;

		for (UINT i = 0;i<count;i++)
		{
			UINT nID = pPopupMenu->GetMenuItemID(i);
			if ((-1 ==nID)||(0 == nID))
			{
				continue;
			}

			//cmdUI.m_nID= nID;
			//cmdUI.m_nIndex = i;
			//cmdUI.DoUpdate(this,FALSE);

			if (nID ==ID_ADDISOTHERM_INTERVAL) 
			{
				pPopupMenu->CheckMenuItem(ID_ADDISOTHERM_INTERVAL, MF_BYCOMMAND | (m_IntervalChecked ? MF_CHECKED : MF_UNCHECKED));
			}
			if (nID ==ID_ADDISOTHERM_ABOVE)
			{
				pPopupMenu->CheckMenuItem(ID_ADDISOTHERM_ABOVE, MF_BYCOMMAND | (m_AboveChecked ? MF_CHECKED : MF_UNCHECKED));
			}
			if (nID ==ID_ADDISOTHERM_BELOW)
			{
				pPopupMenu->CheckMenuItem(ID_ADDISOTHERM_BELOW, MF_BYCOMMAND | (m_BelowChecked ? MF_CHECKED : MF_UNCHECKED));
			}

		}
	}


}

void CDemoPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int tmpposX;
	int tmpposY;
	
	tmpposX = Image_Width*ZoomSize+Tempbar_Width;
	tmpposY = Image_Height*ZoomSize;


	InvalidateRect(&CRect(tmpposX,0,tmpposX+Tempfloatbar_width,tmpposY),TRUE);
	UpdateWindow();
	pDC= this->GetDC();
	OnDrawBlock2(pDC,posYY);
	pDC= this->GetDC();
	OnDrawBlock(pDC,2);

	posYY+=10;

	if (posYY>Image_Height*ZoomSize-30)
	{
		posYY =100;
	}

	CSeries ser_temperature2 = (CSeries)m_chart_temperature.Series(1);
	CSeries ser_temperature = (CSeries)m_chart_temperature.Series(0);
	ser_temperature.Add(900,_T("lable"),1);
	ser_temperature2.Add(800,_T("lable"),1);
	//m_chart.get_Axis()->
	CAxes axesDemo = (CAxes)m_chart_temperature.get_Axis();
	CAxis axisDemo = axesDemo.get_Bottom();
	axisDemo.Scroll(1.0,TRUE);
	
	//CSeries ser_frequency2 = (CSeries)m_chart_frequency.Series(1);
	CSeries ser_frequency = (CSeries)m_chart_frequency.Series(0);
	ser_frequency.Add(900,_T("lable"),1);
	//ser_frequency2.Add(800,_T("lable"),1);
	//m_chart.get_Axis()->
	CAxes axesDemof = (CAxes)m_chart_frequency.get_Axis();
	CAxis axisDemof = axesDemof.get_Bottom();
	axisDemof.Scroll(1.0,TRUE);

	CSeries ser_hotcold = (CSeries)m_chart_hotcold.Series(0);
	ser_hotcold.Add(900,_T("lable"),1);
	CAxes axesDemoh = (CAxes)m_chart_hotcold.get_Axis();
	CAxis axisDemoh = axesDemoh.get_Bottom();
	axisDemoh.Scroll(1.0,TRUE);


	CDialog::OnTimer(nIDEvent);
}
void CDemoPlayerDlg::OnDrawBlock2(CDC* pDC,int posYY)
{


	int tmpposX;
	int tmpposY;
	

	tmpposX = Image_Width*ZoomSize+Tempbar_Width;
	tmpposY = Image_Height*ZoomSize;
	


	CPen NewPen3(PS_SOLID,1,m_blackcolor);
	CPen *pOldPen3;
	pOldPen3 = pDC->SelectObject(&NewPen3);
	pDC->Rectangle(tmpposX+10,0,tmpposX+Tempfloatbar_width-10,tmpposY);
	pDC->Rectangle(tmpposX+5,posYY+5,tmpposX+Tempfloatbar_width-5,posYY);

	CBrush brush5 ,*pOldBrush5;
	brush5.CreateSolidBrush(m_redcolor);
	pOldBrush5 = pDC->SelectObject(&brush5);
	pDC->Rectangle(tmpposX+10+(Tempfloatbar_width-20)/3,posYY+15,tmpposX+10+(Tempfloatbar_width-20)*2/3,posYY+10);
	pDC->SelectObject(pOldBrush5);
	brush5.DeleteObject();

	pDC->Rectangle(tmpposX+5,posYY+25,tmpposX+Tempfloatbar_width-5,posYY+20);
	pDC->SelectObject(pOldPen3);
	NewPen3.DeleteObject();
	ReleaseDC(pDC);	



}
CString lcstr[] = {_T("30"),_T("29"),_T("28"),_T("27"),_T("26"),_T("25"),_T("24"),_T("23"),_T("22"),_T("21"),_T("20"),_T("19"),_T("18")_T("17")_T("16"),_T("15"),_T("14")};
void CDemoPlayerDlg::OnDrawBlock(CDC* pDC,int pXY)
{
	CBrush brush ,*pOldBrush;
	int tmpposX;
	int tmpposY;
	int tmplineX;
	int stepY;

	tmpposX = Image_Width*ZoomSize;
	tmpposY = Image_Height*ZoomSize;
	tmplineX = tmpposX+Tempbar_Width/2;
	stepY = (tmpposY -0)/12; //****************
	brush.CreateSolidBrush(m_redcolor);
	pOldBrush = pDC->SelectObject(&brush);
	pDC->Rectangle(tmpposX,0,tmpposX+Tempbar_Width*2/5,tmpposY);
	pDC->SelectObject(pOldBrush);
	brush.DeleteObject();
	CPen NewPen(PS_SOLID,1,m_greencolor);
	CPen *pOldPen;
	pOldPen = pDC->SelectObject(&NewPen);
	pDC->Rectangle(tmpposX+Tempbar_Width*3/5,0,tmpposX+Tempbar_Width,tmpposY);
	pDC->SelectObject(pOldPen);
	NewPen.DeleteObject();
	CPen pen2(PS_SOLID,2,m_blackcolor);
	CPen *pOldPen2;
	pOldPen2 =pDC->SelectObject(&pen2);
	
	pDC->MoveTo(tmplineX,0);
	pDC->LineTo(tmplineX,tmpposY);


	for (int i =0; i<12;i++)
	{
		if (i %2 ==0)
		{
			pDC->MoveTo(tmplineX,stepY*i);
			pDC->LineTo(tmplineX+10,stepY*i);
			pDC->TextOut(tmplineX+10,stepY*i-5,lcstr[i]);
		}
		else
		{
			pDC->MoveTo(tmplineX,stepY*i);
			pDC->LineTo(tmplineX+5,stepY*i);
		}
	
	}



	pDC->SelectObject(pOldPen2);
	pOldPen2->DeleteObject();



	switch(pXY)
	{
	case 1:
		brush.CreateSolidBrush(m_redcolor);
		break;
	case 2:
		brush.CreateSolidBrush(m_greencolor);
		break;
	case 3:
		brush.CreateSolidBrush(m_redcolor);
		break;
	default:
		brush.CreateSolidBrush(m_redcolor);
	}


	pOldBrush = pDC->SelectObject(&brush);
	if (barYY<tmpposY/10)
	{
		barYY=tmpposY/10;
	}
	else if (barYY>tmpposY*9/10)
	{
		barYY=tmpposY*9/10;
	}


	pDC->Rectangle(tmpposX,barYY-tmpposY/10,tmpposX+Tempbar_Width*2/5,barYY+tmpposY/10);
	pDC->SelectObject(pOldBrush);
	brush.DeleteObject();


	ReleaseDC(pDC);		



}
void CDemoPlayerDlg::OnAddisothermInterval()
{
	// TODO: 在此添加命令处理程序代码
	m_IntervalChecked = !m_IntervalChecked;
}

void CDemoPlayerDlg::OnAddisothermAbove()
{
	// TODO: 在此添加命令处理程序代码
	m_AboveChecked = !m_AboveChecked;
}

void CDemoPlayerDlg::OnAddisothermBelow()
{
	// TODO: 在此添加命令处理程序代码
	m_BelowChecked = !m_BelowChecked;
}

void CDemoPlayerDlg::OnUpdateAddisothermInterval(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_IntervalChecked);
}

void CDemoPlayerDlg::OnUpdateAddisothermAbove(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_AboveChecked);
}

void CDemoPlayerDlg::OnUpdateAddisothermBelow(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_BelowChecked);
}

void CDemoPlayerDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CRect rect_IDC_STATIC_TEMP_BAR;
	m_view_tempbar.GetWindowRect(&rect_IDC_STATIC_TEMP_BAR);

	if (PtInRect(&rect_IDC_STATIC_TEMP_BAR,point))
	{
		TRACE("mouse on \n");
		CMenu menu_IDR_MENU_TEMPBAR;
		menu_IDR_MENU_TEMPBAR.LoadMenu(IDR_MENU_TEMPBAR);
		CMenu *pPopup = menu_IDR_MENU_TEMPBAR.GetSubMenu(0);
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
	}


	CDialog::OnRButtonDown(nFlags, point);
}
