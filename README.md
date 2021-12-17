# Project-8:
Deep-Learning Based Sensor Fusion for Mixed Reality systems

Mixed reality systems need to track users in a real-world to keep the illusion of “reality”. Multi-modal sensory data acquired from a MR headset (such as Hololens 2) is used for tracking the user in its MR world. Primarily, two kinds of data - inertial and visual - serve as data sources for tracking. Sensor fusion techniques stitch these data sources from the 9D inertial data and cameras (visual data) to track the user and provide spatial services. Primarily, two kinds of sensor data – inertial and visual- serve as data sources for tracking. Sensor fusion techniques ( using algorithms such as VINet and Kalman filter) are able to stich data generated from these sources and track the user effectively.

Sensor data is gathered from the Hololens 2 by building and deploying the StreamRecorder App and the SensorVisualization App in Visual Studio 2019 .
We then connect the Hololens 2 to our laptop via WiFi , enabling the Research Mode and deploy the applications by selecting Start Debug option in Visual Studio. 
After the sensor data is collected , the data is retrived by the LocalState cache file in the Windows Device Portal that we set up when we first connected the Hololens to the laptop.

The data is then fed into the EKF algorithm and the DNN model where sensor fusion and training of data takes place.

The EKF algorithm is modelled in sensor.ipynb and the DNN model is defined in tensor.ipynb files. The setup for extracting data from the HoloLens 2 headset and the extracted data has been provided in the Hololens data.md file.

The link to the poster is contained in Poster Demo.md file.



