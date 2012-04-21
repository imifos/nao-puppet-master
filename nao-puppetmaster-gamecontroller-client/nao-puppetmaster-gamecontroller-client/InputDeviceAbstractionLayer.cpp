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

//-----------------------------------------------------------------------------
// Initialises and scans the Joystick interface and displays the joystick status 
// in the passed dialog.
// 
// Based on the DirectX SDK joystick example by Microsoft.
//-----------------------------------------------------------------------------

#include "stdafx.h"

#ifndef DIRECTINPUT_VERSION
    #define DIRECTINPUT_VERSION 0x0800
#endif

#define _CRT_SECURE_NO_DEPRECATE

#ifndef _WIN32_DCOM
    #define _WIN32_DCOM
#endif

#include <windows.h>
#include <commctrl.h>
#include <dinput.h>
#include <dinputd.h>
#include <basetsd.h>
#include <assert.h>
#include <oleauto.h>
#include <shellapi.h>

#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )
#include "resource.h"

#include "InputDeviceAbstractionLayer.h"


#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

LPDIRECTINPUT8 g_pDI=NULL;
LPDIRECTINPUTDEVICE8 g_pJoystick=NULL;

HINSTANCE g_hInstDialog=NULL;

struct DI_ENUM_CONTEXT {
    DIJOYCONFIG* pPreferredJoyCfg;
    bool bPreferredJoyCfgValid;
};

// WIN32 code prototypes
HRESULT InitDirectInput(HWND hDlg);
VOID FreeDirectInput();
HRESULT UpdateInputState(HWND hDlg,DIJOYSTATE2* js);
BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext);
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance,VOID* pContext);


/**
 *
 */
CInputDeviceAbstractionLayer::CInputDeviceAbstractionLayer() {
}

/**
 *
 */
CInputDeviceAbstractionLayer::~CInputDeviceAbstractionLayer() {
}


/**
 *
 */
bool CInputDeviceAbstractionLayer::OnInitParentWindow(CWnd* pParentWindow) {

    pParent=pParentWindow;

    if (FAILED(InitDirectInput(pParent->GetSafeHwnd()))) 
        return false;
    else return true;
}


/**
 *
 */
void CInputDeviceAbstractionLayer::OnActivateParentWindow(CWnd* pParent) {
    if (g_pJoystick) 
       g_pJoystick->Acquire();
}


/**
 *
 */
void CInputDeviceAbstractionLayer::OnDestroyParentWindow(CWnd* pParent) {
    FreeDirectInput();
}


/**
 *
 */
bool CInputDeviceAbstractionLayer::OnDirectInputUpdateTicker(CWnd* pParent,DIJOYSTATE2* joystickState) {
    
    if(FAILED(UpdateInputState(pParent->GetSafeHwnd(),joystickState)))
        return false;
    return true;
}



//-----------------------------------------------------------------------------
// WIN32 DirectInput accessors.
// They don't play nice when declared inside a class, so we keep them outside
// and grant access only via the above class.
//-----------------------------------------------------------------------------



/**
 * Get the input device's state, display it and notify the callbacks.
 */
