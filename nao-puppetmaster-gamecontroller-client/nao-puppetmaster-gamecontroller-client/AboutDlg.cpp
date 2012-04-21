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

#include "AboutDlg.h"

/**
 *
 */
CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD) {
}


/**
 *
 */
BOOL CAboutDlg::OnInitDialog() {

    CDialogEx::OnInitDialog();
        
    TCHAR* t=_TEXT("NAO Puppet Master - Game Controller Client\r\n")   
        _T("\r\n")
        _T("This program is free software: you can redistribute it and/or modify ")
        _T("it under the terms of the GNU General Public License as published by ")
        _T("the Free Software Foundation, either version 3 of the License, or ")
        _T("(at your option) any later version. ")
        _T("\r\n\r\n")
        _T("This program is distributed in the hope that it will be useful, ")
        _T("but WITHOUT ANY WARRANTY; without even the implied warranty of ")
        _T("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ")
        _T("GNU General Public License for more details.")
        _T("\r\n\r\n")
        _T("A copy of the GNU General Public License can be found here: ")
        _T("http://www.gnu.org/licenses")
        _T("\r\n\r\n")
        _T("Author:\r\n")
        _T("    Tasha CARL, 2012, http://lucubratory.eu");

    SetDlgItemText(IDC_ABOUT_EDIT,t);

    return TRUE;  // return TRUE unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
    ON_EN_CHANGE(IDC_ABOUT_EDIT, &CAboutDlg::OnEnChangeAboutEdit)
END_MESSAGE_MAP()


/**
 *
 */
void CAboutDlg::OnEnChangeAboutEdit(){    
}
