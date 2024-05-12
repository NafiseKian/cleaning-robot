import 'package:flutter/material.dart';

import 'battery_indicator.dart';
import 'trash_indicator.dart';

class RobotMainPage extends StatefulWidget {
  @override
  State<StatefulWidget> createState() => _RobotMainPageState();
}

class _RobotMainPageState extends State<RobotMainPage> {
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
    return (robotLongitude - mapLeftLongitude) /
        (mapRightLongitude - mapLeftLongitude) *
        mapWidth;
  }

  double findyPosition(double x, double y) {
    return (mapTopLatitude - robotLatitude) /
        (mapTopLatitude - mapBottomLatitude) *
        mapHeight;
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
                        child: Image.asset('assets/uni_map.jpg',
                            width: mapWidth,
                            height: mapHeight,
                            fit: BoxFit.cover),
                      ),
                    ),
                  ),
                  Positioned(
                    left: findxPosition(40, 40),
                    top: findyPosition(40, 40),
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
                      BatteryIndicator(batteryLevel: 60.0),
                    ],
                  ),
                  Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      TrashIndicator(trashLevel: 77.0),
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

//TODO : add some functions to get the x y coordinates from robot and update the location
//TODO : add a functin to get charger data from robot and update the widget
//TODO : add a function to get the trash bin status from robot and update the widget
//hint : in all functions you need to call set state to update the state of the application in real time
}
