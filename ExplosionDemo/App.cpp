#include "App.h"
#include <fstream>
#include <vector>
#include <sstream>


static App*	sApp = 0;

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_SIZE:
		sApp->SetWinSize(LOWORD(lParam), HIWORD(lParam));
		sApp->Resize();
		return 0;
	case WM_EXITSIZEMOVE:
		sApp->Resize();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_RBUTTONDOWN:
		sApp->RBDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_RBUTTONUP:
		sApp->RBUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		sApp->MouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	
}

float App::canvas[30] = {
	-1, 1, 0, 0, 0,
	1, -1, 0, 1, 1,
	-1, -1, 0, 1, 0,
	-1, 1, 0, 0, 0,
	1, 1, 0, 0, 1,
	1, -1, 0, 1, 1
};

App::App(HINSTANCE hInstance, int width, int height) :
		appInst(hInstance),
		mainWnd(0),
		dev(0),
		devContext(0),
		winWidth(width),
		winHeight(height),
		swapChain(0),
		renderTarget(0),
		inLayout(0),
		vertexShader(0),
		pixelShader(0),
		vertexBuffer(0),
		viewportBuffer(0),
		shaderParams(0),
		permSampler2DSS(0),
		permGradSamplerSS(0),
		fireGradSamplerSS(0),
		groundTextureSamplerSS(0),
		permTexture2dSRV(0),
		permGradTextureSRV(0),
		fireGradTextureSRV(0),
		groundTextureSRV(0),
		simplexGradTextureSRV(0),
		permTexture2D(0),
		permGradTexture(0),
		fireGradTexture(0),
		groundTexture(0),
		simplexGradTexture(0)
		
		
{
	// Initialize default viewport:
	// Look from 14,0,5
	// Look at origo
	// Up positive y-axis
	// FoV 47 degrees (50mm lens)
	viewports.push_back(Viewport(Vec(14, 0, 5), Vec(0,0,0), Vec(0,1,0), 47, (float)width/height, width, height));

	shaderParams = new ShaderParams;
	ZeroMemory(shaderParams, sizeof(ShaderParams));
	shaderParams->lacunarity = 2.0;
	shaderParams->gain = 0.5;
	shaderParams->gradOffset = 0;
	shaderParams->gradScale = 1;

	sApp = this;
}


App::~App(void)
{
	delete shaderParams;

	if(renderTarget)
		renderTarget->Release();

	if(swapChain)
		swapChain->Release();

	if(inLayout)
		inLayout->Release();

	if(vertexBuffer)
		vertexBuffer->Release();
	if(shaderParamsBuffer)
		shaderParamsBuffer->Release();
	if(viewportBuffer)
		viewportBuffer->Release();

	if (vertexShader)
		vertexShader->Release();

	if(pixelShader)
		pixelShader->Release();

	if(permSampler2DSS)
		permSampler2DSS->Release();
	if(permGradSamplerSS)
		permGradSamplerSS->Release();
	if(fireGradSamplerSS)
		fireGradSamplerSS->Release();
	if(groundTextureSamplerSS)
		groundTextureSamplerSS->Release();
	if(permTexture2dSRV)
		permTexture2dSRV->Release();
	if(permGradTextureSRV)
		permGradTextureSRV->Release();
	if(fireGradTextureSRV)
		fireGradTextureSRV->Release();
	if(groundTextureSRV)
		groundTextureSRV->Release();
	if(simplexGradTextureSRV)
		simplexGradTextureSRV->Release();
	if(permTexture2D)
		permTexture2D->Release();
	if(permGradTexture)
		permGradTexture->Release();
	if(fireGradTexture)
			fireGradTexture->Release();
	if(simplexGradTexture)
		simplexGradTexture->Release();


	if(dev)
		dev->Release();

	if(devContext)
	{
		devContext->ClearState();
		devContext->Release();
	}
}


