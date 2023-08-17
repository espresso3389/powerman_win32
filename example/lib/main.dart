import 'dart:convert';

import 'package:flutter/material.dart';

import 'package:powerman_win32/powerman_win32.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Win32 Powerman Example'),
        ),
        body: Column(
          children: [
            StreamBuilder(
                stream: PowermanWin32.instance.acLineStatus,
                builder: (context, snapshot) =>
                    Text(snapshot.data?.toString() ?? '-')),
            StreamBuilder(
                stream: PowermanWin32.instance.suspendResume,
                builder: (context, snapshot) =>
                    Text(snapshot.data?.toString() ?? '-')),
            StreamBuilder(
                stream: PowermanWin32.instance.displayState,
                builder: (context, snapshot) =>
                    Text(snapshot.data?.toString() ?? '-')),
          ],
        ),
      ),
    );
  }
}
