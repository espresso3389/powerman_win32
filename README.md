# powerman_win32

A plugin to listen to Win32 power events such as AC power state, display on/off, and suspend/resume.

## Getting Started

Fristly, install the plugin:

```
flutter pub add powerman_win32
```

Add the codes to listen to events:

```dart
StreamBuilder(
  stream: PowermanWin32.instance.systemPowerStatus,
  builder: (context, snapshot) => Text(snapshot.data?.toString() ?? '-'),
),
```
