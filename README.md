# KeePassXT
KeePass with fingerprint, use Windows Biometric Framework API

# Set config path
- AdditionalIncludeDirectories E:\OpenSource\boost_1_61_0
- AdditionalLibraryDirectories E:\OpenSource\boost_1_61_0\stage\lib

# Update
- add file bio.h bio.cpp at WinGUI
- del code at stdafs.h
    `#`ifndef WINVER
    `#`define WINVER 0x0600
    `#`endif
    `#`ifndef _WIN32_WINNT
    `#`define _WIN32_WINNT 0x0600
    `#`endif
    `#`ifndef _WIN32_WINDOWS
    `#`define _WIN32_WINDOWS 0x0410
    `#`endif
    `#`ifndef _WIN32_IE
    `#`define _WIN32_IE 0x0600
    `#`endif
    `#`define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
    `#`define _AFX_ALL_WARNINGS

- del code at stdafs.h
    `#`if (_WIN32_WINNT != 0x0600)
    `#`error _WIN32_WINNT has been redefined by MFC headers!
    `#`endif

- del code at NewGUICommon.cpp::NewGUI_GetNonClientMetrics()
    BOOST_STATIC_ASSERT(false);

- add code at PasswordDlg.h
    afx_msg LRESULT OnSetPassword(WPARAM wParam, LPARAM lParam);

- add code at PasswordDlg.cpp
    `#`include "bio.h"

- add code at PasswordDlg.cpp::BEGIN_MESSAGE_MAP
    ON_MESSAGE(UM_SET_PASSWORD, OnSetPassword)

- add code at PasswordDlg.cpp
    LRESULT CPasswordDlg::OnSetPassword(WPARAM wParam, LPARAM lParam)
    {
        m_lpKey = (LPTSTR)wParam;
        return 0;
    }

- add code at PasswordDlg.cpp::OnInitDialog()
    _beginthread(check_user, 0, (void*)this->m_hWnd);

- del code at PasswordDlg.cpp::OnOK()
    ASSERT((m_lpKey == NULL) && (m_lpKey2 == NULL));
    m_lpKey = m_pEditPw.GetPassword();

- add code at PasswordDlg.cpp::OnOK()
    if (NULL == m_lpKey)
    {
        m_lpKey = m_pEditPw.GetPassword();
    }

# Compile
- KeePassLibCpp\Util\PwQualityEst.cpp(402): error C2001: 
    del code case L'?:
- Util\SprEngine\SprEncoding.cpp(59): error C2001:
    del code case _T('?): sb += _T("%({NUMPAD0}{NUMPAD1}{NUMPAD8}{NUMPAD0})"); break;