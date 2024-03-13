import 'package:flutter/material.dart';

class TrashIndicator extends StatelessWidget {
  final double trashLevel;

  const TrashIndicator({Key? key, required this.trashLevel}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    Color trashColor;
    if (trashLevel <= 25) {
      trashColor = Colors.green;
    } else if (trashLevel <= 70) {
      trashColor = Colors.yellow;
    } else {
      trashColor = Colors.red;
    }

    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(Icons.delete, color: trashColor, size: 24),
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
              width: 100 * (trashLevel / 100),
              height: 20,
              decoration: BoxDecoration(
                color: trashColor,
                borderRadius: BorderRadius.circular(4),
              ),
            ),
          ],
        ),
        SizedBox(width: 6),
        Text('$trashLevel'),
      ],
    );
  }
}
