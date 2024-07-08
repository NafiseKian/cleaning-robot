import 'package:flutter/material.dart';

class EmergencyControlPanel extends StatefulWidget {
  @override
  _EmergencyControlPanelState createState() => _EmergencyControlPanelState();
}

class _EmergencyControlPanelState extends State<EmergencyControlPanel> {
  bool _isConnected = false; // Assuming initially not connected
  bool _isPowerOn = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Emergency Control Panel'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(20.0),
        child: Stack(
          children: [
            Align(
              alignment: Alignment.topLeft,
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Row(
                    children: [
                      Text(
                        'Power:',
                        style: TextStyle(fontSize: 18),
                      ),
                      SizedBox(width: 10),
                      Switch(
                        value: _isPowerOn,
                        onChanged: (bool value) {
                          setState(() {
                            _isPowerOn = value;
                          });
                          // Implement power switch action
                        },
                      ),
                    ],
                  ),
                  SizedBox(height: 10),
                  Row(
                    children: [
                      Container(
                        width: 20,
                        height: 20,
                        decoration: BoxDecoration(
                          shape: BoxShape.circle,
                          color: _isConnected ? Colors.green : Colors.red,
                        ),
                      ),
                      SizedBox(width: 10),
                      Text(
                        _isConnected ? 'Connected' : 'Disconnected',
                        style: TextStyle(
                          fontSize: 16,
                          color: _isConnected ? Colors.green : Colors.red,
                        ),
                      ),
                    ],
                  ),
                ],
              ),
            ),
            Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  // Arrow buttons
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                    children: [
                      ElevatedButton(
                        onPressed: () {
                          // Implement forward action
                        },
                        child: Icon(Icons.arrow_upward),
                      ),
                    ],
                  ),
                  SizedBox(height: 10),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      ElevatedButton(
                        onPressed: () {
                          // Implement left action
                        },
                        child: Icon(Icons.arrow_back),
                      ),
                      SizedBox(width: 20),
                      ElevatedButton(
                        onPressed: () {
                          // Implement right action
                        },
                        child: Icon(Icons.arrow_forward),
                      ),
                    ],
                  ),
                  SizedBox(height: 10),
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                    children: [
                      ElevatedButton(
                        onPressed: () {
                          // Implement backward action
                        },
                        child: Icon(Icons.arrow_downward),
                      ),
                    ],
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}
