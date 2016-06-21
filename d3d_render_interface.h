// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� D3D_RENDER_INTERFACE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// D3D_RENDER_INTERFACE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef D3D_RENDER_INTERFACE_EXPORTS
#define D3D_RENDER_INTERFACE_API __declspec(dllexport)
#else
#define D3D_RENDER_INTERFACE_API __declspec(dllimport)
#endif

#define MAX_CHANNEL 7

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3d_render_interface.lib")


#define VIDEO_YV12 0
#define VIDEO_YUYV 1
#define VIDEO_UYVY 2
#define VIDEO_420P_SEMI 3

// �����Ǵ� d3d_render_interface.dll ������
class D3D_RENDER_INTERFACE_API Cd3d_render_interface {
public:
	Cd3d_render_interface(void);
	// TODO: �ڴ�������ķ�����

private:
	int pixel_width;
	int pixel_height;

	IDirect3D9 *pd3d9;
	IDirect3DDevice9 *device;
	IDirect3DSurface9 * pOffscreenSurface;

	RECT imageBufferRect[MAX_CHANNEL];
	unsigned char *imageBufferPtr[MAX_CHANNEL];

	int m_type;

public:
	int  createRender (HWND hWnd, int type, int pixelWidth, int pixelHeight, int is_window);
	void display(unsigned char *buf);
	void destroyRender();

};

//extern D3D_RENDER_INTERFACE_API int nd3d_render_interface;

//D3D_RENDER_INTERFACE_API int fnd3d_render_interface(void);
