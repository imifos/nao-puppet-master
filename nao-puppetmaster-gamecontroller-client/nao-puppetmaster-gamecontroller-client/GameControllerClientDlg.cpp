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
#include "afxdialogex.h"

#include "InputDeviceAbstractionLayer.h"
#include "InputDeviceStreamHandler.h"

#include "GameControllerClientApp.h"
#include "GameControllerClientDlg.h"
#include "AboutDlg.h"

#pragma comment(lib,"ws2_32.lib")

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif





/**
 * Constructor.
 */
CGameControllerClientDlg::CGameControllerClientDlg(CWnd* pParent /*=NULL*/)
	                    :CDialogEx(CGameControllerClientDlg::IDD, pParent) {

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    
    m_inputDeviceStreamHandler.SetSister(this);
    m_connected=false;

    memset(&m_joystickState,0,sizeof(DIJOYSTATE2));
    memset(&m_lastGoodJoystickState,0,sizeof(DIJOYSTATE2));
}



/**
 * Dialog windows from/to member variable data exchange.
 */
void CGameControllerClientDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LOG_EDIT, m_logEdit);
    DDX_Control(pDX, IDC_CONNECT, m_connectButton);
    DDX_Control(pDX, IDC_DISCONNECT, m_disconnectButton);
    DDX_Control(pDX, IDC_IPADDRESS, m_ipAddressEdit);
    DDX_Control(pDX, IDC_SHUTUP, m_shutUpButton);
    DDX_Control(pDX, IDC_SAYCMD, m_sayButton);
    DDX_Control(pDX, IDC_SAYTREE, m_sayTree);
    DDX_Control(pDX, IDC_SAYBOX, m_sayEdit);
    DDX_Control(pDX, IDC_BEHAVIORTREE, m_behaviorTree);
}




/**
 * Writes the passed text into the log output.
 */
void CGameControllerClientDlg::log(TCHAR* msg) {

    if (m_logEdit.GetWindowTextLength()>=LOGFILESIZE-1000)
        m_logEdit.SetWindowText(_T(""));

    m_logEdit.GetWindowText(m_logfile,LOGFILESIZE-1000);
    
    if (wcslen(msg)>950)
        wcscat_s(m_logfile,_T("Log file entry too long, only 950 characters are allowed"));
    else wcscat_s(m_logfile,msg);

    wcscat_s(m_logfile,_T("\r\n"));
    m_logEdit.SetWindowText(m_logfile);

    m_logEdit.LineScroll(m_logEdit.GetLineCount());
}



/**
 * Writes the passed NAO command as text into the log output, shorting the text if needed.
 */
void CGameControllerClientDlg::log(CNaoCommand& cmd) {
   
    CString* c=cmd.GetShortenString(120);
    log(c->GetBuffer(0));
    delete c;
}



/**
 * Sends the passed command to the PUMA server. Returns false if we aren't
 * connected and the data cannot be sent.
 */
bool CGameControllerClientDlg::toServer(CNaoCommand& command) {

    // No need to check 'm_connected'

    if (command.IsEmpty()) return true; // just ignore, error handling is not our business

    if (m_socket.Send(command.GetRaw(),strlen(command.GetRaw()),0)==SOCKET_ERROR) {

        OnServerConnectionStateChange(false);

        TCHAR text[200];
        if (GetLastError()==0)
            StringCchPrintf(text,200,TEXT("Unable to send data. We have been disconnected."));
        else 
            StringCchPrintf(text,200,TEXT("Unable to send data! Error Code: %d"),GetLastError());
        log(text);

        m_socket.Close();
        return false;
    }

    return true;
}



/**
 * Pre-filters some event messages.
 */
BOOL CGameControllerClientDlg::PreTranslateMessage(MSG* pMsg) {

    // Enter key in say dialog
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && GetFocus()==&m_sayEdit) {
        OnBnClickedSaycmd();
        return TRUE;
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}




