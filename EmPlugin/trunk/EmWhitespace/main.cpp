#include "main.h"
#include "config.h"

// The following line is needed after EmPlugin definition.
_ETL_IMPLEMENT

EmPlugin::EmPlugin()
{
    // Create a pop-up menu with the desired entries.
    m_menu_handle=CreatePopupMenu();
    if (!m_menu_handle) {
        return;
    }

    AppendMenu(m_menu_handle,MF_STRING,MI_SHOW_RETURNS,_T("Show Returns"));
    AppendMenu(m_menu_handle,MF_STRING,MI_SHOW_EOF,_T("Show End of File"));
    AppendMenu(m_menu_handle,MF_STRING,MI_SHOW_TABS,_T("Show Tabs"));
    AppendMenu(m_menu_handle,MF_STRING,MI_SHOW_SPACES,_T("Show Spaces"));

    // Separate toggle items from action items.
    AppendMenu(m_menu_handle,MF_SEPARATOR,0,NULL);

    AppendMenu(m_menu_handle,MF_STRING,MI_SPACES_TO_TABS,_T("Convert Spaces to Tabs"));
    AppendMenu(m_menu_handle,MF_STRING,MI_TABS_TO_SPACES,_T("Convert Tabs to Spaces"));
    AppendMenu(m_menu_handle,MF_STRING,MI_TRIM_WHITESPACES,_T("Trim Trailing Whitespaces"));
}

EmPlugin::~EmPlugin()
{
    DestroyMenu(m_menu_handle);
}

/*
 * Functions to Export
 */

// The plug-in has been selected from a menu or a toolbar.
void EmPlugin::OnCommand(HWND hwndView)
{
    // Get the mouse cursor position to display the menu there.
    POINT point;
    if (!m_menu_handle || !GetCursorPos(&point)) {
        return;
    }

    // Set the menu check-marks according to the current configuration.
    EmConfig config(hwndView);
    CCustomizeInfo info;
    if (!config.GetConfig(info)) {
        return;
    }

    CheckMenuItem(m_menu_handle,MI_SHOW_RETURNS,info.m_bShowCR?MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(m_menu_handle,MI_SHOW_EOF,info.m_bShowEOF?MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(m_menu_handle,MI_SHOW_TABS,info.m_bShowTab?MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(m_menu_handle,MI_SHOW_SPACES,info.m_bShowSpace?MF_CHECKED:MF_UNCHECKED);

    // Display the pop-up menu and wait for a choice.
    UINT item=(UINT)TrackPopupMenuEx(m_menu_handle,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,point.x,point.y,hwndView,NULL);
    switch (item) {
        case MI_SHOW_RETURNS: {
            info.m_bShowCR=!info.m_bShowCR;
            config.SetConfig(info);
            break;
        }
        case MI_SHOW_EOF: {
            info.m_bShowEOF=!info.m_bShowEOF;
            config.SetConfig(info);
            break;
        }
        case MI_SHOW_TABS: {
            info.m_bShowTab=!info.m_bShowTab;
            config.SetConfig(info);
            break;
        }
        case MI_SHOW_SPACES: {
            info.m_bShowSpace=!info.m_bShowSpace;
            config.SetConfig(info);
            break;
        }

        case MI_SPACES_TO_TABS:
        case MI_TABS_TO_SPACES: {
            bool no_selection=(Editor_GetSelType(hwndView)==SEL_TYPE_NONE);
            POINT_PTR pos;

            // (Un)tabify only works on a selection, so if we do not have one,
            // select all and work on the whole document.
            if (no_selection) {
                Editor_GetCaretPos(hwndView,POS_VIEW,&pos);
                Editor_ExecCommand(hwndView,EEID_EDIT_SELECT_ALL);
            }

            Editor_ExecCommand(hwndView,(item==MI_SPACES_TO_TABS)?EEID_TABIFY:EEID_UNTABIFY);

            if (no_selection) {
                Editor_SetSelType(hwndView,SEL_TYPE_NONE);

                // Restore the caret position; it actually is correct, but the
                // caret is invisible if we do not set it here.
                Editor_SetCaretPos(hwndView,POS_VIEW,&pos);
            }

            break;
        }
        case MI_TRIM_WHITESPACES: {
            Editor_ReplaceW(hwndView,FLAG_REPLACE_ALL|FLAG_FIND_NO_PROMPT|FLAG_FIND_REG_EXP,_T("(\\t|\\s)+$\0"));
            break;
        }
    }
}

// Queries the status of the plug-in, whether the command is enabled and
// whether the plug-in is a checked status.
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
