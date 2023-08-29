// ignore_for_file: public_member_api_docs, sort_constructors_first
import 'package:flutter/services.dart';
import 'package:rxdart/rxdart.dart';

class PowermanWin32 {
  final _methodChannel = const MethodChannel('powerman_win32');
  final _displayStateSubject = BehaviorSubject<DisplayState>();
  final _suspendResumeSubject = BehaviorSubject<SuspendResume>();
  final _systemPowerStatusSubject = BehaviorSubject<SystemPowerStatus>();

  Stream<DisplayState> get displayState => _displayStateSubject.stream;
  Stream<SuspendResume> get suspendResume => _suspendResumeSubject.stream;
  Stream<SystemPowerStatus> get systemPowerStatus => _systemPowerStatusSubject.stream;

  PowermanWin32._() {
    _methodChannel.setMethodCallHandler(_methodHandler);
  }

  static PowermanWin32 instance = PowermanWin32._();

  Future<SystemPowerStatus> getSystemPowerStatus() async {
    final map = await _methodChannel.invokeMethod('getSystemPowerStatus');
    final s = powerStatusFromMap(map);
    _systemPowerStatusSubject.add(s);
    return s;
  }

  static SystemPowerStatus powerStatusFromMap(dynamic map) {
    return SystemPowerStatus(
      acLineStatus: ACLineStatus.values[map['ACLineStatus'] as int],
      batteryFlag: BatteryFlag(map['BatteryFlag'] as int),
      batteryLifePercent: map['BatteryLifePercent'] as int,
      systemStatusFlag: SystemStatusFlag(map['SystemStatusFlag'] as int),
      batteryLifeTime: durFromSec(map['BatteryLifeTime'] as int),
      batteryFullLifeTime: durFromSec(map['BatteryFullLifeTime'] as int),
    );
  }

  static Duration? durFromSec(int seconds) {
    if (seconds < 0) return null;
    return Duration(seconds: seconds);
  }

  Future<dynamic> _methodHandler(MethodCall call) async {
    if (call.method == 'win32ConsoleDisplayState') {
      _displayStateSubject.add(DisplayState.values[call.arguments as int]);
      return 0;
    }
    if (call.method == 'win32DeviceNotifyCallback') {
      if (call.arguments == 4) {
        _suspendResumeSubject.add(SuspendResume.suspend);
      } else if (call.arguments == 18) {
        _suspendResumeSubject.add(SuspendResume.resume);
      }
      return 0;
    }
    if (call.method == 'win32PowerStatusChange') {
      _systemPowerStatusSubject.add(powerStatusFromMap(call.arguments));
    }
    return 0;
  }
}

enum DisplayState {
  off,
  on,
  dimmed,
}

enum SuspendResume {
  suspend,
  resume,
}

enum ACLineStatus {
  offline,
  online,
  unknown,
}

class SystemPowerStatus {
  final ACLineStatus acLineStatus;
  final BatteryFlag batteryFlag;
  final int batteryLifePercent;
  final SystemStatusFlag systemStatusFlag;
  final Duration? batteryLifeTime;
  final Duration? batteryFullLifeTime;

  SystemPowerStatus({
    required this.acLineStatus,
    required this.batteryFlag,
    required this.batteryLifePercent,
    required this.systemStatusFlag,
    required this.batteryLifeTime,
    required this.batteryFullLifeTime,
  });

  @override
  String toString() {
    return 'SystemPowerStatus(acLineStatus: $acLineStatus, batteryFlag: $batteryFlag, batteryLifePercent: $batteryLifePercent, systemStatusFlag: $systemStatusFlag, batteryLifeTime: $batteryLifeTime, batteryFullLifeTime: $batteryFullLifeTime)';
  }

  @override
  bool operator ==(covariant SystemPowerStatus other) {
    if (identical(this, other)) return true;

    return other.acLineStatus == acLineStatus &&
        other.batteryFlag == batteryFlag &&
        other.batteryLifePercent == batteryLifePercent &&
        other.systemStatusFlag == systemStatusFlag &&
        other.batteryLifeTime == batteryLifeTime &&
        other.batteryFullLifeTime == batteryFullLifeTime;
  }

  @override
  int get hashCode {
    return acLineStatus.hashCode ^
        batteryFlag.hashCode ^
        batteryLifePercent.hashCode ^
        systemStatusFlag.hashCode ^
        batteryLifeTime.hashCode ^
        batteryFullLifeTime.hashCode;
  }
}

class BatteryFlag {
  final int flag;
  const BatteryFlag(this.flag);

  // More than 66%
  bool get isHigh => (flag & 1) != 0;

  /// Lower than 33%
  bool get isLow => (flag & 2) != 0;

  /// Less than 5%
  bool get isCritical => (flag & 4) != 0;

  bool get isCharging => (flag & 8) != 0;

  bool get hasNoSystemBattery => (flag & 128) != 0;

  bool get isUnknownStatus => flag == 255;

  @override
  String toString() => 'BatteryFlag(flag: $flag)';

  @override
  bool operator ==(covariant BatteryFlag other) {
    if (identical(this, other)) return true;

    return other.flag == flag;
  }

  @override
  int get hashCode => flag.hashCode;
}

class SystemStatusFlag {
  final int flag;
  const SystemStatusFlag(this.flag);

  bool get isBatterySaverOn => flag == 1;

  @override
  String toString() => 'SystemStatusFlag(flag: $flag)';

  @override
  bool operator ==(covariant SystemStatusFlag other) {
    if (identical(this, other)) return true;

    return other.flag == flag;
  }

  @override
  int get hashCode => flag.hashCode;
}
