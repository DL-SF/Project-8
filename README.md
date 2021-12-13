# Project-8:
Deep-Learning Based Sensor Fusion for Mixed Reality systems

Mixed reality systems need to track users in a real-world to keep the illusion of  “reality”. Multi-modal sensory data acquired from a MR headset (such as Hololens) is used for tracking the user in its MR world. Primarily, two kinds of data - inertial and visual - serve as data sources for tracking. Sensor fusion techniques stitch these data sources from the 9D inertial data and cameras (visual data) to track the user and provide spatial services.

In this project we are implementing sensor fusion algorithms - VINet - for fusing the sensor data gathered from the Microsoft Hololens 2.
Sensor data is gathered from the Hololens 2 by building and deploying the StreamRecorder App and the SensorVisualization App in Visual Studio 2019 .
We then connect the Hololens 2 to our laptop via WiFi , enabling the Research Mode and deploy the applications by selecting Start Debug option in Visual Studio. 
After the sensor data is collected , the data is retrived by the LocalState cache file in the Windows Device Portal that we set up when we first connected the Hololens to the laptop.
[Demo](https://drive.google.com/file/d/1KZcFBMJ_sk-gh0QxmzvOBc2gd8cCYhYI/view?usp=sharing)
