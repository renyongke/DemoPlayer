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

#include "YUV/Arcus_YUV.h"
#include "YUV/Inferno_YUV.h"
#include "YUV/iron_YUV.h"
#include "YUV/Memoriam_YUV.h"
#include "YUV/RHot_YUV.h"
#include "YUV/sunset_YUV.h"
#include "YUV/Whot_YUV.h"

#define Image_Width 320
#define Image_Height 240//256

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
unsigned char yuvBuf1[Image_Width*Image_Height];

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




CDemoPlayerDlg::CDemoPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDemoPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDemoPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_v0, m_view[0]);
}

BEGIN_MESSAGE_MAP(CDemoPlayerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_MOUSEMOVE()
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
	pWnd ->MoveWindow(CRect(0,0,Image_Width*2,Image_Height*2));

}

void autobc(unsigned short *Image_in, unsigned char *Image_out)
{
	//int Hist_max,Hist_min,Hist_sum_5low,Hist_sum_5hign=0;
	int Hist_sum,Hist_sum_buf=0;
	int data_buf = 0;
	int Hist[16384]={0,};

	Hist_max = 0;
	Hist_min = 16383;
	Hist_sum = 0;

	for (int i=0;i<Image_Width*Image_Height;i++)
	{
		data_buf = 0;
		data_buf = (*Image_in);//+((*(Image_in+1))<<8);
		if (data_buf < Hist_min)
		{
			Hist_min = data_buf;

		}
		if (data_buf > Hist_max)
		{
			Hist_max = data_buf;
		}
		Hist[data_buf] = Hist[data_buf] + 1;
		Image_in +=1;
	}

	for(int j = 0;j<16384;j++)
	{
		Hist_sum_buf += Hist[j];
		if (Hist_sum_buf < 16384)
		{
			Hist_sum_5low = j;
			Hist_sum = Hist_sum_buf - Hist[j];
		}
		else if (Hist_sum_buf < (unsigned int)(Image_Width*Image_Height*0.95))
		{
			Hist_sum_5hign = j+1;
			Hist_sum = Hist_sum_buf;
		}
	}

	Image_in = (Image_in - Image_Width*Image_Height);//*2);

	for (int k=0;k<Image_Width*Image_Height;k++)
	{
		data_buf = (*Image_in);//+(*(Image_in + 1)<<8);
		Image_in +=1;
		if (data_buf < Hist_sum_5low)
			*Image_out++ = 0;
		else if (data_buf > Hist_sum_5hign)
			*Image_out++ = 255;
		else
		{
			Hist_sum_buf=0;
			for (int h =Hist_sum_5low;h < data_buf;h++)
				Hist_sum_buf += Hist[h];
			*Image_out++ = 255* Hist_sum_buf/ Hist_sum;
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
	int count=0;
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
					count=0;
					n= 0;
					TRACE(" throw away, framenum:%d \n", framenum);

				}
				else 
				{

					memset(yuvBuf1,0x80,Image_Width*Image_Height*3/2);
					//memcpy(yuvBuf1,udprecvvideobuf,320*256);
					autobc(savebuf16bit,yuvBuf1);
#if 0
					for (m=0;m<Image_Width*Image_Height;m++)
					{
						if (m=0)
						{
							yuvBuf1[m]=Arcus_YUV[0];

							yuvBuf1[m+Image_Width*Image_Height]=Arcus_YUV[1];
							yuvBuf1[m+Image_Width*Image_Height+1]=Arcus_YUV[2];
						}
						else
						{
							yuvBuf1[m]=Arcus_YUV[m*3-1];

							if ((m+1)%4==0)
							{
								yuvBuf1[m+Image_Width*Image_Height+Image_Width*count]=Arcus_YUV[m*3-1+1];
								yuvBuf1[m+Image_Width*Image_Height+Image_Width*count+1]=Arcus_YUV[m*3-1+2];
								n += 1;
							}
							if (n==319)
							{
								n=0;
								count+=2;
							}
						}


					}
#endif

					//TRACE("___savenum = %d, size:%d\n",framenum, savenum);
					if(!_init_render){
						Dlg->m_render[ch].createRender(Dlg->m_view[ch].GetSafeHwnd(), VIDEO_YV12,Image_Width, Image_Height, 1);

						_init_render = 1;
					}
					//TRACE("____ j = %d",j);
					//fwrite(udprecvvideobuf,j, 1, pFile);
					//memset(udprecvvideobuf,255,j);
					//Dlg->m_render[ch].display(udprecvvideobuf);
					//fwrite(yuvBuf1,320*384, 1, pFile);
					fwrite(savebuf,savenum, 1, pFile);
					Dlg->m_render[ch].display(yuvBuf1);


					if (mouseonvideo == 1)
					{
						//GetCursorPos(&point);
						/*
						if (XX%2 == 1)
						{
						XX= XX-1;
						}*/
						//memcpy(nowmousecolor,(savebuf+(((int)YY)/2*640+XX)),2);
						//nowtemp = 0.3138*pow(((nowmousecolor[1]<<8 )+ nowmousecolor[0]),0.6491);

						//TRACE("*** X = %ld Y = %ld  :  T=  %.2f\n ",XX,YY,nowtemp);
						for (i=0;i<j;i++)
						{
							average+=savebuf16bit[i];
						}
						average = average/j;

						nowtemp = savebuf16bit[(((int)YY)/2*Image_Width+((int)XX)/2)];
						nowtemp = 0.3138*pow((int)nowtemp,0.6491);

						TRACE("********** X = %ld Y = %d  :  T=  %.2f  average = %f\n ",XX,YY,nowtemp,average);

						//memset(nowmousecolor,0,10);
					}

					memset(udprecvvideobuf,0,j);
					memset(savebuf,0,savenum);
					memset(savebuf16bit,0,j);
					savenum =0;
					j=0;
					count=0;

				}
				recverrorflag =0;




			}
			else if (abs(framenum-beforframenum) > 1)
			{
				TRACE("___________________frame drop\n");
			}



			/*
			unsigned short tmp;
			for (i=0;i<iRecv-4;i++)
			{
			tmp = recv_buf[i+4 +1];
			tmp = tmp << 8;

			tmp +=  (unsigned char)recv_buf[i+4];
			savebuf16bit[j]= tmp;
			tmp =  (tmp >> 3);
			udprecvvideobuf[j]=tmp;

			j++;
			i++;
			}*/
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



			//memcpy(savebuf+savenum,recv_buf+4,iRecv-4);
			//savenum = savenum+iRecv-4;





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
	//TRACE("___mouseonvideo=%d\n",mouseonvideo);
	//TRACE("_____________________________ X = %ld Y = %d\n ",point.x,point.y);


}
