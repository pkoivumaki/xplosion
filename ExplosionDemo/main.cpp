#include "App.h"
#include <sstream>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	App app(hInstance, 800, 600);
	if(!app.Init())
	{
		DWORD error	= 0;
		error = GetLastError();
		std::basic_ostringstream<TCHAR> ss;
		ss << "Initialization error: " << error;
		std::wstring str(ss.str());
		MessageBox(0, str.c_str()/* L"Windows creation failed"*/, 0, MB_OK | MB_ICONWARNING);
		return 0;
	}

	return app.Run();
}