HRESULT UpdateInputState(HWND hDlg,DIJOYSTATE2* js) {

    HRESULT hr;
    TCHAR strText[512] = {0}; // Device state text
    
    // Reset the data to have defined values in case the reading isn't performed.
    // (Which happens e.g. before the application gets the focus).
    memset(js,0,sizeof(DIJOYSTATE2));

    if( NULL == g_pJoystick ) {
        return -1;
    }

    // Poll the device to read the current state
    hr = g_pJoystick->Poll();
    if( FAILED( hr ) ) {

        // DInput is telling us that the input stream has been
        // interrupted. We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done. We
        // just re-acquire and try again.
        hr = g_pJoystick->Acquire();
        while( hr == DIERR_INPUTLOST )
            hr = g_pJoystick->Acquire();

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return -1;
    }

    // Get the input's device state
    if( FAILED( hr = g_pJoystick->GetDeviceState( sizeof( DIJOYSTATE2 ), js ) ) ) {
        return -1; // The device should have been acquired during the Poll()
    }

    // Display joystick state to dialog
    // Axes
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->lX );
    SetWindowText( GetDlgItem( hDlg, IDC_X_AXIS ), strText );
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->lY );
    SetWindowText( GetDlgItem( hDlg, IDC_Y_AXIS ), strText );
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->lZ );
    SetWindowText( GetDlgItem( hDlg, IDC_Z_AXIS ), strText );
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->lRx );
    SetWindowText( GetDlgItem( hDlg, IDC_X_ROT ), strText );
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->lRy );
    SetWindowText( GetDlgItem( hDlg, IDC_Y_ROT ), strText );
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->lRz );
    SetWindowText( GetDlgItem( hDlg, IDC_Z_ROT ), strText );

    // Slider controls
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->rglSlider[0] );
    SetWindowText( GetDlgItem( hDlg, IDC_SLIDER0 ), strText );
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->rglSlider[1] );
    SetWindowText( GetDlgItem( hDlg, IDC_SLIDER1 ), strText );

    // Points of view
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->rgdwPOV[0] );
    SetWindowText( GetDlgItem( hDlg, IDC_POV0 ), strText );
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->rgdwPOV[1] );
    SetWindowText( GetDlgItem( hDlg, IDC_POV1 ), strText );
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->rgdwPOV[2] );
    SetWindowText( GetDlgItem( hDlg, IDC_POV2 ), strText );
    StringCchPrintf( strText, 512, TEXT( "%ld" ), js->rgdwPOV[3] );
    SetWindowText( GetDlgItem( hDlg, IDC_POV3 ), strText );
    
    // Fill up text with which buttons are pressed
    StringCchCopy( strText, 512, TEXT( "" ) );
    for( int i = 0; i < 128; i++ ) {
        if( js->rgbButtons[i] & 0x80 ) {
            TCHAR sz[128];
            StringCchPrintf( sz, 128, TEXT( "%02d " ), i );
            StringCchCat( strText, 512, sz );
        }
    }
    SetWindowText(GetDlgItem(hDlg,IDC_BUTTONS),strText);
 
    return 0;
}




/**
 * Initializes the DirectInput variables.
 */
