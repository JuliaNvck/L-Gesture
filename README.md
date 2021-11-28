# L-Gesture
IoT device: L-Gesture Detection <br>
Using Arduino boards, C language, and Adafruit IO MQTT Broker <br><br>
- Connects to WiFi, uses smoothing filter and reads accelerometer data <br>
- Displays acceleration, velocity, and displacement data, and the number of detected gestures on LCD screen <br>
- Uses gesture detection state machine to recognize only correct L-shape gesture <br>
- Once detected, publishes data to Adafruit and light (IO indicator) on web turns green <br>
- Polls client to check if IO button is set to read X-accelerometer or Y-accelerometer data; publishes data to graph on Adafruit
<br><br>
<header> <h2> Gesture Detection State Machine </h2> 
<img width="572" alt="Screen Shot 2021-11-26 at 10 46 51 AM" src="https://user-images.githubusercontent.com/94994105/143621064-7c5030f6-8a7c-4f42-91a3-d87bfcb933af.png">
  <h2> MQTT Gesture Detection </h2>
<img width="576" alt="Screen Shot 2021-11-26 at 10 47 10 AM" src="https://user-images.githubusercontent.com/94994105/143621069-dd95febb-fbac-4f89-a55e-840a5710063f.png">
<h2> Adafruit IO </h2>
<img width="569" alt="Screen Shot 2021-11-26 at 10 47 46 AM" src="https://user-images.githubusercontent.com/94994105/143621071-41b70f4d-5edf-4829-9eb4-2ab79a18d9a6.png">
<h2> Arduino Board </h2>
<img width="185" alt="Screen Shot 2021-11-26 at 10 48 12 AM" src="https://user-images.githubusercontent.com/94994105/143621079-82bc8cb6-6062-4c5c-8127-ba2c9aa847d7.png">
  </header>

