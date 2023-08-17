import 'package:flutter/services.dart';
import 'package:rxdart/rxdart.dart';

class PowermanWin32 {
  final _methodChannel = const MethodChannel('powerman_win32');
  final _displayStateSubject = BehaviorSubject<DisplayState>();
  final _suspendResumeSubject = BehaviorSubject<SuspendResume>();
  final _acLineStatusSubject = BehaviorSubject<ACLineStatus>();

  Stream<DisplayState> get displayState => _displayStateSubject.stream;
  Stream<SuspendResume> get suspendResume => _suspendResumeSubject.stream;
  Stream<ACLineStatus> get acLineStatus => _acLineStatusSubject.stream;

  PowermanWin32._() {
    _methodChannel.setMethodCallHandler(_methodHandler);
  }

  static PowermanWin32 instance = PowermanWin32._();

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
      if (call.arguments == 0) {
        _acLineStatusSubject.add(ACLineStatus.offline);
      } else if (call.arguments == 1) {
        _acLineStatusSubject.add(ACLineStatus.online);
      } else if (call.arguments == 255) {
        _acLineStatusSubject.add(ACLineStatus.unknown);
      }
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
