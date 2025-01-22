import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import '../utils/snackbar.dart';
import '../utils/extra.dart';

class EvsePilotScreen extends StatefulWidget {
  final BluetoothDevice device;

  const EvsePilotScreen({Key? key, required this.device}) : super(key: key);

  @override
  State<EvsePilotScreen> createState() => _EvsePilotScreenState();
}

class _EvsePilotScreenState extends State<EvsePilotScreen> {
  late BluetoothCharacteristic _characteristic;
  late Stream<List<int>> _valueStream;
  late StreamSubscription<List<int>> _subscription;

  @override
  void initState() {
    super.initState();
    _connectToDevice();
  }

  @override
  void dispose() {
    _subscription.cancel();
    super.dispose();
  }

  Future<void> _connectToDevice() async {
    await widget.device.connect();
    List<BluetoothService> services = await widget.device.discoverServices();
    for (BluetoothService service in services) {
      for (BluetoothCharacteristic characteristic in service.characteristics) {
        if (characteristic.properties.notify) {
          _characteristic = characteristic;
          _valueStream = _characteristic.value;
          _subscription = _valueStream.listen((value) {
            // Handle the value received from the characteristic
          });
          await _characteristic.setNotifyValue(true);
        }
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('EVSE Pilot Screen'),
      ),
      body: Center(
        child: Text('Connected to ${widget.device.name}'),
      ),
    );
  }
}
