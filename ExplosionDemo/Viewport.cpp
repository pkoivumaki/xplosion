#include "Viewport.h"
#include <WinBase.h>
#include <xnamath.h>
#include <sstream>

// viewport constant buffer description
D3D11_BUFFER_DESC Viewport::viewportBufferDesc = {
	sizeof(RawData_S),
	D3D11_USAGE_DYNAMIC,
	D3D11_BIND_CONSTANT_BUFFER,
	D3D11_CPU_ACCESS_WRITE,
	0
};

Viewport::Viewport(Vec lf, Vec la, Vec vup, float fov, float ar, int w, int h) :
		fov(fov),
		aspectRatio(ar),
		width(w),
		height(h),
		time(0)
{
	rawData = new RawData;
	ZeroMemory(rawData, sizeof(RawData));
	viewport.Height					= static_cast<float>(height);
	viewport.MaxDepth				= 1.f;
	viewport.MinDepth				= 0.f;
	viewport.TopLeftX				= 0;
	viewport.TopLeftY				= 0;
	viewport.Width					= static_cast<float>(width);

	LookAt(lf, la, vup);

	// Just to calculate fov length
	SetFieldOfView(fov);
}

Viewport::Viewport( const Viewport& other )
{
	fov = other.fov;
	fovLen = other.fovLen;
	aspectRatio = other.aspectRatio;
	width = other.width;
	height = other.height;
	time = other.time;
	right = other.right;
	up = other.up;
	look = other.look;
	position = other.position;
	viewport = other.viewport;
	rawData = new RawData();
	memcpy(rawData, other.rawData, sizeof(RawData));
	SetFieldOfView(fov);
}


Viewport::~Viewport(void)
{
	delete rawData;
}


void Viewport::SetVPSize( int w, int h )
{
	viewport.Width = static_cast<float>(w);
	viewport.Height = static_cast<float>(h);
	aspectRatio = viewport.Width / viewport.Height;
	rawData->aRatio = aspectRatio;
}

D3D11_VIEWPORT Viewport::GetViewport() const
{
	return viewport;
}

void Viewport::SetVPPosition( int x, int y )
{
	viewport.TopLeftX = static_cast<float>(x);
	viewport.TopLeftY = static_cast<float>(y);
}

void Viewport::RefreshBuffer()
{
	//DebugPrint();
	rawData->pos = position;
	rawData->xDir = right;
	rawData->yDir = up;
	rawData->zDir = look;
	rawData->fov = (float)TWOPI*fov/360.f;
	rawData->aRatio = aspectRatio;
	rawData->time = time;

}

void Viewport::DebugPrint()
{
	std::stringstream ss;
	ss << "pos: x: " << position.x << " y: " << position.y << " z: " << position.z << std::endl;
	ss << "right: x: " << right.x << " y: " << right.y << " z: " << right.z << std::endl;
	ss << "up: x: " << up.x << " y: " << up.y << " z: " << up.z << std::endl;
	ss << "look: x: " << look.x << " y: " << look.y << " z: " << look.z << std::endl;
	ss << "fov: " << fov << " fovlen: " << fovLen << std::endl;
	OutputDebugStringA(ss.str().c_str());
}


const RawData* Viewport::GetBufferData() const
{
	return rawData;
}


void Viewport::SetTime( float t )
{
	time = t;
}

void Viewport::SetFieldOfView( float degrees )
{
	fov = degrees;
	fovLen = 1.f/tan(fov/2.f / 360.f * (float)TWOPI);
}

void Viewport::SetPosition( const Vec& vec )
{
	position = vec;
}

void Viewport::LookAt( const Vec& pos, const Vec& target, const Vec& u )
{	
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&u);
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(P,T));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(U, L));
	XMVECTOR V = XMVector3Cross(L, R);

	XMStoreFloat3(&position, P);
	XMStoreFloat3(&look, L);
	XMStoreFloat3(&right, R);
	XMStoreFloat3(&up, V);
	//look.x = -look.x;

}

void Viewport::Pitch( float angle )
{
	XMMATRIX rot = XMMatrixRotationAxis(XMLoadFloat3(&right), angle);
	XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), rot));
	XMStoreFloat3(&look, XMVector3TransformNormal(XMLoadFloat3(&look), rot));
}

void Viewport::RotateY( float angle )
{
	XMMATRIX rot = XMMatrixRotationY(angle);
	XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), rot));
	XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), rot));
	XMStoreFloat3(&look, XMVector3TransformNormal(XMLoadFloat3(&look), rot));
}

void Viewport::Strafe( float d )
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&right);
	XMVECTOR p = XMLoadFloat3(&position);
	XMStoreFloat3(&position, XMVectorMultiplyAdd(s, r, p));
}

void Viewport::Walk( float d )
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&look);
	XMVECTOR p = XMLoadFloat3(&position);
	XMStoreFloat3(&position, XMVectorMultiplyAdd(s, l, p));
	if(position.y < -3.2)
		position.y = -3.2;
}
