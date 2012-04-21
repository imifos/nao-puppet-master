#pragma once

#ifndef DIRECTINPUT_VERSION
    #define DIRECTINPUT_VERSION 0x0800
#endif

#include <dinput.h>

class CInputDeviceAbstractionLayer {

private:
    CWnd* pParent;

public:
	CInputDeviceAbstractionLayer();
	virtual ~CInputDeviceAbstractionLayer();

    bool OnInitParentWindow(CWnd* pParent);
    void OnActivateParentWindow(CWnd* pParent);
    void OnDestroyParentWindow(CWnd* pParent);
    bool OnDirectInputUpdateTicker(CWnd* pParent,DIJOYSTATE2* joystickState);

};
