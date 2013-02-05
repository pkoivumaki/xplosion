#pragma once

// very basic high precision clock
// modified from Frank Luna's GameTimer

class Clock
{
public:
	Clock(void);
	~Clock(void);

	float TotalTime() const;
	float DeltaTime() const;
	float DeltaTimeMul() const;

	void SetSpeed(float speedMultiplier);

	void Tick();
	void Start();
	void Stop();
	void Reset();

private:
	double		secPerCount;
	double		deltaTime;
	double		deltaTimeMul;
	double		time;
	double		speedMul;

	bool		stopped;

	__int64		prevTime;


};

