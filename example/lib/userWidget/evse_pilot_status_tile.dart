import 'dart:async';
import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_blue_plus_example/widgets/descriptor_tile.dart';

import "../utils/snackbar.dart";

import 'evse_pilot_status_tile.dart';

import 'dart:convert';

final pilot_service_uuid = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
final cp_duty_uuid = "bcaf4dee-e9b1-4f61-8bfa-340554dc63fa";
final cp_voltage_uuid = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
final cp_status_uuid = "ee17c408-d77e-4a12-9357-fd4e26206d8f";
final pp_voltage_uuid = "6e99e63f-55a5-4a1e-9788-ed2c44101750";
final pp_status_uuid = "1f3ef3d7-18be-44e5-82f7-ff8d498f4e4b";

class EvsePilotStatusTile extends StatefulWidget {
  final BluetoothService service;

  //const EvsePilotStatusTile({Key? key, required this.service}) : super(key: key);
  const EvsePilotStatusTile({Key? key, required this.service}) : super(key: key);
  @override
  _EvsePilotStatusTileState createState() => _EvsePilotStatusTileState();
}

class _EvsePilotStatusTileState extends State<EvsePilotStatusTile> {
  late BluetoothCharacteristic cpDutyCharacteristic;
  late BluetoothCharacteristic cpVoltageCharacteristic;
  late BluetoothCharacteristic cpStatusCharacteristic;
  late BluetoothCharacteristic ppVoltageCharacteristic;
  late BluetoothCharacteristic ppStatusCharacteristic;

  String cpDutySetValue = 'Enter 0~100';

  String _cpDuty = 'TBD';

  String get cpDuty => _cpDuty;

  set cpDuty(String value) {
    _cpDuty = value;
  }
  String _cpVoltage = 'TBD';

  String get cpVoltage => _cpVoltage;

  set cpVoltage(String value) {
    _cpVoltage = value;
  }
  String _cpStatus = 'TBD';

  String get cpStatus => _cpStatus;

  set cpStatus(String value) {
    _cpStatus = value;
  }
  String _ppVoltage = 'TBD';

  String get ppVoltage => _ppVoltage;

  set ppVoltage(String value) {
    _ppVoltage = value;
  }
  String ppStatus = 'TBD';

  Future sendCpDuty() async {
    if (cpDutySetValue.isEmpty) {
      return;
    }
    try {
      await cpDutyCharacteristic.write(utf8.encode(cpDutySetValue));
      Snackbar.show(ABC.c, "Write: Success", success: true);
    } catch (e) {
      Snackbar.show(ABC.c, prettyException("Write Error:", e), success: false);
      print(e);
    }
  }

  @override
  void initState() {
    super.initState();
    // Add your initialization code here
    for (BluetoothCharacteristic c in widget.service.characteristics) {
      if (c.uuid.toString() == cp_duty_uuid) {
        cpDutyCharacteristic = c;
        c.read().then((value) {
          setState(() {
            cpDuty = value.toString();
          });
        });
      }
      if (c.uuid.toString() == cp_voltage_uuid) {
        cpVoltageCharacteristic = c;
        c.read().then((value) {
          setState(() {
            cpVoltage = value.toString();
          });
        });
      }
      if (c.uuid.toString() == cp_status_uuid) {
        cpStatusCharacteristic = c;
        c.read().then((value) {
          setState(() {
            cpStatus = value.toString();
          });
        });
      }
      if (c.uuid.toString() == pp_voltage_uuid) {
        ppVoltageCharacteristic = c;
        c.read().then((value) {
          setState(() {
            ppVoltage = value.toString();
          });
        });
      }
      if (c.uuid.toString() == pp_status_uuid) {
        ppStatusCharacteristic = c;
        c.read().then((value) {
          setState(() {
            ppStatus = value.toString();
          });
        });
      }
    }
    Timer.periodic(Duration(milliseconds: 1000), (timer) async {
      for (BluetoothCharacteristic c in widget.service.characteristics) {
        if (c.uuid.toString() == cp_duty_uuid) {
          await c.read().then((value) {
            setState(() {
              cpDuty = String.fromCharCodes(value);
              //cpDuty = value.toString();
            });
          });
        }
        if (c.uuid.toString() == cp_voltage_uuid) {
            await c.read().then((value) {
            setState(() {
              cpVoltage = String.fromCharCodes(value);
              //cpVoltage = value.toString();
            });
            });
        }
        if (c.uuid.toString() == cp_status_uuid) {
            await c.read().then((value) {
            setState(() {
              cpStatus = String.fromCharCodes(value);
              //cpStatus = value.toString();
            });
            });
        }
        if (c.uuid.toString() == pp_voltage_uuid) {
            await c.read().then((value) {
            setState(() {
              ppVoltage = String.fromCharCodes(value);
              //ppVoltage = value.toString();
            });
            });
        }
        if (c.uuid.toString() == pp_status_uuid) {
            await c.read().then((value) {
            setState(() {
              ppStatus = String.fromCharCodes(value);
              //ppStatus = value.toString();
            });
            });
        }
      }
      setState(() {
        debugPrint('evse_pilot_status_tile');
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.only(left: 10, right: 10),
      child: Column(
        children: [
          SizedBox(height: 10),
          Text(
            'EVSE Pilot Status',
            textAlign: TextAlign.left,
            style: Theme.of(context).textTheme.titleLarge,
            ),
          SizedBox(height: 10),
          Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
                Text(
                'Set CP duty:',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
              Container(
                width: 150.0, // Set the desired width here
                child: TextField(
                  textAlign: TextAlign.right,
                  decoration: InputDecoration(
                  border: OutlineInputBorder(),
                  labelText: cpDutySetValue,
                  labelStyle: Theme.of(context).textTheme.bodyLarge,
                  ),
                  keyboardType: TextInputType.number,
                  onSubmitted: (value) {
                  setState(() {
                    cpDutySetValue = value;
                  });
                  },
                ),
                height: 40.0, // Set the desired height here
              ),
              IconButton(
                icon: Icon(Icons.send),
                onPressed: () {
                  // Add your onPressed code here
                  sendCpDuty();
                },
              ),
            ],
          ),
          SizedBox(height: 10),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
                Text(
                'CP Duty:',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
                Text(
                '${cpDuty} %',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
            ],
          ),
          SizedBox(height: 10),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
                Text(
                'CP Voltage:',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
                Text(
                '${cpVoltage} V',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
            ],
          ),
          SizedBox(height: 10),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
                Text(
                'CP Status:',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
                Text(
                '${cpStatus}',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
            ],
          ),
          SizedBox(height: 10),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
                Text(
                'PP Voltage:',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
                Text(
                '${ppVoltage} V',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
            ],
          ),
          SizedBox(height: 10),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
                Text(
                'PP Status:',
                style: Theme.of(context).textTheme.bodyLarge,
                ),
                Text(
                '${ppStatus}',
                style: Theme.of(context).textTheme.bodyLarge,
              ),
            ],
          ),
        ],
      ),
    );
  }
}