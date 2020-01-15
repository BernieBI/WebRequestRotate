/*
  Web client

  This sketch connects to a website (http://www.google.com)
  using the WiFi module.

  This example is written for a network using WPA encryption. For
  WEP or WPA, change the Wifi.begin() call accordingly.

  This example is written for a network using WPA encryption. For
  WEP or WPA, change the Wifi.begin() call accordingly.

  Circuit:
   Board with NINA module (Arduino MKR WiFi 1010, MKR VIDOR 4000 and UNO WiFi Rev.2)

  created 13 July 2010
  by dlf (Metodo2 srl)
  modified 31 May 2012
  by Tom Igoe
*/


#include <SPI.h>
#include <WiFiNINA.h>
#include <String.h>
#include <Stepper.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "Dark Web";        // your network SSID (name)
char pass[] = "wifipassordet";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiClient client;

// server address:
char server[] = "matsiotstorage.blob.core.windows.net";

//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 500L; // delay between updates, in milliseconds

// Define number of steps per rotation:
const int stepsPerRevolution = 2048;
Stepper stepper = Stepper(stepsPerRevolution, 4, 5, 6, 7);


String prevState = "closed";


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  stepper.setSpeed(12);
  prevState = "closed";
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop() {

  String response;


  while (client.available()) {
    char c = client.read();
    response = String(response + c);
    Serial.write(c);
  }

  String delimiter = "\r\n\r\n";
  int startOfBody = response.indexOf(delimiter);
  String latestState = response.substring(startOfBody);
  latestState.trim();
  
  //Serial.println("States:");
  //Serial.println(String("newest: " + latestState));
  //Serial.println(String("oldest: " + prevState));
  
  if (latestState != prevState) {
    
    if (latestState == "open") {
      
      Serial.println("Opening");
      stepper.step(-stepsPerRevolution*10);
      prevState = latestState;
      
    } else if (latestState == "closed") {

      Serial.println("Closing");
      stepper.step(-stepsPerRevolution*10);
      prevState = latestState;
    }
  }
  
  if (millis() - lastConnectionTime > postingInterval) {
    Serial.println("Getting data");
    httpRequest();
  }
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the Nina module
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {

    client.println("GET /statecontainer/curtainState.txt?sp=rl&st=2020-01-12T19:52:18Z&se=2032-01-13T19:52:00Z&sv=2019-02-02&sr=c&sig=bMP7niABky1r%2Blb36MFYcTOPRgTRqSSQ8DpiuFEHrNs%3D HTTP/1.0"); //download text
    client.println("Host: matsiotstorage.blob.core.windows.net");

    //client.println("GET /curtainState.txt HTTP/1.0"); //download text
    //client.println("Host: phpstack-181246-1116780.cloudwaysapps.com");
    client.println(); //end of get request

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
