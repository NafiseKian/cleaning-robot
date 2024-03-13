import 'package:flutter/material.dart';
import 'battery_indicator.dart';
import 'trash_indicator.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Robot Controller',
      theme: ThemeData(
        primarySwatch: Colors.orange,
      ),
      home: RobotControlScreen(),
    );
  }
}

class RobotControlScreen extends StatelessWidget {
  // Example map bounds and robot location
  final double mapTopLatitude = 40.0;
  final double mapBottomLatitude = 40.1;
  final double mapLeftLongitude = -74.0;
  final double mapRightLongitude = -73.9;
  final double robotLatitude = 40.05;
  final double robotLongitude = -73.95;

  // These dimensions should match the displayed size of your map image in the app
  final double mapWidth = 600; // Adjust to your displayed map image's width
  final double mapHeight = 300; // Adjust to your displayed map image's height

  @override
  Widget build(BuildContext context) {
    // Calculate the robot's position on the map in pixels
    double xPosition = (robotLongitude - mapLeftLongitude) /
        (mapRightLongitude - mapLeftLongitude) *
        mapWidth;
    double yPosition = (mapTopLatitude - robotLatitude) /
        (mapTopLatitude - mapBottomLatitude) *
        mapHeight;
    return Scaffold(
      appBar: AppBar(
        title: Text('Robot Controller'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(8.0),
        child: Column(
          children: [
            Expanded(
              flex: 3,
              child: Stack(
                children: [
                  Container(
                    decoration: BoxDecoration(
                      border: Border.all(
                          color: Colors.black, width: 3), // Frame effect
                      borderRadius: BorderRadius.circular(12),
                    ),
                    child: ClipRRect(
                      borderRadius: BorderRadius.circular(9),
                      child: InteractiveViewer(
                        child: Image.asset('assets/uni_map.jpg',
                            width: mapWidth,
                            height: mapHeight,
                            fit: BoxFit.cover),
                      ),
                    ),
                  ),
                  Positioned(
                    left: xPosition,
                    top: yPosition,
                    child: Icon(Icons.location_on,
                        color: Colors.red, size: 24), // Robot marker
                  ),
                ],
              ),
            ),
            SizedBox(height: 20),
            Expanded(
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                children: [
                  Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      BatteryIndicator(batteryLevel: 90.0),
                    ],
                  ),
                  Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      TrashIndicator(trashLevel: 40.0),
                    ],
                  ),
                ],
              ),
            ),
            SizedBox(height: 20),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                ElevatedButton(
                  onPressed: () {
                    // Handle Start Cleaning action
                  },
                  child: Text('Start Cleaning'),
                ),
                ElevatedButton(
                  onPressed: () {
                    // Handle Return Home action
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
