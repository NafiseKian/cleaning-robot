import 'package:flutter/material.dart';
import 'package:my_robot_app/MainPage.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: Text('Robot App'),
        ),
        drawer: Drawer(
          child: ListView(
            padding: EdgeInsets.zero,
            children: <Widget>[
              DrawerHeader(
                decoration: BoxDecoration(
                  color: Color.fromARGB(244, 248, 242, 240),
                ),
                child: Row(
                  children: [
                    Image.asset('assets/logo.png', width: 100),
                    SizedBox(width: 5),
                    Text(
                      'CIU ',
                      style: TextStyle(
                        color: Color.fromARGB(255, 111, 61, 17),
                        fontSize: 24,
                      ),
                    ),
                    Text(
                      'ROBOT',
                      style: TextStyle(
                        color: Color.fromARGB(255, 255, 1, 1),
                        fontSize: 24,
                      ),
                    ),
                  ],
                ),
              ),
              Container(
                color: Colors.red,
                child: ListTile(
                  title: Text(
                    'Emergency Controller',
                    style: TextStyle(
                      color: Colors.white,
                    ),
                  ),
                  onTap: () {
                    // Implement here
                    Navigator.pop(context);
                  },
                ),
              ),
              ListTile(
                title: Text('Locate Robot'),
                onTap: () {
                  Navigator.pop(context);
                },
              ),
            ],
          ),
        ),
        body: RobotMainPage(),
      ),
    );
  }
}
