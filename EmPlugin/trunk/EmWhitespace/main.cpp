#include "main.h"
#include "config.h"

#include <assert.h>
#include <process.h>

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(*x))

// The following line is needed after EmPlugin definition.
_ETL_IMPLEMENT

unsigned int EmPlugin::AnalyzeLineEnds(void* _this)
{
    // Note that m_eol_stats may point to a different location if the thread was
    // suspended and resumed, so do not use a copy but a reference here.
    LineEndStats*& stats=static_cast<EmPlugin*>(_this)->m_eol_stats;

    // Get the total number of lines in the document.
    stats->total_lines=Editor_GetLines(stats->view_handle,POS_VIEW);

    // As we want to analyze the line ends, get the raw line.
    GET_LINE_INFO info;
    info.flags=FLAG_WITH_CRLF;

    // Always allocate at least MAX_PATH chars.
    UINT_PTR size=MAX_PATH;
    LPTSTR buffer=(LPTSTR)malloc(size*sizeof(TCHAR));

    UINT_PTR const MAX_LINE_STEP=5000;

    while (stats->curr_line < stats->total_lines) {
        // Continuously output some progress.
        if (stats->curr_line%MAX_LINE_STEP==0) {
            ShowLineEndStatus(stats);
        }

        // Get the number of chars in the current line.
        info.cch=0;
        info.yLine=stats->curr_line;
        UINT_PTR chars=Editor_GetLineW(stats->view_handle,&info,NULL);

        // Adjust the buffer size if required.
        if (size<chars) {
            // Make room for twice the number of chars to reduce
            // the number of possible future reallocations.
            size=2*chars;
            buffer=(LPTSTR)realloc(buffer,size*sizeof(TCHAR));
        }

        // Get the text of the current line.
        info.cch=size;
        Editor_GetLineW(stats->view_handle,&info,buffer);

        // Jump to the last char in the line and check the line
        // end style.
        LPTSTR pos=buffer+chars-sizeof(TCHAR);
        if (*pos==_T('\n')) {
            if (pos>buffer && *(pos-1)==_T('\r')) {
                ++stats->dos_count;
            }
            else {
                ++stats->unix_count;
            }
        }
        else if (*pos==_T('\r')) {
            ++stats->mac_count;
        }

        ++stats->curr_line;
    }

    free(buffer);

    // Mark this thread as having finished.
    stats->thread_addr=NULL;

    return 0;
}

void EmPlugin::ShowLineEndStatus(LineEndStats const* stats)
{
    static LPCTSTR title=_T("Line End Style: ");

    if (!stats) {
        return;
    }

    TCHAR status[1024],prefix[512],suffix[512];

    // Get the current status text.
    Editor_GetStatusW(stats->view_handle,status,ARRAY_LENGTH(status));

    ZeroMemory(prefix,ARRAY_LENGTH(prefix));
    ZeroMemory(suffix,ARRAY_LENGTH(suffix));

    // Copy everything before the start of the line end status text.
    TCHAR* start=_tcsstr(status,title);
    if (start) {
        _tcsncpy_s(prefix,status,start-status);

        // Copy everything after the end of the line end status text.
        TCHAR* end=_tcschr(start,'.');
        if (end) {
            ++end;
            _tcsncpy_s(suffix,end,_tcslen(end));
        }
    }
    else {
        // As there is no line end status text, there also is
        // no prefix or suffix, so copy the whole string.
        _tcsncpy_s(prefix,status,_tcslen(status));
        _tcscat_s(prefix,_T(" "));
    }

#ifdef NDEBUG
    if (stats->curr_line < stats->total_lines) {
        _stprintf_s(
            status,
            _T("%s%s%s (%d/%d).%s"),
            prefix,
            title,
            stats->getName(),
            stats->curr_line,
            stats->total_lines,
            suffix
        );
    }
    else {
        _stprintf_s(
            status,
            _T("%s%s%s.%s"),
            prefix,
            title,
            stats->getName(),
            suffix
        );
    }
#else
    _stprintf_s(
        status,
        _T("%s%s%s, D:%d, U:%d, M:%d (%d/%d).%s"),
        prefix,
        title,
        stats->getName(),
        stats->dos_count,
        stats->unix_count,
        stats->mac_count,
        stats->curr_line,
        stats->total_lines,
        suffix
    );
#endif

    // Set the new status line text.
    Editor_SetStatusW(stats->view_handle,status);
}

EmPlugin::EmPlugin()
:   m_eol_stats(NULL)
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
    DWORD result;
    TCHAR name[MAX_PATH];

    // If a file is loaded, EVENT_FILE_OPENED is sent before EVENT_DOC_SEL_CHANGED,
    // but when starting up with a new untitled document, only EVENT_DOC_SEL_CHANGED
    // is sent.
    switch (nEvent) {
        case EVENT_FILE_OPENED: {
            // Suspend any currently analyzing thread.
            if (m_eol_stats && m_eol_stats->thread_addr) {
                result=SuspendThread((HANDLE)m_eol_stats->thread_addr);
                assert(result==0);
            }

            // Create a new suspended thread for this document.
            uintptr_t thread_addr=_beginthreadex(NULL,0,AnalyzeLineEnds,this,CREATE_SUSPENDED,NULL);
            assert(thread_addr!=NULL);

            // Create an entry in the hash table for this document.
            Editor_DocInfo(hwndView,-1,EI_GET_FILE_NAMEW,(LPARAM)name);
            m_eol_stats_table[name]=LineEndStats(thread_addr,hwndView);

            // Point to the stats of the current document.
            m_eol_stats=&m_eol_stats_table[name];

            // Run the thread for the current document.
            result=ResumeThread((HANDLE)m_eol_stats->thread_addr);
            assert(result==1);

            break;
        }
        case EVENT_DOC_SEL_CHANGED: {
            if (m_eol_stats && m_eol_stats->thread_addr) {
                // Suspend any currently analyzing thread.
                result=SuspendThread((HANDLE)m_eol_stats->thread_addr);
                assert(result==0);
            }

            // Point to the stats of the current document.
            Editor_DocInfo(hwndView,-1,EI_GET_FILE_NAMEW,(LPARAM)name);
            m_eol_stats=&m_eol_stats_table[name];

            if (m_eol_stats->thread_addr) {
                // Resume the thread for the current document.
                result=ResumeThread((HANDLE)m_eol_stats->thread_addr);
                assert(result==1);
            }

            break;
        }
        case EVENT_IDLE: {
            // Leave if we do not stats to display yet.
            if (!m_eol_stats || m_eol_stats->isEmpty()) {
                break;
            }

            // Work-around for the last dummy line in a document which always
            // has DOS line-endings.
            if (!m_eol_stats->thread_addr && m_eol_stats->dos_count==1
            && ((m_eol_stats->unix_count>0 && m_eol_stats->mac_count==0) || (m_eol_stats->unix_count==0 && m_eol_stats->mac_count>0)))
            {
                m_eol_stats->dos_count=0;
            }

            ShowLineEndStatus(m_eol_stats);

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
