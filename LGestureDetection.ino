
#include <stdio.h>
#include <driverlib.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <stdlib.h>

/* LCD Screen libraries*/
#include <LCD_screen.h>
#include <LCD_screen_font.h>
#include <LCD_utilities.h>
#include <Screen_HX8353E.h>
#include <Terminal12e.h>
#include <Terminal6e.h>
#include <Terminal8e.h>
Screen_HX8353E myScreen;

volatile uint16_t counter = 0;
volatile uint16_t count = 0;
volatile uint8_t IO_button = 1;
volatile uint16_t resultsBuffer[2];
volatile uint16_t xVal[10];
volatile uint16_t yVal[10];
volatile uint16_t xAvg;
volatile uint16_t yAvg;
volatile uint16_t prevVX = 0;
volatile uint16_t prevVY = 0;
uint8_t f = 9;
volatile uint16_t prevX = 0;
volatile uint16_t prevY = 0;
volatile uint16_t xVelocity;
volatile uint16_t yVelocity;
volatile uint16_t xDisp;
volatile uint16_t yDisp;
char textXAvg[4];
char textYAvg[4];
volatile uint16_t minX = 6300;
volatile uint16_t minY = 6300;
volatile uint16_t maxX = 0;
volatile uint16_t maxY = 0;
volatile uint16_t xSum = 0;
volatile uint16_t ySum = 0;
volatile uint16_t xVSum = 0;
volatile uint16_t yVSum = 0;

volatile uint16_t current_state;
volatile uint16_t gestures = 0;

volatile uint8_t gestureDetector = 0;
volatile uint8_t flag = 0;






void drawAccelData(void);



// your network name also called SSID
char ssid[] = "Snnoopy";
// your network password
char password[] = "nytimes25";
// MQTTServer to use
char server[] = "io.adafruit.com";

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {

  // Type Cast Input Bytes to Char
  char* str = (char*)payload;

  /* Check the Second Character of the char* pointer
      str[1] == 'N' ---> IO_button = ON
      str[1] == 'F' ---> IO_button = OFF
  */

  if (str[1] == 'N') {
    IO_button = 0;
    Serial.println("ON");
  }
  else if (str[1] == 'F') {
    IO_button = 1;
    Serial.println("OFF");
  }

  /*  feed data */
  /* Implement logic to decide whether Xacc or Yacc is published here */

}

void LCD_init() {

  myScreen.begin();

  /* Let's make a title*/
  myScreen.gText(8, 5, "Acceleration Data");
  myScreen.gText(8, 35, "Velocity Data");
  myScreen.gText(8, 65, "Displacement Data");
  myScreen.gText(8, 95, "Gesture Detected");

}

void ADC_init() {
  /* Configures Pin 4.0, 4.2, and 6.1 ad ADC inputs */
  // ACC Z = P4.2
  // ACC Y = P4.0
  // ACC X = P6.1


  current_state = 0;

  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN2, GPIO_TERTIARY_MODULE_FUNCTION);
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN1, GPIO_TERTIARY_MODULE_FUNCTION);

  ADC14_registerInterrupt(ADC14_IRQHandler);

  /* Initializing ADC (ADCOSC/64/8) */
  ADC14_enableModule();
  ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64, ADC_DIVIDER_8, 0);

  /* Configuring ADC Memory (ADC_MEM0 - ADC_MEM2 (A11, A13, A14)  with no repeat)
     with internal 2.5v reference */
  ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM2, true);
  ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A14, ADC_NONDIFFERENTIAL_INPUTS);

  ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A13, ADC_NONDIFFERENTIAL_INPUTS);

  ADC14_configureConversionMemory(ADC_MEM2, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A11, ADC_NONDIFFERENTIAL_INPUTS);


  ADC14_setResolution(ADC_10BIT); //IMPORTANT -> This seemed to give me the least noisey values (8 bit res was too small though)

  /* Enabling the interrupt when a conversion on channel 2 (end of sequence)
      is complete and enabling conversions */
  ADC14_enableInterrupt(ADC_INT2);

  /* Enabling Interrupts */
  Interrupt_enableInterrupt(INT_ADC14);
  Interrupt_enableMaster();

  /* Setting up the sample timer to automatically step through the sequence
     convert.*/
  ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

  /* Triggering the start of the sample */
  ADC14_enableConversion();
  ADC14_toggleConversionTrigger();
}

void drawAccelData(void) {
  myScreen.gText(40, 15,  "X: " + String(xAvg));
  myScreen.gText(40, 25,  "Y: " + String(yAvg));
  myScreen.gText(40, 45,  "X: " + String(xVelocity)); 
  myScreen.gText(40, 55,  "Y: " + String(yVelocity));
  myScreen.gText(40, 75,  "X: " + String(xDisp));
  myScreen.gText(40, 85,  "Y: " + String(yDisp));
  myScreen.gText(40, 105,  "G: " + String(gestures));
  myScreen.gText(40, 115,  "S: " + String(current_state));

}

