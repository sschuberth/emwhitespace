#include "main.h"
#include "config.h"

#include <assert.h>
#include <math.h>
#include <process.h>

// The following line is needed after EmPlugin definition.
_ETL_IMPLEMENT

EmPlugin::EmPlugin()
:   m_menu_handle(NULL)
,   m_show_whitespaces(true)
,   m_final_line_end(false)
{
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
    CCustomizeInfo config_info;
    if (!config.GetConfig(config_info)) {
        return;
    }

    CheckMenuItem(m_menu_handle,MI_SHOW_WHITESPACES,m_show_whitespaces?MF_CHECKED:MF_UNCHECKED);

    CheckMenuItem(m_menu_handle,MI_SHOW_LINE_ENDS,config_info.m_bShowCR?MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(m_menu_handle,MI_SHOW_EOF,config_info.m_bShowEOF?MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(m_menu_handle,MI_SHOW_TABS,config_info.m_bShowTab?MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(m_menu_handle,MI_SHOW_SPACES,config_info.m_bShowSpace?MF_CHECKED:MF_UNCHECKED);

    CheckMenuItem(m_menu_handle,MI_FINAL_LINE_END,m_final_line_end?MF_CHECKED:MF_UNCHECKED);

    // Display the pop-up menu and wait for a choice.
    UINT item=(UINT)TrackPopupMenuEx(m_menu_handle,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,mouse_pos.x,mouse_pos.y,hwndView,NULL);
    switch (item) {
        case MI_SHOW_WHITESPACES: {
            // Toggle the setting and immediately store it.
            m_show_whitespaces=!m_show_whitespaces;
            LONG result=Editor_RegSetValue(
                hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowWhitespaces"),REG_DWORD,(BYTE const*)&m_show_whitespaces,sizeof(m_show_whitespaces),0
            );
            assert(result==ERROR_SUCCESS);
            break;
        }

        case MI_SHOW_LINE_ENDS: {
            config_info.m_bShowCR=!config_info.m_bShowCR;
            config.SetConfig(config_info);
            break;
        }
        case MI_SHOW_EOF: {
            config_info.m_bShowEOF=!config_info.m_bShowEOF;
            config.SetConfig(config_info);
            break;
        }
        case MI_SHOW_TABS: {
            config_info.m_bShowTab=!config_info.m_bShowTab;
            config.SetConfig(config_info);
            break;
        }
        case MI_SHOW_SPACES: {
            config_info.m_bShowSpace=!config_info.m_bShowSpace;
            config.SetConfig(config_info);
            break;
        }

        case MI_FINAL_LINE_END: {
            // Toggle the setting and immediately store it.
            m_final_line_end=!m_final_line_end;
            LONG result=Editor_RegSetValue(
                hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("FinalLineEnd"),REG_DWORD,(BYTE const*)&m_final_line_end,sizeof(m_final_line_end),0
            );
            assert(result==ERROR_SUCCESS);
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

// Queries the status of the plug-in, whether the command is enabled and whether
// the plug-in is a checked status.
BOOL EmPlugin::QueryStatus(HWND hwndView,LPBOOL pbChecked)
{
    return TRUE;
}

// When a status is changed, this function is called with the Events parameter.
void EmPlugin::OnEvents(HWND hwndView,UINT nEvent,LPARAM lParam)
{
    if (!m_menu_handle) {
        // Create a pop-up menu with the desired entries.
        m_menu_handle=CreatePopupMenu();

        AppendMenu(m_menu_handle,MF_STRING,MI_SHOW_WHITESPACES,_T("Show Whitespaces"));

        // Separate the global toggle from the specific toggles.
        AppendMenu(m_menu_handle,MF_SEPARATOR,0,NULL);

        AppendMenu(m_menu_handle,MF_STRING,MI_SHOW_LINE_ENDS,_T("Show Line-Ends"));
        AppendMenu(m_menu_handle,MF_STRING,MI_SHOW_EOF,_T("Show End of File"));
        AppendMenu(m_menu_handle,MF_STRING,MI_SHOW_TABS,_T("Show Tabs"));
        AppendMenu(m_menu_handle,MF_STRING,MI_SHOW_SPACES,_T("Show Spaces"));

        if (Editor_GetVersion(hwndView)>=8000) {
            // Separate items available only since EmEditor 8.
            AppendMenu(m_menu_handle,MF_SEPARATOR,0,NULL);

            AppendMenu(m_menu_handle,MF_STRING,MI_FINAL_LINE_END,_T("Ensure final Line-End"));
        }

        // Separate toggle items from action items.
        AppendMenu(m_menu_handle,MF_SEPARATOR,0,NULL);

        AppendMenu(m_menu_handle,MF_STRING,MI_SPACES_TO_TABS,_T("Spaces to Tabs (Tabify)"));
        AppendMenu(m_menu_handle,MF_STRING,MI_TABS_TO_SPACES,_T("Tabs to Spaces (Untabify)"));
        AppendMenu(m_menu_handle,MF_STRING,MI_TRIM_WHITESPACES,_T("Trim Trailing Whitespaces"));

        // Get the initial values from the stored settings.
        DWORD size;
        LONG result;

        size=sizeof(m_show_whitespaces);
        result=Editor_RegQueryValue(
            hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowWhitespaces"),REG_DWORD,(BYTE*)&m_show_whitespaces,&size,0
        );

        size=sizeof(m_final_line_end);
        result=Editor_RegQueryValue(
            hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("FinalLineEnd"),REG_DWORD,(BYTE*)&m_final_line_end,&size,0
        );
    }

    if (nEvent==EVENT_SAVING && m_final_line_end) {
        bool finalize=true;

        // Get the total number of lines in the document.
        UINT_PTR lines=Editor_GetLines(hwndView,POS_VIEW);

        // As we want to analyze the line ends, get the raw line. Without
        // FLAG_LOGICAL, there is always a last dummy line with DOS line ends
        // returned.
        GET_LINE_INFO info;
        info.flags=FLAG_LOGICAL|FLAG_WITH_CRLF;

        // Get the number of chars in the current line, including line end and
        // the terminating \0 character.
        info.cch=0;
        info.yLine=lines-1;
        UINT_PTR chars=Editor_GetLineW(hwndView,&info,NULL);

        // Only do something if there is more than the terminating \0 character.
        if (chars>1) {
            LPTSTR buffer=(LPTSTR)malloc(chars*sizeof(TCHAR));
            if (buffer) {
                // Get the text of the current line.
                info.cch=chars;
                Editor_GetLineW(hwndView,&info,buffer);

                // Jump to the last char in the line and check the line end style.
                LPTSTR pos=buffer+chars-2;
                finalize=(*pos!=_T('\n') && *pos!=_T('\r'));
                free(buffer);
            }
        }

        if (finalize) {
            // Save a possible selection.
            POINT_PTR sel_start,sel_end;
            Editor_GetSelStart(hwndView,POS_VIEW,&sel_start);
            Editor_GetSelEnd(hwndView,POS_VIEW,&sel_end);
            UINT sel_type=Editor_GetSelType(hwndView);

            // Save the cursor position.
            POINT_PTR caret_old,caret_new={chars-1,lines-1};
            Editor_GetCaretPos(hwndView,POS_VIEW,&caret_old);

            // Insert a final line end.
            Editor_SetCaretPos(hwndView,POS_VIEW,&caret_new);
            Editor_InsertW(hwndView,_T("\n"),true);

            // Restore a possible selection.
            if (sel_start.x==caret_old.x && sel_start.y==caret_old.y) {
                Editor_SetSelView(hwndView,&sel_end,&sel_start);
            }
            else {
                Editor_SetSelView(hwndView,&sel_start,&sel_end);
            }
            Editor_SetSelTypeEx(hwndView,FALSE,sel_type);
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

// Queries whether the plug-in wants to use dropped files.
BOOL EmPlugin::UseDroppedFiles(HWND hwndView)
{
    return FALSE;
}

/*
 * Private helper methods
 */

void EmPlugin::SaveWhitespaceConfig(HWND hwndView,CCustomizeInfo const& config_info)
{
    LONG result;

    result=Editor_RegSetValue(
        hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowLineEnds"),REG_DWORD,(BYTE const*)&config_info.m_bShowCR,sizeof(config_info.m_bShowCR),0
    );
    assert(result==ERROR_SUCCESS);

    result=Editor_RegSetValue(
        hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowEOF"),REG_DWORD,(BYTE const*)&config_info.m_bShowEOF,sizeof(config_info.m_bShowEOF),0
    );
    assert(result==ERROR_SUCCESS);

    result=Editor_RegSetValue(
        hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowTabs"),REG_DWORD,(BYTE const*)&config_info.m_bShowTab,sizeof(config_info.m_bShowTab),0
    );
    assert(result==ERROR_SUCCESS);

    result=Editor_RegSetValue(
        hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowSpaces"),REG_DWORD,(BYTE const*)&config_info.m_bShowSpace,sizeof(config_info.m_bShowSpace),0
    );
    assert(result==ERROR_SUCCESS);
}

void EmPlugin::LoadWhitespaceConfig(HWND hwndView,CCustomizeInfo& config_info)
{
    bool toggle;
    DWORD size=sizeof(toggle);
    LONG result;

    result=Editor_RegQueryValue(
        hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowLineEnds"),REG_DWORD,(BYTE*)&toggle,&size,0
    );
    if (result==ERROR_SUCCESS) {
        config_info.m_bShowCR=toggle;
    }

    result=Editor_RegQueryValue(
        hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowEOF"),REG_DWORD,(BYTE*)&toggle,&size,0
    );
    if (result==ERROR_SUCCESS) {
        config_info.m_bShowEOF=toggle;
    }

    result=Editor_RegQueryValue(
        hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowTabs"),REG_DWORD,(BYTE*)&toggle,&size,0
    );
    if (result==ERROR_SUCCESS) {
        config_info.m_bShowTab=toggle;
    }

    result=Editor_RegQueryValue(
        hwndView,EEREG_EMEDITORPLUGIN,_T("Whitespace"),_T("ShowSpaces"),REG_DWORD,(BYTE*)&toggle,&size,0
    );
    if (result==ERROR_SUCCESS) {
        config_info.m_bShowSpace=toggle;
    }
}
