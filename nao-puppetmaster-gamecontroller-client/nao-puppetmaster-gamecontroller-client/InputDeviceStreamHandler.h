#pragma once

#include "InputDeviceAbstractionLayer.h"


// Manual foward declaration to avoid stress with (on purpose) circular references/includes
class CGameControllerClientDlg;


class CInputDeviceStreamHandler
{
protected:
    CGameControllerClientDlg* m_gameControllerClientDlg;
    
    struct LastSentState {
        LONG rglSlider0;
    };

    struct LastSentState m_lastState;

    void HandleInputDeviceButtons(DIJOYSTATE2& joystickState);
    void HandleInputDeviceSliders(DIJOYSTATE2& joystickState);

public:
    CInputDeviceStreamHandler(void);
    ~CInputDeviceStreamHandler(void);

    void SetSister(CGameControllerClientDlg* guiDlg);
    void HandleInputDeviceData(DIJOYSTATE2& joystickState);
};

