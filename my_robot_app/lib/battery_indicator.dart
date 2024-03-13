import 'package:flutter/material.dart';

class BatteryIndicator extends StatelessWidget {
  final double batteryLevel;

  const BatteryIndicator({Key? key, required this.batteryLevel})
      : super(key: key);

  @override
  Widget build(BuildContext context) {
    Color batteryColor;
    if (batteryLevel > 75) {
      batteryColor = Colors.green;
    } else if (batteryLevel > 20) {
      batteryColor = Colors.yellow;
    } else {
      batteryColor = Colors.red;
    }

    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(Icons.battery_std, color: batteryColor, size: 24),
        SizedBox(width: 6),
        Stack(
          children: [
            Container(
              width: 100,
              height: 20,
              decoration: BoxDecoration(
                color: Colors.grey.shade300,
                borderRadius: BorderRadius.circular(4),
              ),
            ),
            Container(
              width: 100 * (batteryLevel / 100),
              height: 20,
              decoration: BoxDecoration(
                color: batteryColor,
                borderRadius: BorderRadius.circular(4),
              ),
            ),
          ],
        ),
        SizedBox(width: 6),
        Text('$batteryLevel%'),
      ],
    );
  }
}
