#ifndef CONFIG_H
#define CONFIG_H

#include <windows.h>
#include <tchar.h>

#include "plugin/plugin.h"

class EmConfig
{
  public:

    EmConfig(HWND view);
    ~EmConfig();

    bool GetConfig(CCustomizeInfo& info,LPCTSTR name=NULL);
    bool SetConfig(CCustomizeInfo const& info,LPCTSTR name=NULL);

  private:

    static LPCTSTR ROOT_KEY;

    HWND m_view;
    HKEY m_root_key;
};

#endif // CONFIG_H
