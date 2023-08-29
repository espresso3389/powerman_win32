#include "include/powerman_win32/powerman_win32_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "powerman_win32_plugin.h"

void PowermanWin32PluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar)
{
    powerman_win32::PowermanWin32Plugin::RegisterWithRegistrar(
        flutter::PluginRegistrarManager::GetInstance()
            ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