// Initialize application window
// Most of the error checking omitted
bool App::Init()
{
	/*
	viewports[0].SetVPSize(winWidth/2, winHeight/2);
	Viewport vp(viewports[0]);
	vp.SetVPPosition(winWidth/2, winHeight/2);
	viewports.push_back(vp);
	*/

	if(!InitWindow())
		return false;

	if(!InitD3D())
		return false;

	// Call resize to do rest of window initialization
	Resize();

	if(!CompileShaders())
		return false;

	// Set up sampler states for perlin noise
	SetupSamplers();

	// Set up lookup textures for perlin noise
	SetupTextures();	

	// Initialize vertex buffer with canvas quad
	InitCanvas();

	// Init default viewport
	viewports[0].RefreshBuffer();

	// Initialize viewport buffer
	D3D11_SUBRESOURCE_DATA vpbufData;
	vpbufData.pSysMem			= viewports[0].GetBufferData();
	vpbufData.SysMemPitch		= NULL;
	vpbufData.SysMemSlicePitch	= NULL;
	HR(dev->CreateBuffer(&Viewport::viewportBufferDesc, NULL, &viewportBuffer));

	// Initialize shader params buffer;
	D3D11_BUFFER_DESC shaderBufferDesc= {
		sizeof(ShaderParams),
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_CONSTANT_BUFFER,
		D3D11_CPU_ACCESS_WRITE,
		0
	};
	D3D11_SUBRESOURCE_DATA shaderBuffer;
	shaderBuffer.pSysMem			= shaderParams;
	shaderBuffer.SysMemPitch		= NULL;
	shaderBuffer.SysMemSlicePitch	= NULL;
	HR(dev->CreateBuffer(&shaderBufferDesc, &shaderBuffer, &shaderParamsBuffer));

	// And finally update the window
	UpdateWindow(mainWnd);

	return true;
}

int App::Run()
{
	MSG msg = {0};

	clock.Start();

	while (msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update();
			Draw();
		}
	}

	return 0;
}

void App::Resize()
{
	if(!(devContext && swapChain && dev))
		return;
	// Release old render target
	if(renderTarget)
		renderTarget->Release();
	renderTarget = 0;

	swapChain->ResizeBuffers(1, winWidth, winHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, 0);
	ID3D11Texture2D* bbuf;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&bbuf));
	HR(dev->CreateRenderTargetView(bbuf, 0, &renderTarget));
	bbuf->Release();
	
	devContext->OMSetRenderTargets(1, &renderTarget, NULL);

	for(auto it=viewports.begin();it != viewports.end(); it++)
		it->SetVPSize(winWidth, winHeight);
}

void App::SetWinSize( int width, int height )
{
	winWidth = width;
	winHeight = height;
}

void App::Draw()
{
	const FLOAT bgColor[4] = {0.0, 0.0, 0.0, 1.0};
	devContext->ClearRenderTargetView(renderTarget, bgColor);
	devContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devContext->IASetInputLayout(inLayout);

	for(auto it = viewports.begin(); it != viewports.end(); it++)
	{
		// Map and copy cb data
		D3D11_MAPPED_SUBRESOURCE msr;
		devContext->Map(viewportBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &msr);
		memcpy(msr.pData, it->GetBufferData(), sizeof(RawData));
		devContext->Unmap(viewportBuffer, NULL);

		// Map and copy shader data
		devContext->Map(shaderParamsBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &msr);
		memcpy(msr.pData, shaderParams, sizeof(ShaderParams));
		devContext->Unmap(shaderParamsBuffer, NULL);

		// Set constant buffers
		ID3D11Buffer* cBuffers[2];
		cBuffers[0] = viewportBuffer;
		cBuffers[1] = shaderParamsBuffer;
		devContext->PSSetConstantBuffers(0, 2, cBuffers);

		// set viewport and draw to it
		devContext->RSSetViewports(1, &(it->GetViewport()));
		devContext->Draw(6, 0);
	}
	
	swapChain->Present(0,0);
}

void App::Update()
{
	static float elapsed = 0;
	static int frames = 0;

	clock.Tick();
	float time = clock.TotalTime();
	float dt = clock.DeltaTime();

	frames++;
	elapsed += dt;

	// Count and print FPS data to title
	if (elapsed >= 1.0f)
	{
		float secsperframe = 1.0f / (float)frames;
		std::wstringstream ss;
		ss.precision(6);
		ss << "ExplosionDemo FPS: " << frames << "   ||    ms/frame: " << secsperframe;
		SetWindowText(mainWnd, ss.str().c_str());
		frames = 0;
		elapsed = 0;
		ss.flush();
		ss << " lacunarity: " << shaderParams->lacunarity << " gain: " << shaderParams->gain << " offset: " << shaderParams->gradOffset << " scale: " << shaderParams->gradScale << std::endl;
		OutputDebugStringW(ss.str().c_str());
	}

	ScanInput(dt);
	
	for(auto it = viewports.begin(); it != viewports.end(); it++)
	{
		it->SetTime(time);
		it->RefreshBuffer();
	}
}

