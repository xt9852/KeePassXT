#include "StdAfx.h"
#include <process.h>
#include <winbio.h>
#include <winbio_types.h>
#include "bio.h"
#include "../KeePassLibCpp/Util/Base64.h"
#include "../KeePassLibCpp/Util/Base64.h"

#pragma comment(lib, "Winbio.lib")

//-----------------------------------------------------------------
/*
1, del code at stdafs.h
    #ifndef WINVER
    #define WINVER 0x0600
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
    #endif
    #ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS 0x0410
    #endif
    #ifndef _WIN32_IE
    #define _WIN32_IE 0x0600
    #endif
    #define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
    #define _AFX_ALL_WARNINGS

2, del code at stdafs.h
    #if (_WIN32_WINNT != 0x0600)
    #error _WIN32_WINNT has been redefined by MFC headers!
    #endif

3, del code at NewGUICommon.cpp::NewGUI_GetNonClientMetrics()
    BOOST_STATIC_ASSERT(false);

4, add code at PasswordDlg.h
    afx_msg LRESULT OnSetPassword(WPARAM wParam, LPARAM lParam);

5, add code at PasswordDlg.cpp
    #include "bio.h"

6, add code at PasswordDlg.cpp::BEGIN_MESSAGE_MAP
    ON_MESSAGE(UM_SET_PASSWORD, OnSetPassword)

7, add code at PasswordDlg.cpp
    LRESULT CPasswordDlg::OnSetPassword(WPARAM wParam, LPARAM lParam)
    {
        m_lpKey = (LPTSTR)wParam;
        return 0;
    }

8, add code at PasswordDlg.cpp::OnInitDialog()
    _beginthread(check_user, 0, (void*)this->m_hWnd);

9, del code at PasswordDlg.cpp::OnOK()
    ASSERT((m_lpKey == NULL) && (m_lpKey2 == NULL));
    m_lpKey = m_pEditPw.GetPassword();

10, add code at PasswordDlg.cpp::OnOK()
    if (NULL == m_lpKey)
    {
        m_lpKey = m_pEditPw.GetPassword();
    }
*/
//-----------------------------------------------------------------

HRESULT GetCurrentUserIdentity(__inout PWINBIO_IDENTITY Identity)
{
    // Declare variables.
    HRESULT hr = S_OK;
    HANDLE tokenHandle = NULL;
    DWORD bytesReturned = 0;

    struct
    {
        TOKEN_USER tokenUser;
        BYTE buffer[SECURITY_MAX_SID_SIZE];

    } tokenInfoBuffer;

    // Zero the input identity and specify the type.
    ZeroMemory(Identity, sizeof(WINBIO_IDENTITY));
    Identity->Type = WINBIO_ID_TYPE_NULL;

    // Open the access token associated with the
    // current process
    if (!OpenProcessToken(
        GetCurrentProcess(),            // Process handle
        TOKEN_READ,                     // Read access only
        &tokenHandle))                  // Access token handle
    {
        DWORD win32Status = GetLastError();
        wprintf_s(L"Cannot open token handle: %d\n", win32Status);
        hr = HRESULT_FROM_WIN32(win32Status);
        goto e_Exit;
    }

    // Zero the tokenInfoBuffer structure.
    ZeroMemory(&tokenInfoBuffer, sizeof(tokenInfoBuffer));

    // Retrieve information about the access token. In this case,
    // retrieve a SID.
    if (!GetTokenInformation(
        tokenHandle,                    // Access token handle
        TokenUser,                      // User for the token
        &tokenInfoBuffer.tokenUser,     // Buffer to fill
        sizeof(tokenInfoBuffer),        // Size of the buffer
        &bytesReturned))                // Size needed
    {
        DWORD win32Status = GetLastError();
        wprintf_s(L"Cannot query token information: %d\n", win32Status);
        hr = HRESULT_FROM_WIN32(win32Status);
        goto e_Exit;
    }

    // Copy the SID from the tokenInfoBuffer structure to the
    // WINBIO_IDENTITY structure. 
    CopySid(
        SECURITY_MAX_SID_SIZE,
        Identity->Value.AccountSid.Data,
        tokenInfoBuffer.tokenUser.User.Sid
        );

    // Specify the size of the SID and assign WINBIO_ID_TYPE_SID
    // to the type member of the WINBIO_IDENTITY structure.
    Identity->Value.AccountSid.Size = GetLengthSid(tokenInfoBuffer.tokenUser.User.Sid);
    Identity->Type = WINBIO_ID_TYPE_SID;

e_Exit:

    if (tokenHandle != NULL)
    {
        CloseHandle(tokenHandle);
    }

    return hr;
}

class CPasswordDlg;

void check_user(void *param)
{
    HWND wnd = (HWND)param;
    WINBIO_IDENTITY identity = { 0 };

    // Find the identity of the user.
    HRESULT hr = GetCurrentUserIdentity(&identity);

    if (FAILED(hr))
    {
        MessageBox(NULL, "User identity not found. ", "Error", MB_OK);
        return;
    }

    WINBIO_SESSION_HANDLE sessionHandle = NULL;

    // Connect to the system pool. 
    hr = WinBioOpenSession(
        WINBIO_TYPE_FINGERPRINT,    // Service provider
        WINBIO_POOL_SYSTEM,         // Pool type
        WINBIO_FLAG_DEFAULT,        // Configuration and access
        NULL,                       // Array of biometric unit IDs
        0,                          // Count of biometric unit IDs
        NULL,                       // Database ID
        &sessionHandle              // [out] Session handle
        );

    if (FAILED(hr))
    {
        MessageBox(NULL, "WinBioOpenSession failed. ", "Error", MB_OK);
        return;
    }

    BOOLEAN match = FALSE;
    WINBIO_UNIT_ID unitId = 0;
    WINBIO_REJECT_DETAIL rejectDetail = 0;
    WINBIO_BIOMETRIC_SUBTYPE subFactor = WINBIO_SUBTYPE_ANY;

    do
    {
        // Verify a biometric sample.
        hr = WinBioVerify(
            sessionHandle,
            &identity,
            subFactor,
            &unitId,
            &match,
            &rejectDetail
            );

        if (SUCCEEDED(hr))
        {
            DWORD base64_len = identity.Value.AccountSid.Size * 2;
            BYTE *base64 = new BYTE[base64_len];
            CBase64Codec::Encode(identity.Value.AccountSid.Data, identity.Value.AccountSid.Size, base64, &base64_len);

            ::SendMessage(wnd, UM_SET_PASSWORD, (WPARAM)base64, NULL);
            ::PostMessage(wnd, WM_COMMAND, IDOK, NULL);
        }

    } while (!match);

    if (sessionHandle != NULL)
    {
        WinBioCloseSession(sessionHandle);
        sessionHandle = NULL;
    }
}

