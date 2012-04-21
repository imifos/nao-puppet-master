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

#include "stdafx.h"
#include "GameControllerClientApp.h"
#include "GameControllerClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CGameControllerClientApp, CWinApp)
	//ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


/**
 * CGameControllerClientApp construction
 */
CGameControllerClientApp::CGameControllerClientApp() {

	// Support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// Add construction code here
	// Place all significant initialization in InitInstance
}


// The one and only CGameControllerClientApp object
CGameControllerClientApp theApp;


/**
 * CGameControllerClientApp initialization
 */
BOOL CGameControllerClientApp::InitInstance() {

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	// Set this to include all the common control classes you want to use in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

    // About WinSockets, consider to read this:
    // http://tangentsoft.net/wskfaq/articles/csocket.html
	if (!AfxSocketInit()) {
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need.

	SetRegistryKey(_T("NAO Puppet Master Game Controller Client"));

	CGameControllerClientDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	if (nResponse == IDOK) {
		// Place code here to handle when the dialog is dismissed with OK
	}
	else if (nResponse == IDCANCEL)	{
		// Place code here to handle when the dialog is dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL) {
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the application's message pump.
	return FALSE;
}

