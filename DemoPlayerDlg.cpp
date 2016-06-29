// DemoPlayerDlg.cpp : ʵ���ļ�
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
//�������˿ں�Ϊ5050
#define DEFAULT_PORT 8500
//����������
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
//��������ַ����
int iLen;
//�������ݵĻ���
int iSend;
int   iRecv=0;
long savenum=0;
const char iaddr[100]="127.0.0.1";
//Ҫ���͸�����������Ϣ
char send_buf[]="Hello!I am a client.";
//�������ݵĻ�����
unsigned char recv_buf[DATA_LANGTH];
unsigned char savebuf[DATA_BUFFER*2];
unsigned short savebuf16bit[DATA_BUFFER];
unsigned char udprecvvideobuf[DATA_BUFFER];
//�������˵�ַ
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CDemoPlayerDlg �Ի���

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
	ON_BN_CLICKED(IDC_BUTTON_B1Cal, &CDemoPlayerDlg::OnBnClickedButtonB1cal)
	ON_BN_CLICKED(IDC_BUTTON_B1Calof30, &CDemoPlayerDlg::OnBnClickedButtonB1calof30)
	ON_BN_CLICKED(IDC_BUTTON_B2calof80, &CDemoPlayerDlg::OnBnClickedButtonB2calof80)
	ON_BN_CLICKED(IDC_BUTTON_findBP, &CDemoPlayerDlg::OnBnClickedButtonfindbp)
END_MESSAGE_MAP()


// CDemoPlayerDlg ��Ϣ�������

BOOL CDemoPlayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	Data_Space_Init();
	memset(recv_buf,0,sizeof(recv_buf));

	if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
	{
		AfxMessageBox(_T("Failed to load Winsock"));
	}

	//�����������˵�ַ
	ser.sin_family=AF_INET;
	ser.sin_port=htons(iPort);
	ser.sin_addr.s_addr=htonl(INADDR_ANY);//inet_addr(iaddr);//inet_addr("127.0.0.1");
	//�����ͻ������ݱ��׽ӿ�
	sClient=socket(AF_INET,SOCK_DGRAM,0);
	if(sClient==INVALID_SOCKET)
	{
		printf("socket()Failed��%d\n",WSAGetLastError());
		AfxMessageBox(_T("socket Failed"));

	}   

	if(bind( sClient, (SOCKADDR FAR *)&ser, sizeof(ser))!=0)
	{
		printf("Can't bind socket to local port!Program stop.\n");//��ʼ��ʧ�ܷ���-1
		return -1;
	}

	iLen = sizeof(saclient);
	unsigned int videoThreadId;
	gPM[0].ch  = 0;
	gPM[0].dlg = this;
	udpRecvThreadHandle =  (HANDLE)_beginthreadex(NULL, 0, udpRecvThread, PVOID(&gPM[0]), 0, &videoThreadId);



	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CDemoPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
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
					
					n= 0;
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

					DDE(savebuf16bit,savebuf16bit);
					autobc(savebuf16bit,yuvBuf1);
					Cal_Color(yuvBuf1,yuvBuf1);

					if (mouseonvideo == 1)
					{
						//TRACE("*** X = %ld Y = %ld  :  T=  %.2f\n ",XX,YY,nowtemp);

						//nowtemp = savebuf16bit[(((int)YY)/ZoomSize*Image_Width+((int)XX)/ZoomSize)];
						//nowtemp = 0.3138*pow((int)nowtemp,0.6491);

						nowtemp = (unsigned int)(savebuf16bit[(((unsigned int)YY)/2*320+((unsigned int)XX)/2)]);
						nowtemp = 30 + float(nowtemp - B_aver_30)*50/(B2_aver - B_aver_30);

						TRACE("********** X = %ld Y = %d  :  T=  %.2f  average = %f\n ",XX,YY,nowtemp,aver_screen);
					}

					
					if(!_init_render){
						Dlg->m_render[ch].createRender(Dlg->m_view[ch].GetSafeHwnd(), VIDEO_YV12,Image_Width, Image_Height, 1);

						_init_render = 1;
					}
					
					fwrite(yuvBuf1,Image_Width*Image_Height*3/2, 1, pFile);
					Dlg->m_render[ch].display(yuvBuf1);


					memset(udprecvvideobuf,0,j);
					memset(savebuf,0,savenum);
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






//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CDemoPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDemoPlayerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

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

void CDemoPlayerDlg::OnBnClickedButtonB1cal()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	B1_Cal = 1;
}

void CDemoPlayerDlg::OnBnClickedButtonB1calof30()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	B1_Cal_30 = 1;
}

void CDemoPlayerDlg::OnBnClickedButtonB2calof80()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	B2_Cal = 1;
}

void CDemoPlayerDlg::OnBnClickedButtonfindbp()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	Find_Bp_en = 1;
}
