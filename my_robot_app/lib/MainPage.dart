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
  bool isUniMap = true;  // State for the map.
  bool _isConnected = false;  // State to display connection status.
  double batteryLevel = 0.0;  // State for battery level.
  double trashLevel = 0.0;  // State for trash level.
  List<Map<String, dynamic>> predefinedMarkers = []; // Predefined markers with names and coordinates
  List<Map<String, double>> markerPositions = []; // State for marker positions.

  @override
  void initState() {
    super.initState();
    // Initialize predefined markers with names, x, y, left, and top
    predefinedMarkers = [
      {'name': 'Marker 1', 'x': 10.0, 'y': 10.0, 'left': 20.0, 'top': 10.0},
      {'name': 'Marker 2', 'x': 20.0, 'y': 20.0, 'left': 0.0, 'top': 0.0},
      {'name': 'Marker 3', 'x': 30.0, 'y': 30.0, 'left': 0.0, 'top': 0.0},
      {'name': 'Marker 4', 'x': 40.0, 'y': 40.0, 'left': 0.0, 'top': 0.0},
      // Add more predefined markers as needed
    ];
  }

  void _sendHelloToServer() async {
    var host = ServerConfig.host;
    var port = ServerConfig.port;

    try {
      var socket = await Socket.connect(host, port);
      print('Connected to: ${socket.remoteAddress.address}:${socket.remotePort}');
      setState(() {
        _isConnected = true; 
      });

      // Send a specific request to get status
      socket.write('APP,STATUS');

      socket.listen(
        (data) {
          var response = utf8.decode(data);
          print('Server response: $response');
          _updateIndicators(response);  
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
        _isConnected = false;
      });
    }
  }

  void _updateIndicators(String responseData) {
    print('Raw response data: $responseData');
    
    try {
      var decoded = jsonDecode(responseData);
      print('Decoded response data: $decoded');

      // Parse the coordinates from 'x' and 'y' directly
      double x = decoded.containsKey('x') ? double.parse(decoded['x'].toString()) : 0.0;
      double y = decoded.containsKey('y') ? double.parse(decoded['y'].toString()) : 0.0;
      print('Parsed coordinates: x=$x, y=$y');

      // Update the predefined markers with received x, y, left, and top values
      for (var marker in predefinedMarkers) {
        if (marker['x'] == x && marker['y'] == y) {
          // Calculate left and top positions based on map dimensions
          double mapWidth = 500; // Width of the map image
          double mapHeight = 250; // Height of the map image
          double coordMaxX = 90; // Maximum X value from the server
          double coordMaxY = 90; // Maximum Y value from the server

          marker['left'] = (x / coordMaxX) * mapWidth;
          marker['top'] = (y / coordMaxY) * mapHeight;
          print('Updated marker ${marker['name']} positions: left=${marker['left']}, top=${marker['top']}');
          break;
        }
      }

      setState(() {
        // Update markers
        markerPositions = List<Map<String, double>>.from(predefinedMarkers.map((marker) =>
            {'left': marker['left'], 'top': marker['top']}
        ));
      });
    } catch (e) {
      print('Error parsing response data: $e');
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
            SwitchAndConnectionStatus(isUniMap: isUniMap, isConnected: _isConnected, toggleMap: _toggleMap),
            Expanded(
              flex: 3,
              child: MapAndPosition(isUniMap: isUniMap, mapWidth: 600, mapHeight: 300, markerPositions: markerPositions),
            ),
            SizedBox(height: 20),
            BatteryAndTrash(batteryLevel: batteryLevel, trashLevel: trashLevel),
            ControlButtons(sendToServer: _sendHelloToServer),
          ],
        ),
      ),
    );
  }

  void _toggleMap() {
    setState(() {
      isUniMap = !isUniMap;
    });
  }
}

class SwitchAndConnectionStatus extends StatelessWidget {
  const SwitchAndConnectionStatus({
    Key? key,
    required this.isUniMap,
    required this.isConnected,
    required this.toggleMap,
  }) : super(key: key);

  final bool isUniMap;
  final bool isConnected;
  final Function toggleMap;

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Switch(
          value: isUniMap,
          onChanged: (value) => toggleMap(),
          activeTrackColor: Colors.lightGreenAccent,
          activeColor: Colors.green,
        ),
        Text('Indoor / Outdoor Map'),
        SizedBox(width: 20),
        Icon(
          isConnected ? Icons.signal_wifi_4_bar : Icons.signal_wifi_off,
          color: isConnected ? Colors.green : Colors.red,
        ),
        Text(isConnected ? 'Connected' : 'Disconnected'),
      ],
    );
  }
}

class MapAndPosition extends StatelessWidget {
  const MapAndPosition({
    Key? key,
    required this.isUniMap,
    required this.mapWidth,
    required this.mapHeight,
    required this.markerPositions,
  }) : super(key: key);

  final bool isUniMap;
  final double mapWidth;
  final double mapHeight;
  final List<Map<String, double>> markerPositions;

  @override
  Widget build(BuildContext context) {
    return Stack(
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
                isUniMap ? 'assets/st.jpeg' : 'assets/other_map.jpg',
                width: mapWidth,
                height: mapHeight,
                fit: BoxFit.cover,
              ),
            ),
          ),
        ),
        ...markerPositions.map((position) {
          return Positioned(
            left: position['left'],
            top: position['top'],
            child: Icon(Icons.location_on, color: Colors.red, size: 24),
          );
        }).toList(),
      ],
    );
  }
}

class BatteryAndTrash extends StatelessWidget {
  const BatteryAndTrash({
    Key? key,
    required this.batteryLevel,
    required this.trashLevel,
  }) : super(key: key);

  final double batteryLevel;
  final double trashLevel;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceAround,
      children: [
        BatteryIndicator(batteryLevel: batteryLevel),
        TrashIndicator(trashLevel: trashLevel),
      ],
    );
  }
}

class ControlButtons extends StatelessWidget {
  const ControlButtons({
    Key? key,
    required this.sendToServer,
  }) : super(key: key);

  final Function sendToServer;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceAround,
      children: [
        ElevatedButton(
          onPressed: () => sendToServer(),
          child: Text('Locate The Robot'),
        ),
        ElevatedButton(
          onPressed: () {},
          child: Text('Return Home'),
        ),
      ],
    );
  }
}