bool App::CompileShaders()
{
	if(vertexShader)
	{
		vertexShader->Release();
		vertexShader = 0;
	}

	if(pixelShader)
	{
		pixelShader->Release();
		pixelShader = 0;
	}

	// Compile shaders
	ID3D10Blob* vs = 0;
	ID3D10Blob* ps = 0;
	ID3D10Blob* errorblob = 0;
	D3DX11CompileFromFile(L"..\\ExplosionDemo\\effect.fx", 0, 0, "vertexShader", "vs_5_0", 0, 0, 0, &vs, &errorblob, 0);

	if(errorblob)
	{
		MessageBoxA(0, (char*)errorblob->GetBufferPointer(), 0, MB_OK);
		errorblob->Release();
		errorblob = 0;
	}

	D3DX11CompileFromFile(L"..\\ExplosionDemo\\effect.fx", 0, 0, "rayMarcher", "ps_5_0", 0, 0, 0, &ps, &errorblob, 0);
	if(errorblob)
	{
		MessageBoxA(0, (char*)errorblob->GetBufferPointer(), 0, MB_OK);
		errorblob->Release();
		errorblob = 0;
	}

	if(!(vs && ps))
	{
		MessageBox(0, L"Shader compilation failed", 0, MB_OK | MB_ICONWARNING);
		return false;
	}

	dev->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &vertexShader);
	dev->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &pixelShader);

	devContext->VSSetShader(vertexShader, 0, 0);
	devContext->PSSetShader(pixelShader, 0, 0);

	D3D11_INPUT_ELEMENT_DESC inElDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float)*3, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	dev->CreateInputLayout(inElDesc, 2, vs->GetBufferPointer(), vs->GetBufferSize(), &inLayout);

	vs->Release();
	ps->Release();

	return true;

}

void App::ScanInput( float dt )
{
	if(GetFocus() == mainWnd)
	{
		// update shaders if necessary
		if(GetAsyncKeyState(VK_F1) & 0x8000)
			CompileShaders();

		// Camera movement
		if (GetAsyncKeyState('W') & 0x8000)
			viewports[0].Walk(10.f * dt);
		if (GetAsyncKeyState('S') & 0x8000)
			viewports[0].Walk(-10.f * dt);
		if (GetAsyncKeyState('A') & 0x8000)
			viewports[0].Strafe(-10.f * dt);
		if (GetAsyncKeyState('D') & 0x8000)
			viewports[0].Strafe(10.f * dt);
		if(GetAsyncKeyState('Y') & 0x8000)
			shaderParams->lacunarity += 0.01f;
		if(GetAsyncKeyState('H') & 0x8000)
			shaderParams->lacunarity -= 0.01f;
		if(GetAsyncKeyState('U') & 0x8000)
			shaderParams->gain += 0.005f;
		if(GetAsyncKeyState('J') & 0x8000)
			shaderParams->gain -= 0.005f;
		if(GetAsyncKeyState('I') & 0x8000)
			shaderParams->gradOffset += 0.01f;
		if(GetAsyncKeyState('K') & 0x8000)
			shaderParams->gradOffset -=0.01f;
		if(GetAsyncKeyState('O') & 0x8000)
			shaderParams->gradScale += 0.01f;
		if(GetAsyncKeyState('L') & 0x8000)
			shaderParams->gradScale -=0.01f;
	}
}

void App::RBDown( WPARAM btn, int x, int y )
{
	mouseX = x;
	mouseY = y;
	SetCapture(mainWnd);
}

void App::RBUp( WPARAM btn, int x, int y )
{
	ReleaseCapture();
}

void App::MouseMove( WPARAM btn, int x, int y )
{
	if ((btn & MK_RBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x-mouseX));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y-mouseY));

		viewports[0].Pitch(dy);
		viewports[0].RotateY(dx);
	}
	mouseX = x;
	mouseY = y;
}

