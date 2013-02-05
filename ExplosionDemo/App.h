#pragma once
#include "Viewport.h"
#include "Clock.h"
#include <D3DX11.h>
#include <D3D11Shader.h>
#include <WindowsX.h>
#include <DxErr.h>
#include <vector>

// HR macro by Frank Luna
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                      \
	{                                                  \
	HRESULT hr = x;                                \
	if((hr) < 0)                                 \
		{                                              \
		DXTrace(__FILE__, __LINE__, hr, L#x, TRUE); \
		}                                              \
	}
#endif

#else
#ifndef HR
#define HR(x) x;
#endif
#endif 

typedef struct ShaderParams_S{
	float	lacunarity;
	float	gain;
	float	gradOffset;
	float	gradScale;
} ShaderParams;



class App
{
public:
	App(HINSTANCE hInstance, int width, int height);
	~App(void);

	bool Init();
	int Run();

	void SetWinSize(int width, int height);
	void Resize();
	void Update();
	void Draw();

	// Control
	void ScanInput(float dt);
	void RBDown(WPARAM btn, int x, int y);
	void RBUp(WPARAM btn, int x, int y);
	void MouseMove(WPARAM btn, int x, int y);


private:
	bool InitWindow();
	bool InitD3D();

	void InitCanvas();

	bool CompileShaders();
	void SetupSamplers();
	void SetupTextures();

	HINSTANCE						appInst;
	HWND							mainWnd;
	ID3D11Device*					dev;
	ID3D11DeviceContext*			devContext;
	int								winWidth;
	int								winHeight;
	IDXGISwapChain*					swapChain;
	ID3D11RenderTargetView*			renderTarget;
	static float					canvas[30];
	ID3D11InputLayout*				inLayout;
	ID3D11VertexShader*				vertexShader;
	ID3D11PixelShader*				pixelShader;
	ID3D11Buffer*					vertexBuffer;

	ID3D11Buffer*					viewportBuffer;
	ID3D11Buffer*					shaderParamsBuffer;

	ID3D11SamplerState*				permSampler2DSS;
	ID3D11SamplerState*				permGradSamplerSS;
	ID3D11SamplerState*				fireGradSamplerSS;
	ID3D11SamplerState*				groundTextureSamplerSS;

	ID3D11ShaderResourceView*		permTexture2dSRV;
	ID3D11ShaderResourceView*		permGradTextureSRV;
	ID3D11ShaderResourceView*		fireGradTextureSRV;
	ID3D11ShaderResourceView*		groundTextureSRV;
	ID3D11ShaderResourceView*		simplexGradTextureSRV;

	ID3D11Texture2D*				permTexture2D;
	ID3D11Texture1D*				permGradTexture;
	ID3D11Texture1D*				fireGradTexture;
	ID3D11Texture2D*				groundTexture;
	ID3D11Texture1D*				simplexGradTexture;

	// Multiple viewport support not yet fully implemented
	std::vector<Viewport>			viewports;

	Clock							clock;

	int								mouseX;
	int								mouseY;

	ShaderParams*					shaderParams;
};

