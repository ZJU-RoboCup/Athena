#ifndef _XBOX_CONTROLLER_H_
#define _XBOX_CONTROLLER_H_

// No MFC
#define WIN32_LEAN_AND_MEAN

// We need the Windows Header and the XInput Header
#include <windows.h>
#include <XInput.h>

#pragma comment(lib, "XInput9_1_0.lib")

// XBOX Controller Class Definition
class XBoxController
{
public:
	XBoxController(int playerNumber);
	XINPUT_STATE GetState();
	bool IsConnected();
	void Vibrate(int leftVal = 0, int rightVal = 0);
private:
	XINPUT_STATE _controllerState;
	int _controllerNum;
};

#endif // ~_XBOX_CONTROLLER_H_