HRESULT InitDirectInput(HWND hDlg) {

    HRESULT hr;

    // Register with the DirectInput subsystem and get a pointer to a IDirectInput interface we can use.
    // Create a DInput object
    if( FAILED( hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,IID_IDirectInput8,(VOID**)&g_pDI,NULL)))
        return hr;
    
    DIJOYCONFIG PreferredJoyCfg = {0};
    DI_ENUM_CONTEXT enumContext;
    enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
    enumContext.bPreferredJoyCfgValid = false;

    IDirectInputJoyConfig8* pJoyConfig = NULL;
    if( FAILED( hr = g_pDI->QueryInterface( IID_IDirectInputJoyConfig8, ( void** )&pJoyConfig ) ) )
        return hr;

    PreferredJoyCfg.dwSize = sizeof( PreferredJoyCfg );
    if( SUCCEEDED( pJoyConfig->GetConfig( 0, &PreferredJoyCfg, DIJC_GUIDINSTANCE ) ) ) // This function is expected to fail if no joystick is attached
        enumContext.bPreferredJoyCfgValid = true;
    SAFE_RELEASE( pJoyConfig );

    // Look for a simple joystick we can use for this sample program.
    if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,EnumJoysticksCallback,&enumContext, DIEDFL_ATTACHEDONLY ) ) )
        return hr;

    // Make sure we got a joystick
    if( NULL == g_pJoystick )
        return -100;
    

    // Set the data format to "simple joystick" - a predefined data format.
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
    if (FAILED(hr = g_pJoystick->SetDataFormat(&c_dfDIJoystick2)))
        return hr;

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    if (FAILED(hr = g_pJoystick->SetCooperativeLevel(hDlg, DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
        return hr;

    // Enumerate the joystick objects. The callback function enabled user
    // interface elements for objects that are found, and sets the min/max
    // values property for discovered axes.
    hr = g_pJoystick->EnumObjects( EnumObjectsCallback,( VOID* )hDlg, DIDFT_ALL);
    if (FAILED(hr = g_pJoystick->EnumObjects( EnumObjectsCallback,( VOID* )hDlg, DIDFT_ALL)))
        return hr;

    return S_OK;
}



/**
 * Called once for each enumerated joystick. If we find one, create a device 
 * interface on it so we can play with it.
 */
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,VOID* pContext ) {

    DI_ENUM_CONTEXT* pEnumContext = ( DI_ENUM_CONTEXT* )pContext;
    HRESULT hr;

    // Skip anything other than the perferred joystick device as defined by the control panel.  
    // Instead you could store all the enumerated joysticks and let the user pick.
    if( pEnumContext->bPreferredJoyCfgValid &&
        !IsEqualGUID( pdidInstance->guidInstance, pEnumContext->pPreferredJoyCfg->guidInstance ) )
        return DIENUM_CONTINUE;

    // Obtain an interface to the enumerated joystick.
    hr = g_pDI->CreateDevice( pdidInstance->guidInstance, &g_pJoystick, NULL );

    // If it failed, then we can't use this joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if( FAILED( hr ) )
        return DIENUM_CONTINUE;

    // Stop enumeration. Note: we're just taking the first joystick we get. You
    // could store all the enumerated joysticks and let the user pick.
    return DIENUM_STOP;
}



/**
 * Callback function for enumerating objects (axes, buttons, POVs) on a 
 * joystick. This function enables user interface elements for objects
 * that are found to exist, and scales axes min/max values.
 */
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext ) {

    HWND hDlg = ( HWND )pContext;

    static int nSliderCount = 0;  // Number of returned slider controls
    static int nPOVCount = 0;     // Number of returned POV controls

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg;
        diprg.diph.dwSize = sizeof( DIPROPRANGE );
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin = -1000;
        diprg.lMax = +1000;

        // Set the range for the axis
        if( FAILED( g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
            return DIENUM_STOP;

    }
        
    // Set the UI to reflect what objects the joystick supports
    if( pdidoi->guidType == GUID_XAxis )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_X_AXIS ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_X_AXIS_TEXT ), TRUE );
    }
    if( pdidoi->guidType == GUID_YAxis )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_Y_AXIS ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Y_AXIS_TEXT ), TRUE );
    }
    if( pdidoi->guidType == GUID_ZAxis )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_Z_AXIS ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Z_AXIS_TEXT ), TRUE );
    }
    if( pdidoi->guidType == GUID_RxAxis )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_X_ROT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_X_ROT_TEXT ), TRUE );
    }
    if( pdidoi->guidType == GUID_RyAxis )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_Y_ROT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Y_ROT_TEXT ), TRUE );
    }
    if( pdidoi->guidType == GUID_RzAxis )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_Z_ROT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Z_ROT_TEXT ), TRUE );
    }
    if( pdidoi->guidType == GUID_Slider )
    {
        switch( nSliderCount++ )
        {
            case 0 :
                EnableWindow( GetDlgItem( hDlg, IDC_SLIDER0 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_SLIDER0_TEXT ), TRUE );
                break;

            case 1 :
                EnableWindow( GetDlgItem( hDlg, IDC_SLIDER1 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_SLIDER1_TEXT ), TRUE );
                break;
        }
    }
    if( pdidoi->guidType == GUID_POV )
    {
        switch( nPOVCount++ )
        {
            case 0 :
                EnableWindow( GetDlgItem( hDlg, IDC_POV0 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_POV0_TEXT ), TRUE );
                break;

            case 1 :
                EnableWindow( GetDlgItem( hDlg, IDC_POV1 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_POV1_TEXT ), TRUE );
                break;

            case 2 :
                EnableWindow( GetDlgItem( hDlg, IDC_POV2 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_POV2_TEXT ), TRUE );
                break;

            case 3 :
                EnableWindow( GetDlgItem( hDlg, IDC_POV3 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_POV3_TEXT ), TRUE );
                break;
        }
    }

    return DIENUM_CONTINUE;
}




/**
 * Releases the DirectInput resources.
 */
VOID FreeDirectInput() {

    // Unacquire the device one last time just in case 
    // the app tried to exit while the device is still acquired.
    if( g_pJoystick )
        g_pJoystick->Unacquire();

    // Release any DirectInput objects.
    SAFE_RELEASE( g_pJoystick );
    SAFE_RELEASE( g_pDI );
}



