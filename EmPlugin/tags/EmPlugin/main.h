#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <tchar.h>

#define ETL_FRAME_CLASS_NAME CMyFrame
#include "plugin/etlframe.h"

#include "resource.h"

class CMyFrame:public CETLFrame<CMyFrame>
{
  public:

    enum {
        _IDS_MENU   = IDS_MENU_TEXT       // Name of the command (main menu).
    ,   _IDS_STATUS = IDS_STATUS_MESSAGE  // Description of the command (status bar).
    ,   _IDS_NAME   = IDS_MENU_TEXT       // Name of the plug-in (settings dialog box).
    ,   _IDS_VER    = IDS_VERSION         // Version string of the plug-in (settings dialog box).
    };

    enum {
        // Large 24bit color (true color) bitmaps.
        _IDB_TRUE_24_DEFAULT = IDB_24_24_24      // Default state bitmap.
    ,   _IDB_TRUE_24_BW      = IDB_24_24_24_BW   // Disabled state bitmap.
    ,   _IDB_TRUE_24_HOT     = IDB_24_24_24_HOT  // Hot state bitmap.

        // Large 256 color bitmaps.
    ,   _IDB_256C_24_DEFAULT = IDB_24_24_8
    ,   _IDB_256C_24_BW      = IDB_24_24_8_BW
    ,   _IDB_256C_24_HOT     = IDB_24_24_8_HOT

        // Large 16 color bitmap (fallback).
    ,   _IDB_16C_24          = IDB_24_24_4

        // Small 24bit color (true color) bitmaps.
    ,   _IDB_TRUE_16_DEFAULT = IDB_16_16_24
    ,   _IDB_TRUE_16_BW      = IDB_16_16_24_BW
    ,   _IDB_TRUE_16_HOT     = IDB_16_16_24_HOT

        // Small 256 color bitmaps.
    ,   _IDB_256C_16_DEFAULT = IDB_16_16_8
    ,   _IDB_256C_16_BW      = IDB_16_16_8_BW
    ,   _IDB_256C_16_HOT     = IDB_16_16_8_HOT

        // Small 16 color bitmap (fallback).
    ,   _IDB_BITMAP          = IDB_16_16_4
    };

    enum {
        _ALLOW_OPEN_SAME_GROUP    = TRUE  // Is it allowed to open a file in the same window group during plug-in execution?
    ,   _ALLOW_MULTIPLE_INSTANCES = TRUE  // Is it allowed to have multiple instances?
    ,   _MAX_EE_VERSION           = 7100  // Newest supported EmEditor version * 1000.
    ,   _MIN_EE_VERSION           = 6100  // Oldest supported EmEditor version * 1000.
    ,   _SUPPORT_EE_PRO           = TRUE  // Is EmEditor Professional supported?
    ,   _SUPPORT_EE_STD           = TRUE  // Is EmEditor Standard supported?
    };

    // The mask color in the bitmap will be replaced with the background color of the toolbar.
    enum {
        _MASK_TRUE_COLOR = RGB(0,255,0)
    ,   _MASK_256_COLOR  = RGB(0,255,0)
    };

    /*
     * Functions to Export
     */

    // The plug-in has been selected from a menu or a toolbar.
    void OnCommand(HWND hwndView) {
    }

    // Queries the status of the plug-in, whether the command is enabled and
    // whether the plug-in is a checked status.
    BOOL QueryStatus(HWND hwndView,LPBOOL pbChecked) {
        return FALSE;
    }

    // When a status is changed, this function is called with the Events parameter.
    void OnEvents(HWND hwndView,UINT nEvent,LPARAM lParam) {
    }

    /*
     * Messages to Plug-ins
     */

    // Queries whether the plug-in can be uninstalled.
    BOOL QueryUninstall(HWND hDlg) {
        return TRUE;
    }

    // Uninstalls the plug-in.
    BOOL SetUninstall(HWND hDlg,LPTSTR pszUninstallCommand,LPTSTR pszUninstallParam) {
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
    BOOL QueryProperties(HWND hDlg) {
        return FALSE;
    }

    // Requests the plug-in to display the properties.
    BOOL SetProperties(HWND hDlg) {
        return FALSE;
    }

    // Called before each Windows message is translated.
    BOOL PreTranslateMessage(HWND hwndView,MSG* pMsg) {
        return FALSE;
    }
};

#endif // MAIN_H
