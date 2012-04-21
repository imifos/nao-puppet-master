
#pragma once

#include "afxwin.h"

#include "Resource.h"

#include "InputDeviceAbstractionLayer.h"
#include "InputDeviceStreamHandler.h"
#include "NaoCommand.h"



#define LOGFILESIZE 30000
#define PUMA_SERVER_TCPIP_PORT 30457
#define PUMA_SERVER_APPKEY "SAUERKRAUT"
    
    

class CGameControllerClientDlg : public CDialogEx {
        

public:
	CGameControllerClientDlg(CWnd* pParent = NULL);	

    // Logging
    void log(TCHAR* msg);
    void log(CNaoCommand& cmd);
    	

protected:
    
    enum { IDD = IDD_NAOPUMA_MAIN_DIALOG };

    // GUI
    CEdit m_logEdit;
    CSocket m_socket;
    CButton m_connectButton;
    CButton m_disconnectButton;
    CIPAddressCtrl m_ipAddressEdit;
    HICON m_hIcon;
    CButton m_shutUpButton;
    CButton m_sayButton;
    CTreeCtrl m_sayTree;
    CEdit m_sayEdit;
    CTreeCtrl m_behaviorTree;
    
    // Logging
    TCHAR m_logfile[LOGFILESIZE+1];
    
    // The input device stream handler is just the extended arm if the GUI, 
    // an alternative way to handle input events. The clean solution would
    // be to pass all resulting events thru the dialog to the "executor", but
    // for performance, simplicity and readability we place the class in 
    // "parallel" to the dialog by declaring it as 'friend'.
    // Some input devices also modify the GUI (selection). This is much easier
    // having a direct access.
    friend class CInputDeviceStreamHandler;

    CInputDeviceStreamHandler m_inputDeviceStreamHandler;

    // Input controller HAL
    CInputDeviceAbstractionLayer m_inputDevice;
    DIJOYSTATE2 m_joystickState;
    DIJOYSTATE2 m_lastGoodJoystickState;
    
    // Network
    bool m_connected;

    // NAO 
    //CNaoCommander m_naoCommander;

protected:
	
    // Network
    bool toServer(CNaoCommand& command);

    // Windows / Messages
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    BOOL PreTranslateMessage(MSG* pMsg);
    
    virtual BOOL OnInitDialog();
    void OnOK();
    void OnCancel();
    void OnServerConnectionStateChange(bool connected);
        
    DECLARE_MESSAGE_MAP()

    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnIpnFieldchangedIpaddress2(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedConnect();
    afx_msg void OnBnClickedDisconnect();
    afx_msg void OnNMClickSaytree(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMRClickSaytree(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTvnDeleteitemSaytree(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTvnBeginlabeleditSaytree(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTvnSelchangedSaytree(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedSaycmd();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnClickedShutup();
    
    
};
