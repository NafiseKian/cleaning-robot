import 'dart:convert';
import 'dart:io';
import 'package:flutter/material.dart';
import 'config.dart';
import 'battery_indicator.dart';
import 'trash_indicator.dart';

class RobotMainPage extends StatefulWidget {
  @override
  State<StatefulWidget> createState() => _RobotMainPageState();
}

class _RobotMainPageState extends State<RobotMainPage> {
  bool isUniMap = true;  //  state for the map
  bool _isConnected = false;  // Connection status

  @override
  void initState() {
    super.initState();

  }

  double mapTopLatitude = 40.0;
  double mapBottomLatitude = 40.1;
  double mapLeftLongitude = -74.0;
  double mapRightLongitude = -73.9;
  double robotLatitude = 40.05;
  double robotLongitude = -73.95;

  final double mapWidth = 600;
  final double mapHeight = 300;

  double findxPosition(double x, double y) {
    return (robotLongitude - mapLeftLongitude) / (mapRightLongitude - mapLeftLongitude) * mapWidth;
  }

  double findyPosition(double x, double y) {
    return (mapTopLatitude - robotLatitude) / (mapTopLatitude - mapBottomLatitude) * mapHeight;
  }

  void _toggleMap() {
    setState(() {
      isUniMap = !isUniMap;  
    });
  }

  Future<void> _sendHelloToServer() async {
    var host = ServerConfig.host;
    var port = ServerConfig.port;

    try {
      var socket = await Socket.connect(host, port);
      print('Connected to: ${socket.remoteAddress.address}:${socket.remotePort}');
      setState(() {
        _isConnected = true; 
      });

      socket.write('APP,');  

      socket.listen(
        (data) {
          print('Server response: ${utf8.decode(data)}');
        },
        onDone: () {
          print('Done with the server.');
          socket.destroy();
        },
        onError: (error) {
          print('Error: $error');
          socket.destroy();
        },
        cancelOnError: true,
      );
    } catch (e) {
      print('Failed to connect to the server: $e');
      setState(() {
        _isConnected = false;  // Set connection status as disconnected
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Robot Controller'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(8.0),
        child: Column(
          children: [
            Container(
              alignment: Alignment.topLeft,
              child: Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Switch(
                    value: isUniMap,
                    onChanged: (value) => _toggleMap(),
                    activeTrackColor: Colors.lightGreenAccent,
                    activeColor: Colors.green,
                  ),
                  Text('Indoor / Outdoor Map'),
                  SizedBox(width: 20),
                  Container(
                    width: 20,
                    height: 20,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      color: _isConnected ? Colors.green : Colors.red,
                    ),
                  ),
                  SizedBox(width: 8),
                  Text(
                    _isConnected ? 'Connected' : 'Disconnected',
                    style: TextStyle(
                      fontSize: 16,
                      color: _isConnected ? Colors.green : Colors.red,
                    ),
                  ),
                ],
              ),
            ),
            Expanded(
              flex: 3,
              child: Stack(
                children: [
                  Container(
                    decoration: BoxDecoration(
                      border: Border.all(color: Colors.black, width: 3),
                      borderRadius: BorderRadius.circular(12),
                    ),
                    child: ClipRRect(
                      borderRadius: BorderRadius.circular(9),
                      child: InteractiveViewer(
                        child: Image.asset(
                          isUniMap ? 'assets/uni_map.jpg' : 'assets/other_map.jpg',
                          width: mapWidth,
                          height: mapHeight,
                          fit: BoxFit.cover,
                        ),
                      ),
                    ),
                  ),
                  Positioned(
                    left: findxPosition(40, 40),
                    top: findyPosition(40, 40),
                    child: Icon(Icons.location_on, color: Colors.red, size: 24),
                  ),
                ],
              ),
            ),
            SizedBox(height: 20),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                BatteryIndicator(batteryLevel: 100.0),
                TrashIndicator(trashLevel: 7.0),
              ],
            ),
            SizedBox(height: 20),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                ElevatedButton(
                  onPressed: _sendHelloToServer,
                  child: Text('Locate The Robot'),
                ),
                ElevatedButton(
                  onPressed: () {
                    setState(() {});
                  },
                  child: Text('Return Home'),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}


//TODO : add some functions to get the x y coordinates from robot and update the location
//TODO : add a function to get charger data from robot and update the widget
//TODO : add a function to get the trash bin status from robot and update the widget
//hint : in all functions you need to call set state to update the state of the application in real time
