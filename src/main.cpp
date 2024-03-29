#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

#include <Arduino.h>

#include <ESPAsyncWebServer.h>
#include <WiFi.h>


const char *ssid = "smartd1";
const char *password = "ghosting";

AsyncWebServer server(80); // Create an AsyncWebServer object on port 80
const char *htmlContent = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Webpage</title>
    <style>
        body{
            text-align: center; color: #fff;
            background-color: rgb(161, 6, 6);
        }
       
        input{
            margin-top:10px; margin-bottom: 10px;
            width: 80%; height: 35px;
            border: none; border-bottom: solid 0.5px #fff;
            background: none; color: #fff;
        }

        input[type="text"]::placeholder, input[type="password"]::placeholder {
            color: #ccc; /* Change placeholder color to gray */
        }

        button{
            width: 80%; border-radius: 3px;
            color: #fff; border: solid 0.5px;
            background: none; height: 35px;
            margin-top: 20px;
        }
    </style>
</head>
<body>
     <h1>Hello,</h1> <p>Enter your smart home router/wifi name and password</span>
    <form id="wifiForm" action="/connect" method="post">
        <input type="text" name="ssid" placeholder="enter wifi name"><br>
          <br>
        <input type="password" name="password" placeholder="enter wifi password"><br>
        
        <br>
        <button type="submit">Connect</button>
    </form>
</body>
</html>
)";



IPAddress local_IP(192, 168, 1, 1); // Set your desired static IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);




const int switchPin = 2; // Assuming switch x is connected to GPIO 2

const int ledPin = 4; // GPIO pin connected to the LED

void setup() {
   // Initialize Serial port
    Serial.begin(115200);

     delay(1000); // Delay to allow time for Serial Monitor to connect


   pinMode(ledPin, OUTPUT); // Set the LED pin as an output
 // digitalWrite(ledPin, HIGH); // Turn the LED on


  // Configure switch pin as input
pinMode(switchPin, INPUT);

// Initialize WiFi in access point mode
WiFi.softAP(ssid, password);

// Print IP address of the access point
Serial.println("Access Point Started");
Serial.print("IP Address: ");
Serial.println(WiFi.softAPIP());

//below is for server______

// Configure captive portal// Set subnet mask to 255.255.255.0
    WiFi.softAPConfig(WiFi.softAPIP(), WiFi.softAPIP(), IPAddress(255, 255, 255, 0)); 
    // Define request handler to serve the HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", htmlContent);
    });

      // Define request handler to handle WiFi credentials
    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Extract SSID and password from the POST request
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();

            // Connect to the specified WiFi network
            WiFi.begin(ssid.c_str(), password.c_str());

            request->send(200, "text/plain", "Connecting to WiFi...");
        } else {
            request->send(400, "text/plain", "Missing SSID or password");
        }
    });




  // Start the server
  server.begin();
  Serial.println("HTTP server started");

  //--server-----

  
  
}

void loop() {
    


      // Check if there are any connected stations to the ESP32's access point
    int stationCount = WiFi.softAPgetStationNum();

    if (stationCount > 0) {
        
            digitalWrite(ledPin, HIGH); // Turn the LED on
    } 
    else {
       delay(1000);
      
        digitalWrite(ledPin, HIGH); // Turn the LED on
         delay(1000);
           digitalWrite(ledPin, LOW); // Turn the LED off
    }
    

    //delay(5000); // Check status every 5 second
  
  

 

}




// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}