BEGIN_MESSAGE_MAP(CGameControllerClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_ACTIVATE()
    ON_WM_DESTROY()
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_CONNECT, &CGameControllerClientDlg::OnBnClickedConnect)
    ON_BN_CLICKED(IDC_DISCONNECT, &CGameControllerClientDlg::OnBnClickedDisconnect)
    ON_NOTIFY(NM_CLICK, IDC_SAYTREE, &CGameControllerClientDlg::OnNMClickSaytree)
    ON_NOTIFY(NM_RCLICK, IDC_SAYTREE, &CGameControllerClientDlg::OnNMRClickSaytree)
    ON_NOTIFY(TVN_DELETEITEM, IDC_SAYTREE, &CGameControllerClientDlg::OnTvnDeleteitemSaytree)
    ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_SAYTREE, &CGameControllerClientDlg::OnTvnBeginlabeleditSaytree)
    ON_NOTIFY(TVN_SELCHANGED, IDC_SAYTREE, &CGameControllerClientDlg::OnTvnSelchangedSaytree)
    ON_BN_CLICKED(IDC_SAYCMD, &CGameControllerClientDlg::OnBnClickedSaycmd)
    ON_BN_CLICKED(IDC_CANCEL, &CGameControllerClientDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_SHUTUP, &CGameControllerClientDlg::OnClickedShutup)
END_MESSAGE_MAP()


 

/**
 * Dialog initialisation.
 */
BOOL CGameControllerClientDlg::OnInitDialog() {

    CDialogEx::OnInitDialog();
    
    // Add "About..." menu item to system menu.
    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX); 
    ASSERT(IDM_ABOUTBOX < 0xF000); 
    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        pSysMenu->AppendMenu(MF_SEPARATOR);
        pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, _T("About..."));
    }

    // Set the icon for this dialog.  The framework does this automatically
    // when the application's main window is not a dialog
    SetIcon(m_hIcon,TRUE);		// Set big icon
    SetIcon(m_hIcon,FALSE);		// Set small icon

    ShowWindow(SW_MINIMIZE);
    
    if (!m_inputDevice.OnInitParentWindow(this)) {
        log(_T("Error Init Joystick/Game Controller Device. Continuing without!"));
    }
    
    SetTimer(9999,1000/30,NULL);
    
    m_ipAddressEdit.SetAddress(127,0,0,1);
    OnServerConnectionStateChange(false);

    log(_T("Initialisation done."));
    log(_T("MAKE SURE THAT THE STIFFNESS SHIFTER IS CORRECTLY SET BEFORE CONNECTING!"));
    
	return TRUE;  // return TRUE unless you set the focus to a control
}



/**
 * Inactivates the classic end dialog handler.
 */
void CGameControllerClientDlg::OnOK() {
}



/**
 * Inactivates the classic end dialog handler.
 */
void CGameControllerClientDlg::OnCancel() {
}



/**
 * System Command Handler.
 */
void CGameControllerClientDlg::OnSysCommand(UINT nID, LPARAM lParam) {
	
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}



/**
 * If you add a minimize button to your dialog, you will need the code below
 * to draw the icon. For MFC applications using the document/view model,
 * this is automatically done for you by the framework.
 */
void CGameControllerClientDlg::OnPaint() {

	if (IsIconic()) {

		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialogEx::OnPaint();
	}
}



/**
 * The system calls this function to obtain the cursor to display while the user drags
 * the minimized window.
 */
HCURSOR CGameControllerClientDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}



/**
 * Activate window event handler used to re-aquire the joystick after focus loose.
 */
void CGameControllerClientDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) {
    CDialogEx::OnActivate(nState, pWndOther, bMinimized);
    m_inputDevice.OnActivateParentWindow(this);
}



/**
 * Window destroy event handler.
 */
void CGameControllerClientDlg::OnDestroy() {
    m_inputDevice.OnDestroyParentWindow(this);
    CDialogEx::OnDestroy();
}



/**
 * Timer event handler that queries the joystick and invokes command execution x times per second.
 */
void CGameControllerClientDlg::OnTimer(UINT_PTR nIDEvent) {

    if (!m_inputDevice.OnDirectInputUpdateTicker(this,&m_joystickState)) {
        memcpy(&m_joystickState,&m_lastGoodJoystickState,sizeof(DIJOYSTATE2));
    }
    else 
        memcpy(&m_lastGoodJoystickState,&m_joystickState,sizeof(DIJOYSTATE2));
    
    m_inputDeviceStreamHandler.HandleInputDeviceData(m_joystickState);

    CDialogEx::OnTimer(nIDEvent);
}



/**
 * Connects to the specified Puppet Master server.
 */
