#include "powerman_win32_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

// for PowerRegisterSuspendResumeNotification/PowerUnregisterSuspendResumeNotification
#pragma comment(lib, "Powrprof.lib")

static flutter::EncodableMap getSystemPowerStatus()
{
  SYSTEM_POWER_STATUS sps;
  GetSystemPowerStatus(&sps);
  return flutter::EncodableMap{
      {flutter::EncodableValue("ACLineStatus"), flutter::EncodableValue(static_cast<int32_t>(sps.ACLineStatus))},
      {flutter::EncodableValue("BatteryFlag"), flutter::EncodableValue(static_cast<int32_t>(sps.BatteryFlag))},
      {flutter::EncodableValue("BatteryLifePercent"), flutter::EncodableValue(static_cast<int32_t>(sps.BatteryLifePercent))},
      {flutter::EncodableValue("SystemStatusFlag"), flutter::EncodableValue(static_cast<int32_t>(sps.SystemStatusFlag))},
      {flutter::EncodableValue("BatteryLifeTime"), flutter::EncodableValue(static_cast<int32_t>(sps.BatteryLifeTime))},
      {flutter::EncodableValue("BatteryFullLifeTime"), flutter::EncodableValue(static_cast<int32_t>(sps.BatteryFullLifeTime))},
  };
}

namespace powerman_win32
{

  // static
  void PowermanWin32Plugin::RegisterWithRegistrar(
      flutter::PluginRegistrarWindows *registrar)
  {
    auto plugin = std::make_unique<PowermanWin32Plugin>();
    plugin->channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "powerman_win32",
            &flutter::StandardMethodCodec::GetInstance());

    plugin->channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result)
        {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    plugin->registrar = registrar;
    plugin->proc_id = registrar->RegisterTopLevelWindowProcDelegate(
        [plugin_pointer = plugin.get()](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
        {
          return plugin_pointer->windowProc(hwnd, message, wparam, lparam);
        });

    registrar->AddPlugin(std::move(plugin));
  }

  std::optional<
      LRESULT>
  PowermanWin32Plugin::windowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
  {
    ensureInit(hwnd);

    switch (message)
    {
    case WM_DESTROY:
      onDestroy();
      break;
    case WM_POWERBROADCAST:
      switch (wparam)
      {
      case PBT_APMPOWERSTATUSCHANGE:
        notifyPowerStatusChange();
        break;
      case PBT_POWERSETTINGCHANGE:
        notifyPowerSettingChange(reinterpret_cast<POWERBROADCAST_SETTING *>(lparam));
        break;
      }
    }
    return std::nullopt;
  }

  void PowermanWin32Plugin::HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
  {
    if (method_call.method_name().compare("getSystemPowerStatus") == 0)
    {
      result->Success(flutter::EncodableValue(getSystemPowerStatus()));
    }
    else
    {
      result->NotImplemented();
    }
  }

  void PowermanWin32Plugin::ensureInit(HWND hwnd)
  {
    if (!initialized)
    {
      initialized = true;
      subscribeToPowerEvents(hwnd);
      notifyPowerStatusChange();
    }
  }

  void PowermanWin32Plugin::onDestroy()
  {
    unsubscribePowerEvents();
    registrar->UnregisterTopLevelWindowProcDelegate(proc_id);
  }

  void PowermanWin32Plugin::subscribeToPowerEvents(HWND hwnd)
  {
    if (!powerNotifications.empty())
      return;

    static const LPCGUID psGuids[] = {
        &GUID_CONSOLE_DISPLAY_STATE,
    };
    for (auto &g : psGuids)
    {
      powerNotifications.push_back(RegisterPowerSettingNotification(hwnd, g, 0));
    }

    DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS param = {deviceNotifyCallbackRoutine, this};
    PowerRegisterSuspendResumeNotification(DEVICE_NOTIFY_CALLBACK, &param, &hPowerNotify);
  }

  void PowermanWin32Plugin::unsubscribePowerEvents()
  {
    PowerUnregisterSuspendResumeNotification(hPowerNotify);
    hPowerNotify = nullptr;

    for (auto &h : powerNotifications)
    {
      UnregisterPowerSettingNotification(h);
    }
  }

  void PowermanWin32Plugin::notifyPowerStatusChange()
  {
    channel->InvokeMethod("win32PowerStatusChange", std::make_unique<flutter::EncodableValue>(getSystemPowerStatus()));
  }

  void PowermanWin32Plugin::notifyPowerSettingChange(const POWERBROADCAST_SETTING *setting)
  {
    if (IsEqualGUID(setting->PowerSetting, GUID_CONSOLE_DISPLAY_STATE) && setting->DataLength >= sizeof(DWORD))
    {
      // https://learn.microsoft.com/en-us/windows/win32/power/power-setting-guids
      // 0x0 - The display is off.
      // 0x1 - The display is on.
      // 0x2 - The display is dimmed.
      const auto displayState = *reinterpret_cast<const int32_t *>(setting->Data);
      channel->InvokeMethod("win32ConsoleDisplayState", std::make_unique<flutter::EncodableValue>(displayState));
    }
  }

  ULONG PowermanWin32Plugin::deviceNotifyCallbackRoutine(PVOID Context, ULONG Type, PVOID Setting)
  {
    return reinterpret_cast<PowermanWin32Plugin *>(Context)->deviceNotifyCallback(Type, Setting);
  }

  ULONG PowermanWin32Plugin::deviceNotifyCallback(ULONG Type, PVOID Setting)
  {
    // https://learn.microsoft.com/en-us/windows/win32/api/powerbase/nf-powerbase-powerregistersuspendresumenotification
    // PBT_APMSUSPEND=0x4
    // PBT_APMRESUMESUSPEND=7
    // PBT_APMRESUMEAUTOMATIC=18
    channel->InvokeMethod("win32DeviceNotifyCallback", std::make_unique<flutter::EncodableValue>(static_cast<int32_t>(Type)));
    return 0;
  }
} // namespace powerman_win32
