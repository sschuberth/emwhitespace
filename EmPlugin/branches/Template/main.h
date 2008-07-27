#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <tchar.h>

#define ETL_FRAME_CLASS_NAME EmPlugin
#include "plugin/etlframe.h"

#include "resource.h"

class EmPlugin:public CETLFrame<EmPlugin>
{
  public:

    enum {
        _IDS_MENU   = IDS_MENU_TEXT       // Name of the command (main menu).
    ,   _IDS_STATUS = IDS_STATUS_MESSAGE  // Description of the command (status bar).
    ,   _IDS_NAME   = IDS_MENU_TEXT       // Name of the plug-in (settings dialog box).
    ,   _IDS_VER    = IDS_VERSION         // Version string of the plug-in (settings dialog box).
    };

    enum {
        // Large 32-bit (true color) bitmaps.
        _IDB_TRUE_24_DEFAULT = IDB_24_24_32           // Default state bitmap.
    ,   _IDB_TRUE_24_BW      = IDB_24_24_32_DISABLED  // Disabled state bitmap.
    ,   _IDB_TRUE_24_HOT     = IDB_24_24_32_HOVER     // Hot state bitmap.

        // Large 8-bit (256 color) bitmaps.
    ,   _IDB_256C_24_DEFAULT = IDB_24_24_8
    ,   _IDB_256C_24_BW      = IDB_24_24_8_DISABLED
    ,   _IDB_256C_24_HOT     = IDB_24_24_8_HOVER

        // Large 4-bit (16 color) bitmap (fallback).
    ,   _IDB_16C_24          = IDB_24_24_4

        // Small 32-bit (true color) bitmaps.
    ,   _IDB_TRUE_16_DEFAULT = IDB_16_16_32
    ,   _IDB_TRUE_16_BW      = IDB_16_16_32_DISABLED
    ,   _IDB_TRUE_16_HOT     = IDB_16_16_32_HOVER

        // Small 8-bit (256 color) bitmaps.
    ,   _IDB_256C_16_DEFAULT = IDB_16_16_8
    ,   _IDB_256C_16_BW      = IDB_16_16_8_DISABLED
    ,   _IDB_256C_16_HOT     = IDB_16_16_8_HOVER

        // Small 4-bit (16 color) bitmap (fallback).
    ,   _IDB_BITMAP          = IDB_16_16_4
    };

    enum {
        _ALLOW_OPEN_SAME_GROUP    = TRUE  // Is it allowed to open a file in the same window group during plug-in execution?
    ,   _ALLOW_MULTIPLE_INSTANCES = TRUE  // Is it allowed to have multiple instances?
    ,   _MAX_EE_VERSION           = 7100  // Newest supported EmEditor version * 1000 (the plug-in was developed against EmEditor 7.01, leave some room for newer versions).
    ,   _MIN_EE_VERSION           = 6000  // Oldest supported EmEditor version * 1000 (the used EVENT_IDLE is supported since EmEditor 6.00).
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
    void OnCommand(HWND hwndView);

    // Queries the status of the plug-in, whether the command is enabled and
    // whether the plug-in is a checked status.
    BOOL QueryStatus(HWND hwndView,LPBOOL pbChecked);

    // When a status is changed, this function is called with the Events parameter.
    void OnEvents(HWND hwndView,UINT nEvent,LPARAM lParam);

    /*
     * Messages to Plug-ins
     */

    // Queries whether the plug-in can be uninstalled.
    BOOL QueryUninstall(HWND hDlg);

    // Uninstalls the plug-in.
    BOOL SetUninstall(HWND hDlg,LPTSTR pszUninstallCommand,LPTSTR pszUninstallParam);

    // Queries whether the property is enabled.
    BOOL QueryProperties(HWND hDlg);

    // Requests the plug-in to display the properties.
    BOOL SetProperties(HWND hDlg);

    // Called before each Windows message is translated.
    BOOL PreTranslateMessage(HWND hwndView,MSG* pMsg);
};

#endif // MAIN_H
