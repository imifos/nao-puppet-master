#pragma once

#include "resource.h"		
#include "afxwin.h"

class CAboutDlg : public CDialogEx {

public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

public:
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnEnChangeAboutEdit();
};
