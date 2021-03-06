#include "config.h"

LPCTSTR EmConfig::ROOT_KEY=_T("Software\\EmSoft\\EmEditor v3\\Config");
LPCTSTR EmConfig::CUST_VALUE=_T("Cust");

EmConfig::EmConfig(HWND view_handle)
:   m_view_handle(view_handle)
{
    if (RegOpenKeyEx(HKEY_CURRENT_USER,ROOT_KEY,0,KEY_READ,&m_root_key)!=ERROR_SUCCESS) {
        m_root_key=NULL;
    }
}

EmConfig::~EmConfig()
{
    RegCloseKey(m_root_key);
}

bool EmConfig::GetConfig(CCustomizeInfo& info,LPCTSTR name)
{
    bool result=false;

    TCHAR buffer[MAX_CONFIG_NAME];
    if (!name) {
        // Get the name of the current configuration.
        Editor_GetConfigW(m_view_handle,buffer);
        name=buffer;
    }

    HKEY config_key;
    if (RegOpenKeyEx(m_root_key,name,0,KEY_READ,&config_key)==ERROR_SUCCESS) {
        DWORD type,count;
        if (RegQueryValueEx(config_key,CUST_VALUE,NULL,&type,NULL,&count)==ERROR_SUCCESS) {
            if (type==REG_BINARY && count==sizeof(CCustomizeInfo)) {
                result=(RegQueryValueEx(config_key,CUST_VALUE,NULL,NULL,(LPBYTE)&info,&count)==ERROR_SUCCESS);
            }
        }
        RegCloseKey(config_key);
    }

    return result;
}

bool EmConfig::SetConfig(CCustomizeInfo const& info,LPCTSTR name)
{
    bool result=false;

    TCHAR buffer[MAX_CONFIG_NAME];
    if (!name) {
        // Get the name of the current configuration.
        Editor_GetConfigW(m_view_handle,buffer);
        name=buffer;
    }

    HKEY config_key;
    if (RegOpenKeyEx(m_root_key,name,0,KEY_WRITE,&config_key)==ERROR_SUCCESS) {
        result=(RegSetValueEx(config_key,CUST_VALUE,0,REG_BINARY,(LPBYTE)&info,sizeof(CCustomizeInfo))==ERROR_SUCCESS);
        RegCloseKey(config_key);
    }

    if (result) {
        Editor_LoadConfigW(m_view_handle,name);
    }

    return result;
}
