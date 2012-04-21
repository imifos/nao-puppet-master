/*
 *  NAO Puppet Master - Game Controller Client
 *  ------------------------------------------
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  A copy of the GNU General Public License can be found here: 
 *  http://www.gnu.org/licenses/.
 *
 *  Author: 
 *    Tasha CARL, 2012, http://lucubratory.eu
 */


#include "StdAfx.h"

#include "InputDeviceStreamHandler.h"
#include "GameControllerClientDlg.h"

/**
 *
 */
CInputDeviceStreamHandler::CInputDeviceStreamHandler(void) {
    memset(&m_lastState,0,sizeof(LastSentState));
}


/**
 *
 */
CInputDeviceStreamHandler::~CInputDeviceStreamHandler(void) {
}


/**
 *
 */
void CInputDeviceStreamHandler::SetSister(CGameControllerClientDlg* guiDlg) {
    m_gameControllerClientDlg=guiDlg;
}


/**
 *
 */
void CInputDeviceStreamHandler::HandleInputDeviceData(DIJOYSTATE2& joystickState) {
    
    // Be sure that we specify our sister class before using any other method
    ASSERT(m_gameControllerClientDlg!=NULL);

    // Ignore all commands as long as we aren't connected
    if (!m_gameControllerClientDlg->m_connected) return;

    HandleInputDeviceButtons(joystickState);
    HandleInputDeviceSliders(joystickState);
}

/**
 *
 */
void CInputDeviceStreamHandler::HandleInputDeviceButtons(DIJOYSTATE2& joystickState) {

    CNaoCommand cmd;

    if (joystickState.rgbButtons[8] & 0x80) cmd.Standup(); 
    if (joystickState.rgbButtons[9] & 0x80) cmd.Sitdown(); 
       
    if (!cmd.IsEmpty()) {
        m_gameControllerClientDlg->log(cmd);
        m_gameControllerClientDlg->toServer(cmd);
    }
}


/**
 *
 */
void CInputDeviceStreamHandler::HandleInputDeviceSliders(DIJOYSTATE2& joystickState) {

    CNaoCommand cmd;

    // Slider range = -1000 to 1000

    //-------------------------------------------------------------
    // 
    //-------------------------------------------------------------

    if (abs(m_lastState.rglSlider0-joystickState.rglSlider[0])>100) {

        m_lastState.rglSlider0=joystickState.rglSlider[0];
        
        int percent=((joystickState.rglSlider[0]+1000)/2)/10;
        if (percent<15) percent=0;
        if (percent>85) percent=100;
        
        cmd.SetBodyStiffness(percent);

        TCHAR text[50];
        StringCchPrintf(text,50,TEXT("%d"),percent);
        m_gameControllerClientDlg->SetDlgItemText(IDC_NAO_STIFF,text);
    }
        
    if (!cmd.IsEmpty()) {
        m_gameControllerClientDlg->toServer(cmd);
    }
}