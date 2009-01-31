#include "main.h"
#include "config.h"

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(*x))

// The following line is needed after EmPlugin definition.
_ETL_IMPLEMENT

/*
 * Functions to Export
 */

// The plug-in has been selected from a menu or a toolbar.
void EmPlugin::OnCommand(HWND hwndView)
{
}

// Queries the status of the plug-in, whether the command is enabled and whether
// the plug-in is a checked status.
BOOL EmPlugin::QueryStatus(HWND hwndView,LPBOOL pbChecked)
{
    return TRUE;
}

// When a status is changed, this function is called with the Events parameter.
void EmPlugin::OnEvents(HWND hwndView,UINT nEvent,LPARAM lParam)
{
}

/*
 * Messages to Plug-ins
 */

// Queries whether the plug-in can be uninstalled.
BOOL EmPlugin::QueryUninstall(HWND hDlg)
{
    return TRUE;
}

// Uninstalls the plug-in.
BOOL EmPlugin::SetUninstall(HWND hDlg,LPTSTR pszUninstallCommand,LPTSTR pszUninstallParam)
{
    TCHAR szAsk[80];
    TCHAR szAppName[80];

    LoadString(EEGetInstanceHandle(),IDS_SURE_TO_UNINSTALL,szAsk,sizeof(szAsk)/sizeof(TCHAR));
    LoadString(EEGetInstanceHandle(),IDS_MENU_TEXT,szAppName,sizeof(szAppName)/sizeof(TCHAR));

    if (MessageBox(hDlg,szAsk,szAppName,MB_YESNO|MB_ICONEXCLAMATION)==IDYES) {
        return UNINSTALL_SIMPLE_DELETE;
    }
    return UNINSTALL_FALSE;
}

// Queries whether the property is enabled.
BOOL EmPlugin::QueryProperties(HWND hDlg)
{
    return FALSE;
}

// Requests the plug-in to display the properties.
BOOL EmPlugin::SetProperties(HWND hDlg)
{
    return FALSE;
}

// Called before each Windows message is translated.
BOOL EmPlugin::PreTranslateMessage(HWND hwndView,MSG* pMsg)
{
    return FALSE;
}