void App::SetupSamplers()
{
	D3D11_SAMPLER_DESC sampler;
	ZeroMemory(&sampler, sizeof(sampler));
	sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler.MipLODBias = 0.0f;
	sampler.MaxAnisotropy = 1;
	sampler.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler.BorderColor[0] = 0;
	sampler.BorderColor[1] = 0;
	sampler.BorderColor[2] = 0;
	sampler.BorderColor[3] = 0;
	sampler.MinLOD = 0;
	sampler.MaxLOD = D3D11_FLOAT32_MAX;

	HR(dev->CreateSamplerState(&sampler, &permSampler2DSS));
	sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	HR(dev->CreateSamplerState(&sampler, &permGradSamplerSS));
	sampler.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	HR(dev->CreateSamplerState(&sampler, &fireGradSamplerSS));
	sampler.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	HR(dev->CreateSamplerState(&sampler, &groundTextureSamplerSS));
	ID3D11SamplerState* sstates[4];
	sstates[0] = permSampler2DSS;
	sstates[1] = permGradSamplerSS;
	sstates[2] = fireGradSamplerSS;
	sstates[3] = groundTextureSamplerSS;

	devContext->PSSetSamplers(0, 4, sstates);

}

void App::SetupTextures()
{
	float* perm = new float[256*256*4];
	for(size_t i = 0; i< 256;i++)
		for(size_t j = 0; j<256;j++)
		{
			int A			= permutation[j]+i;
			int AA			= permutation[A%256];
			int AB			= permutation[(A+1)%256];
			int B			= permutation[(j+1)%256]+i;
			int BA			= permutation[B%256];
			int BB			= permutation[(B+1)%256];
			perm[(i*256+j)*4]	= AA/255.0f;
			perm[(i*256+j)*4+1]	= AB/255.0f;
			perm[(i*256+j)*4+2]	= BA/255.0f;
			perm[(i*256+j)*4+3]	= BB/255.0f;
		}

	float* grad = new float[256*4];
		for(size_t i = 0; i< 256;i++)
		{
			grad[i*4]			= g[(permutation[i]%16)*3+2];
			grad[i*4+1]		= g[(permutation[i]%16)*3+1];
			grad[i*4+2]		= g[(permutation[i]%16)*3];
			grad[i*4+3]		= i/255.0f;
		}


	// Create permutation table texture
	D3D11_TEXTURE2D_DESC texDesc2d;
	ZeroMemory(&texDesc2d, sizeof(texDesc2d));
	texDesc2d.Width = 256;
	texDesc2d.Height = 256;
	texDesc2d.MipLevels = 1;
	texDesc2d.ArraySize = 1;
	texDesc2d.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc2d.SampleDesc.Count = 1;
	texDesc2d.Usage = D3D11_USAGE_DYNAMIC;
	texDesc2d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc2d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDesc2d.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = perm;
	subData.SysMemPitch = sizeof(float)*4*256;
	subData.SysMemSlicePitch = 0;
	
	HR(dev->CreateTexture2D(&texDesc2d, &subData, &permTexture2D));

	// Create gradient table texture
	D3D11_TEXTURE1D_DESC texDesc1d;
	ZeroMemory(&texDesc1d, sizeof(texDesc1d));
	texDesc1d.Width = 256;
	texDesc1d.MipLevels = 1;
	texDesc1d.ArraySize = 1;
	texDesc1d.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc1d.Usage = D3D11_USAGE_DYNAMIC;
	texDesc1d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc1d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDesc1d.MiscFlags = 0;

	texDesc2d.Height = 1;
	subData.pSysMem = grad;
	subData.SysMemPitch = sizeof(float)*4*256;
	
	HR(dev->CreateTexture1D(&texDesc1d, &subData, &permGradTexture));

	delete perm;
	delete grad;
	
	// Create fire gradient texture
	texDesc1d.Width = 16;
	subData.pSysMem = &fireGrad;
	subData.SysMemPitch = sizeof(float)*4*16;

	HR(dev->CreateTexture1D(&texDesc1d, &subData, &fireGradTexture));

	// Create simplex texture
	
	for(int i=0;i<64;i++)
	{
		float r = simplex4[i][0];
		float g = simplex4[i][1];
		float b = simplex4[i][2];
		float a = simplex4[i][3];
		simplex4[i][0] = r/256.f;
		simplex4[i][1] = g/256.f;
		simplex4[i][2] = b/256.f;
		simplex4[i][3] = a/256.f;
	}
	
	texDesc1d.Width = 64;
	subData.pSysMem = &simplex4;
	subData.SysMemPitch = sizeof(float)*4*64;

	HR(dev->CreateTexture1D(&texDesc1d, &subData, &simplexGradTexture));


	// Create SRVs
	D3D11_SHADER_RESOURCE_VIEW_DESC texSRVDesc;
	ZeroMemory(&texSRVDesc, sizeof(texSRVDesc));
	texSRVDesc.Format = texDesc2d.Format;
	texSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	texSRVDesc.Texture2D.MipLevels = 1;
	texSRVDesc.Texture2D.MostDetailedMip = 0;
	HR(dev->CreateShaderResourceView(permTexture2D, &texSRVDesc, &permTexture2dSRV));

	texSRVDesc.Format = texDesc1d.Format;
	texSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	HR(dev->CreateShaderResourceView(permGradTexture, &texSRVDesc, &permGradTextureSRV));
	HR(dev->CreateShaderResourceView(fireGradTexture, &texSRVDesc, &fireGradTextureSRV));
	HR(dev->CreateShaderResourceView(simplexGradTexture, &texSRVDesc, &simplexGradTextureSRV));

	ID3D11ShaderResourceView* texViews[4];
	texViews[0] = permTexture2dSRV;
	texViews[1] = permGradTextureSRV;
	texViews[2] = fireGradTextureSRV;
	texViews[3] = simplexGradTextureSRV;

	devContext->PSSetShaderResources(0, 4, texViews);

	
}