void ADC14_IRQHandler(void)
{
  uint64_t status;

  status = MAP_ADC14_getEnabledInterruptStatus();
  MAP_ADC14_clearInterruptFlag(status);

  counter++;
  
  
  /* ADC_MEM2 conversion completed */
  if (status & ADC_INT2 && counter < 30) 
  {
    
    
    
     
    /* Store ADC14 conversion results */
    resultsBuffer[0] = ADC14_getResult(ADC_MEM0); // X Accelerometer
    resultsBuffer[1] = ADC14_getResult(ADC_MEM1); // Y Accelerometer

    /* smoothing filter for Acc readings */
    xSum = 0;
    ySum = 0;
    xVSum = 0;
    yVSum = 0;
    


    for (uint16_t i = 9; i > 0; i--) {
      xVal[i] = xVal[i - 1];
    }
    xVal[0] = resultsBuffer[0];


    for (uint8_t l = 0; l < 10; l++) {
      xSum += xVal[l];
    }

    xAvg = xSum / 10;

    for (uint16_t j = 9; j > 0; j--) {
      yVal[j] = yVal[j - 1];
    }
    yVal[0] = resultsBuffer[1];


    for (uint8_t y = 0; y < 10; y++) {
      ySum += yVal[y];
    }

    yAvg = ySum / 10;




    /************************************************************************************/

    // Calculate Velocity Readings

    xVelocity = 10 * (xAvg + prevX) / (2 * f);
    yVelocity = 10 * (yAvg + prevY) / (2 * f);

    prevX = xAvg;
    prevY = yAvg;




    // Calculate Displacement Readings
    xDisp = 10 * (xVelocity + prevVX) / (2 * f);
    yDisp = 10 * (yVelocity + prevVY) / (2 * f);

    prevVX = 0;
    prevVY = 0;




    /* GESTURE DETECTION STATE MACHINE */
if (count < 20){
            count++;

            if (xDisp > maxX){
                maxX = xDisp;
            }

            if (xDisp < minX){
                minX = xDisp;
            }

            if (yDisp > maxY){
                maxY = yDisp;
            }

            if (yDisp < minY){
                minY = yDisp;
            }            
        }
        else{
            
            count = 0;
            
            if (current_state == 0){
              
              if (((yDisp - minY) > 4) && ((maxX - xDisp) < 4)){
                  current_state = 1; // Y-gesture detected
              }
              else {
                  current_state = 0; //incorrect movement
              }
            }

            
            else if (current_state == 1){
              
              if (((maxX - xDisp) > 4) && ((yDisp - minY) < 4)){
                  current_state = 2; // X-gesture detected
              }
              else {
                current_state = 0; //incorrect movement
              }
            }

            
            else if (current_state == 2){
              gestures++; // L gesture detected
              gestureDetector = 1;
              current_state = 0;
            }
            

            // Reset Max/Min storage values
            minX = 65535; // Max number for uint16
            maxX = 0;
            minY = 65535; // Max number for uint16
            maxY = 0;
        }

    // Draw accelerometer data on display
    drawAccelData();
    
    
    flag = 1;

  }



}


void setup() {

  WDT_A_hold(WDT_A_BASE);

  Serial.begin(115200);


  // Start Ethernet with the build in MAC Address
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid);
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nYou're connected to the network");

  /* Initialize LCD and make some titles */
  LCD_init();
  /* Initialize ADC */
  ADC_init();
}

void loop() {

  /* Everything for interacting with the MQTT server is in this loop */
  /* When publishing to MQTT, you should not be interfacing with the ADC */
if(counter >= 30){
    flag = 1;
}
if(flag == 1){
  if (counter >= 30) {
      if (!client.connected()) {
        Serial.println("Disconnected. Reconnecting....");

        if (!client.connect("novick", "julianovick", "aio_RZMS35k71ojAKdDwSSt5QuJ96ora")) {
          Serial.println("Connection failed");
        } else {
          Serial.println("Connection success");
          if (client.subscribe("julianovick/feeds/button1")) {
            Serial.println("Subscription successfull");

          }
        }
      }
      
    if (gestureDetector == 1) {

      if (client.publish("julianovick/feeds/gestureDetection", "0")) {


        Serial.println("Publish success gesture");
      } else {
        Serial.println("Publish failed");
      }

      delay(1000);

      if (client.publish("julianovick/feeds/gestureDetection", "1")) {


        Serial.println("Publish success gesture");
      } else {
        Serial.println("Publish failed");
      }
    gestureDetector = 0;

    }
    else {
      client.poll();
      if (IO_button == 1) {

        
      sprintf(textYAvg, "%d", yAvg); 
        if (client.publish("julianovick/feeds/yAxis", textYAvg)) { 


          Serial.println("Publish success Y axis");
        } else {
          Serial.println("Publish failed Y axis");
        }
        //delay(1500);
      }
      
      else {

        
    sprintf(textXAvg, "%d", xAvg); 
        if (client.publish("julianovick/feeds/xAxis", textXAvg)) { 


          Serial.println("Publish success X axis");
        } else {
          Serial.println("Publish failed X axis");
        }
      }
    
    }


    counter = 0;
    
    flag = 0;
    

  }
}

else {
  flag = 0;
  }



}
