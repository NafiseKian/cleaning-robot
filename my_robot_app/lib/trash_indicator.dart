import 'package:flutter/material.dart';

class TrashIndicator extends StatefulWidget {
  final double trashLevel;

  const TrashIndicator({Key? key, required this.trashLevel}) : super(key: key);

  @override
  _TrashIndicatorState createState() => _TrashIndicatorState();
}

class _TrashIndicatorState extends State<TrashIndicator> {
  OverlayEntry? overlayEntry;

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _checkAndNotifyTrashLevel();
    });
  }

  @override
  void dispose() {
    overlayEntry?.remove();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    Color trashColor = _getTrashColor(widget.trashLevel);

    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(Icons.delete, color: trashColor),
        SizedBox(width: 6),
        _buildTrashLevelIndicator(trashColor),
        SizedBox(width: 6),
        Text('${widget.trashLevel.toStringAsFixed(0)}%'),
      ],
    );
  }

  Color _getTrashColor(double level) {
    if (level <= 25) {
      return Colors.green;
    } else if (level <= 70) {
      return Colors.yellow;
    } else {
      return Colors.red;
    }
  }

  Widget _buildTrashLevelIndicator(Color color) {
    return Stack(
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
          width: 100 * (widget.trashLevel / 100),
          height: 20,
          decoration: BoxDecoration(
            color: color,
            borderRadius: BorderRadius.circular(4),
          ),
        ),
      ],
    );
  }

  void _checkAndNotifyTrashLevel() {
    if (widget.trashLevel >= 90 && widget.trashLevel <= 100) {
      _showCenteredDialog();
    }
  }

  void _showCenteredDialog() {
    overlayEntry = OverlayEntry(
      builder: (context) => DialogOverlay(
        content: 'Trash level is Full. Time to empty the trash!',
        trashLevel: widget.trashLevel,
        onDismiss: () {
          overlayEntry?.remove();
          overlayEntry = null;
        },
      ),
    );

    Overlay.of(context)?.insert(overlayEntry!);
  }
}

class DialogOverlay extends StatelessWidget {
  final String content;
  final double trashLevel;
  final VoidCallback onDismiss;

  const DialogOverlay(
      {Key? key,
      required this.content,
      required this.trashLevel,
      required this.onDismiss})
      : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(32.0),
        child: Material(
          elevation: 16.0,
          borderRadius: BorderRadius.circular(16),
          child: Container(
            decoration: BoxDecoration(
              color: Colors.white,
              borderRadius: BorderRadius.circular(16),
            ),
            padding: EdgeInsets.all(16),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                Text(
                  content,
                  style: TextStyle(
                    fontSize: 18,
                  ),
                ),
                SizedBox(height: 20),
                Text(
                  '${trashLevel.toStringAsFixed(0)}%', // Showing the percentage
                  style: TextStyle(
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                    color: Colors.red,
                  ),
                ),
                SizedBox(height: 20),
                ElevatedButton(
                  onPressed: onDismiss,
                  child: Text('Dismiss'),
                  style: ElevatedButton.styleFrom(
                    primary: Colors.red,
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8),
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
