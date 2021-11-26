# L-Gesture
IoT device: L-Gesture Detection <br>
Using Arduino boards, C language, and Adafruit IO Broker <br><br>
- Connects to WiFi, reads velocity and acceleration data via accelerometer and uses smoothing filter <br>
- Uses gesture detection state machine to recognize only correct L-shape gesture <br>
- Once detected, publishes data to Adafruit and light on web turns green <br>
- Polls client to check if IO button is set to read X-axis or Y-axis data; publishes data to graph on Adafruit
