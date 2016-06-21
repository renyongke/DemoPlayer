// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 D3D_RENDER_INTERFACE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// D3D_RENDER_INTERFACE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
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

// 此类是从 d3d_render_interface.dll 导出的
class D3D_RENDER_INTERFACE_API Cd3d_render_interface {
public:
	Cd3d_render_interface(void);
	// TODO: 在此添加您的方法。

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
