#pragma once
#include "Utils.h"
#include <D3D11.h>

// datatype for shader's constant buffer regarding camera and viewport data
typedef struct RawData_S{
	Vec			pos;
	float		_pad1;
	Vec			xDir;
	float		_pad2;
	Vec			yDir;
	float		_pad3;
	Vec			zDir;
	float		_pad4;
	float		fov;
	float		aRatio;
	float		time;
	float		_pad7;
} RawData;


class Viewport
{
public:
	static D3D11_BUFFER_DESC viewportBufferDesc;

	Viewport(Vec lf, Vec la, Vec vup, float fov, float ar, int w, int h);
	Viewport(const Viewport& other);
	~Viewport(void);

	void Init();

	void SetPosition(const Vec& vec);

	void LookAt(const Vec& pos, const Vec& target, const Vec& up);

	void Strafe(float d);
	void Walk(float d);

	void Pitch(float angle);
	void RotateY(float angle);

	void SetVPSize(int width, int height);

	void SetFieldOfView(float degrees);

	void SetVPPosition(int x, int y);

	D3D11_VIEWPORT GetViewport() const;

	void RefreshBuffer();

	const RawData* GetBufferData() const;

	void SetTime(float t);

	void DebugPrint();
	

private:
	float			fov;
	float			fovLen;
	float			aspectRatio;
	int				width;
	int				height;
	float			time;

	Vec				right;
	Vec				up;
	Vec				look;
	Vec				position;

	D3D11_VIEWPORT	viewport;

	RawData* rawData;
};