bool App::InitWindow()
{
	HRESULT hr = 0;

	// Minimal WNDCLASS initialization
	WNDCLASS win;
	win.style						= CS_HREDRAW | CS_VREDRAW;
	win.lpfnWndProc					= wndProc;
	win.cbClsExtra					= 0;
	win.cbWndExtra					= 0;
	win.hInstance					= appInst;
	win.hIcon						= NULL;
	win.hCursor						= LoadCursor(0, IDC_CROSS);
	win.hbrBackground				= (HBRUSH)GetStockObject(NULL_BRUSH);
	win.lpszMenuName				= NULL;
	win.lpszClassName				= L"ExplosionDemoWindow";

	if(!RegisterClass(&win))
	{
		MessageBox(0, L"WNDCLASS registration failed", 0, MB_OK | MB_ICONWARNING);
		return false;
	}

	// Create window
	RECT rect;
	rect.left						= 0;
	rect.top						= 0;
	rect.right						= winWidth;
	rect.bottom						= winHeight;
	DWORD winStyle					= WS_OVERLAPPEDWINDOW | WS_VISIBLE;

	AdjustWindowRect(&rect, winStyle, false);

	int width						= rect.right - rect.left;
	int height						= rect.bottom - rect.top;

	mainWnd	= CreateWindow(L"ExplosionDemoWindow", L"Explosion demo", winStyle, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, appInst, 0);

	if(!mainWnd)
	{
		MessageBox(0, L"Windows creation failed", 0, MB_OK | MB_ICONWARNING);
		return false;
	}

	return true;
}

bool App::InitD3D()
{
	HRESULT hr = 0;
	// Create swap chain desc
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width						= winWidth;
	swapChainDesc.BufferDesc.Height						= winHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator		= 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator	= 1;
	swapChainDesc.BufferDesc.Format						= DXGI_FORMAT_R16G16B16A16_FLOAT;
	swapChainDesc.BufferDesc.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count						= 1;
	swapChainDesc.SampleDesc.Quality					= 0;
	swapChainDesc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount							= 1;
	swapChainDesc.OutputWindow							= mainWnd;
	swapChainDesc.Windowed								= true;
	swapChainDesc.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags									= 0;



	// Init D3D
	D3D_FEATURE_LEVEL featLvl;
	HR(hr = D3D11CreateDeviceAndSwapChain(
		0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		D3D11_CREATE_DEVICE_DEBUG,
		0,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain,
		&dev,
		&featLvl,
		&devContext));

	if(hr < 0 || dev == 0)
	{
		MessageBox(0, L"Unable to create d3dDevice", 0, MB_OK | MB_ICONWARNING);
		return false;
	}

	if(featLvl != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Unfortunately this app needs D3D11 capable device", 0, MB_OK | MB_ICONWARNING);
		return false;
	}

	return true;
}

void App::InitCanvas()
{
	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth				= sizeof(canvas);
	vbd.Usage					= D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags				= D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags			= 0;
	vbd.MiscFlags				= 0;
	D3D11_SUBRESOURCE_DATA srd;
	srd.pSysMem					= &canvas[0];
	HR(dev->CreateBuffer(&vbd, &srd, &vertexBuffer));

	UINT stride					= sizeof(float)*5;
	UINT offset					= 0;

	devContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
}
