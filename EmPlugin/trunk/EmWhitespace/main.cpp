#include "main.h"
#include "config.h"

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(*x))

// The following line is needed after EmPlugin definition.
_ETL_IMPLEMENT

void EmPlugin::ShowLineEndStatus(HWND view,LPTSTR name)
{
    static LPCTSTR title=_T("Detected Line End Style: ");
    static TCHAR buffer[1024];

    Editor_GetStatusW(view,buffer,ARRAY_LENGTH(buffer));
    if (_tcslen(buffer)>0) {
        if (_tcsstr(buffer,title)) {
            return;
        }
        _tcscat_s(buffer,_T(" "));
    }

    _tcscat_s(buffer,title);
    _tcscat_s(buffer,name);

    Editor_SetStatusW(view,buffer);
}

EmPlugin::EmPlugin()
{
    ZeroMemory(&m_eol_stats,sizeof(m_eol_stats));

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
    POINT mouse_pos;
    if (!m_menu_handle || !GetCursorPos(&mouse_pos)) {
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
    UINT item=(UINT)TrackPopupMenuEx(m_menu_handle,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,mouse_pos.x,mouse_pos.y,hwndView,NULL);
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
            // Work on the selection if there is one, else on the whole document.
            UINT flags=FLAG_REPLACE_ALL|FLAG_FIND_NO_PROMPT|FLAG_FIND_REG_EXP;
            if (Editor_GetSelType(hwndView)!=SEL_TYPE_NONE) {
                flags|=FLAG_REPLACE_SEL_ONLY;
            }
            Editor_ReplaceW(hwndView,flags,_T("(\\t|\\s)+$\0"));

            // Disable highlighting of any matches.
            Editor_DocInfo(hwndView,-1,EI_SET_HILITE_FIND,0);

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
    switch (nEvent) {
        case EVENT_FILE_OPENED: {
            ZeroMemory(&m_eol_stats,sizeof(m_eol_stats));

            // Analyze at most 5000 lines to not take too much CPU power / time.
            const UINT_PTR MAX_LINES=5000;
            UINT_PTR lines=Editor_GetLines(hwndView,POS_VIEW);

            GET_LINE_INFO info;
            info.flags=FLAG_WITH_CRLF;

            // Always allocate at least MAX_PATH chars.
            LPTSTR buffer=(LPTSTR)malloc(MAX_PATH*sizeof(TCHAR));
            UINT_PTR size=MAX_PATH;

            for (UINT_PTR i=0;i<min(lines,MAX_LINES);++i) {
                // Adjust the required buffer size for a line of text.
                info.cch=0;
                info.yLine=i;
                UINT_PTR chars=Editor_GetLineW(hwndView,&info,NULL);
                if (size<chars) {
                    // Double the size of the buffer right away to reduce the
                    // number of possible future reallocations.
                    size=2*chars;
                    buffer=(LPTSTR)realloc(buffer,size*sizeof(TCHAR));
                }

                // Get the line of text.
                info.cch=size;
                Editor_GetLineW(hwndView,&info,buffer);
                LPTSTR pos=buffer+chars-sizeof(TCHAR);
                while (*pos!=_T('\n') && *pos!=_T('\r') && pos>buffer) {
                    --pos;
                }

                if (*pos==_T('\n')) {
                    if (pos>buffer && *(pos-1)==_T('\r')) {
                        ++m_eol_stats.dos_count;
                    }
                    else {
                        ++m_eol_stats.unix_count;
                    }
                }
                else if (*pos==_T('\r')) {
                    ++m_eol_stats.mac_count;
                }
            }

            // Add this file's stats to the hash table.
            Editor_DocInfo(hwndView,-1,EI_GET_FILE_NAMEW,(LPARAM)buffer);
            m_eol_stats_table[buffer]=m_eol_stats;

            free(buffer);

            // No break here!
        }
        case EVENT_DOC_SEL_CHANGED: {
            if (nEvent!=EVENT_FILE_OPENED) {
                TCHAR buffer[MAX_PATH];
                Editor_DocInfo(hwndView,-1,EI_GET_FILE_NAMEW,(LPARAM)buffer);
                m_eol_stats=m_eol_stats_table[buffer];
            }
            // No break here!
        }
        case EVENT_IDLE: {
            if (m_eol_stats.isEmpty()) {
                break;
            }

            // Work-around for returned dummy line, which has always DOS line-endings.
            if (m_eol_stats.dos_count==1
            && ((m_eol_stats.unix_count>0 && m_eol_stats.mac_count==0) || (m_eol_stats.unix_count==0 && m_eol_stats.mac_count>0)))
            {
                m_eol_stats.dos_count=0;
            }

            if (m_eol_stats.isDOSStyle()) {
                ShowLineEndStatus(hwndView,_T("DOS."));
            }
            else if (m_eol_stats.isUNIXStyle()) {
                ShowLineEndStatus(hwndView,_T("UNIX."));
            }
            else if (m_eol_stats.isMACStyle()) {
                ShowLineEndStatus(hwndView,_T("MAC."));
            }
            else {
                ShowLineEndStatus(hwndView,_T("Mixed."));
            }

            break;
        }
    }
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
