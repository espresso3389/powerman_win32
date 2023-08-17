#ifndef FLUTTER_PLUGIN_POWERMAN_WIN32_PLUGIN_H_
#define FLUTTER_PLUGIN_POWERMAN_WIN32_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

#include <powrprof.h>

namespace powerman_win32
{

    class PowermanWin32Plugin : public flutter::Plugin
    {
    public:
        static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

        PowermanWin32Plugin() {}
        virtual ~PowermanWin32Plugin() {}

        // Disallow copy and assign.
        PowermanWin32Plugin(const PowermanWin32Plugin &) = delete;
        PowermanWin32Plugin &operator=(const PowermanWin32Plugin &) = delete;

        // Called when a method is called on this plugin's channel from Dart.
        void HandleMethodCall(
            const flutter::MethodCall<flutter::EncodableValue> &method_call,
            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

    private:
        std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel;
        flutter::PluginRegistrarWindows *registrar;
        int proc_id;
        std::vector<HPOWERNOTIFY> powerNotifications;
        bool initialized = false;
        HPOWERNOTIFY hPowerNotify = nullptr;

        std::optional<LRESULT> windowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

        void ensureInit(HWND hwnd);
        void onDestroy();

        void subscribeToPowerEvents(HWND hwnd);
        void unsubscribePowerEvents();
        void notifyPowerStatusChange();
        void notifyPowerSettingChange(const POWERBROADCAST_SETTING *setting);

        static ULONG deviceNotifyCallbackRoutine(PVOID Context, ULONG Type, PVOID Setting);
        ULONG deviceNotifyCallback(ULONG Type, PVOID Setting);
    };

} // namespace powerman_win32

#endif // FLUTTER_PLUGIN_POWERMAN_WIN32_PLUGIN_H_