void CGameControllerClientDlg::OnBnClickedConnect() {

    BYTE nField0,nField1,nField2,nField3;
    TCHAR buf[200];

    UpdateData(TRUE);
	
    if (m_ipAddressEdit.IsBlank()) {
        MessageBox(_T("Please specify a valid IP address"));
        return;
    }

    m_connectButton.EnableWindow(false);

    m_ipAddressEdit.GetAddress(nField0,nField1,nField2,nField3);
    StringCchPrintf(buf,200,TEXT("Connecting to server at %d.%d.%d.%d : %d"),nField0,nField1,nField2,nField3,PUMA_SERVER_TCPIP_PORT);
    log(buf);
     
    if (!m_socket.Create()) {
        OnServerConnectionStateChange(false);
        StringCchPrintf(buf,200,TEXT("System error while creating the socket object. Network connection will not be possible. Error Code: %d"),GetLastError());
        log(buf);
        return;
    }

    StringCchPrintf(buf,200,TEXT("%d.%d.%d.%d"),nField0,nField1,nField2,nField3);
    if (!m_socket.Connect(buf,PUMA_SERVER_TCPIP_PORT)) {
        OnServerConnectionStateChange(false);
        StringCchPrintf(buf,200,TEXT("Unable to connect to server! Error Code: %d"),GetLastError());
        log(buf);
        m_socket.Close();
        return;
    }

    BOOL fOptval = TRUE;
    m_socket.SetSockOpt(TCP_NODELAY,&fOptval,sizeof(fOptval),IPPROTO_TCP);

    if (m_socket.Send(PUMA_SERVER_APPKEY,strlen(PUMA_SERVER_APPKEY),0)==SOCKET_ERROR) {
        OnServerConnectionStateChange(false);
        StringCchPrintf(buf,200,TEXT("Unable to send application key to server! Error Code: %d"),GetLastError());
        log(buf);
        m_socket.Close();
        return;
    }

    OnServerConnectionStateChange(true);

    log(_T("Connected!"));
}



/**
 * Sets the GUI state in function of the network connection
 */
void CGameControllerClientDlg::OnServerConnectionStateChange(bool connected) {

    m_connected=connected;
    
    m_connectButton.EnableWindow(!connected);

    m_disconnectButton.EnableWindow(connected);
    m_shutUpButton.EnableWindow(connected);
    m_sayButton.EnableWindow(connected);
    m_sayTree.EnableWindow(connected);
    m_sayEdit.EnableWindow(connected);
    m_behaviorTree.EnableWindow(connected);
}



/**
 * Disconnects from the Puppet Master server.
 */
void CGameControllerClientDlg::OnBnClickedDisconnect() {
    
    log(_T("Disconnecting from Server."));
    m_socket.Close();

    OnServerConnectionStateChange(false);
}



/**
 * 
 */
void CGameControllerClientDlg::OnBnClickedSaycmd() {
    
    TCHAR text[1024];
    m_sayEdit.GetWindowText(text,1024);
    m_sayEdit.SetWindowText(_T(""));

    CNaoCommand cmd;
    cmd.Say(text);
    log(cmd);
    toServer(cmd);
}



/**
 * Closes the main dialog window.
 */
void CGameControllerClientDlg::OnBnClickedCancel() {
     EndDialog(0);
}



/**
 * 
 */
void CGameControllerClientDlg::OnNMClickSaytree(NMHDR *pNMHDR, LRESULT *pResult)
{
    *pResult = 0;
}


/**
 * 
 */
void CGameControllerClientDlg::OnNMRClickSaytree(NMHDR *pNMHDR, LRESULT *pResult)
{
    *pResult = 0;
}


/**
 * 
 */
void CGameControllerClientDlg::OnTvnDeleteitemSaytree(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    *pResult = 0;
}


/**
 * 
 */
void CGameControllerClientDlg::OnTvnBeginlabeleditSaytree(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
    *pResult = 0;
}


/**
 * 
 */
void CGameControllerClientDlg::OnTvnSelchangedSaytree(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    *pResult = 0;
}



void CGameControllerClientDlg::OnClickedShutup()
{
    CNaoCommand cmd;
    cmd.Shutup();
    log(_T("Shut-up!"));
    toServer(cmd);
}



