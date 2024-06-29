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
  List<Map<String, double>> markerPositions = []; // State for marker positions.

  @override
  void initState() {
    super.initState();
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
    var decoded = jsonDecode(responseData);
    print('Decoded response data: $decoded');
    
    setState(() {
      batteryLevel = double.parse(decoded['battery'].toString());
      print('Updated battery level: $batteryLevel');
      
      trashLevel = double.parse(decoded['trash'].toString());
      print('Updated trash level: $trashLevel');
      
      var coordinates = decoded['coordinates'];
      print('Coordinates received: $coordinates');
      
      _updateMarkers(coordinates);
    });
  }

  void _updateMarkers(List<dynamic> coordinates) {
    List<Map<String, double>> newMarkerPositions = [];
    double maxX = 600; // Maximum width of the map image
    double maxY = 300; // Maximum height of the map image
    double scaleX = 4; // Scaling factor for X coordinate
    double scaleY = 4; // Scaling factor for Y coordinate

    for (var coord in coordinates) {
      // Assuming the coordinates are in the form [x, y]
      double x = double.parse(coord[0].toString());
      double y = double.parse(coord[1].toString());

      // Estimate the position on the map based on existing logic
      double estimatedLeft = x * scaleX; // Example transformation
      double estimatedTop = y * scaleY;  // Example transformation

      // Clamp the coordinates to be within the map boundaries
      estimatedLeft = estimatedLeft.clamp(0.0, maxX);
      estimatedTop = estimatedTop.clamp(0.0, maxY);

      newMarkerPositions.add({'left': estimatedLeft, 'top': estimatedTop});
    }

    setState(() {
      markerPositions = newMarkerPositions;
      print('Updated marker positions: $markerPositions');
    });
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
