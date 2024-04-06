#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

const char* ap_ssid = "SMARTDB";
const char* ap_password = "SMARTDB1";
const int switchPin = 2; //momentary is connected to GPIO 2
const int ledPin = 17; // GPIO 17 pin connected to the LED
const int ledPinRed = 4; // GPIO 4 pin connected to the LED


// Variable for storing the previous state of the switch
int previousState = LOW;

AsyncWebServer server(80);
Preferences preferences;
String user_password; // Declare user_password globally

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
            background: none; height: 50px;
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
        <input type="password" name="user_password" placeholder="enter user password"><br><br>
        <br>
        <button type="submit">Connect</button>
    </form>
</body>
</html>
)";






void handleConnect(AsyncWebServerRequest *request);

//delete data function
void Deletedata() {
    preferences.remove("wifi_ssid");
    preferences.remove("wifi_password");
    preferences.remove("user_password");
     Serial.println("Data deleted.");
}

//blinking led function for pin 4
void blinkLED(int pin) {
    static unsigned long previousMillis = 0;
    static bool ledState = LOW;
    const unsigned long interval = 1000; // 1 second interval

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        // Save the last time the LED state was toggled
        previousMillis = currentMillis;

        // Toggle the LED state
        ledState = !ledState;

        // Set the LED to the new state
        digitalWrite(pin, ledState);
    }

}

void setup() {
    Serial.begin(115200);
    delay(100); // Delay to allow time for serial monitor to initialize

    // Initialize unique identifier for this ESP32 device
  String uniqueIdentifier = "GhostDB_Alpha";

    pinMode(ledPin, OUTPUT); // Set the LED pin as an output
    pinMode(ledPinRed, OUTPUT); // Set the LED pin as an output
    pinMode(switchPin, INPUT_PULLUP); // Set the switch pin as an input with internal pull-up resistor
    digitalWrite(ledPin, LOW); // Ensure the LED is initially off

    preferences.begin("my-app", false); // Open preferences with the given namespace
    
    
      // Print saved user password
    user_password = preferences.getString("user_password", "");
    Serial.print("Saved user password: ");
    Serial.println(user_password);

    // Check if the user password is empty
 user_password = preferences.getString("user_password", "");
    if (user_password.length() == 0) {
        // User password is empty, configure ESP32 as an Access Point
           WiFi.softAP(ap_ssid, ap_password);
    } else {
        // User password is not empty, connect to WiFi using saved credentials
        String saved_ssid = preferences.getString("wifi_ssid", "");
        String saved_password = preferences.getString("wifi_password", "");
        WiFi.begin(saved_ssid.c_str(), saved_password.c_str());
        
        // Wait for WiFi connection
        int retries = 0;
        while (WiFi.status() != WL_CONNECTED && retries < 10) {
            delay(500);
            Serial.print(".");
            retries++;
        }
        if (WiFi.status() == WL_CONNECTED) {
       
            digitalWrite(ledPinRed, HIGH); // Turn the LED onto indicate wifi connected
        } else {
            WiFi.softAP(ap_ssid, ap_password);
   
        }
    }
    
    // Initialize web server routes
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // Check the User-Agent header to determine if the request is from a browser or your app
    String userAgent = request->header("User-Agent");
    if (userAgent.indexOf("Ghost-Switch") != -1) {
        // Respond with plain text for requests from your Android app
        request->send(200, "text/plain", "node");
    } else {
        // Respond with HTML for requests from web browsers
        request->send(200, "text/html", htmlContent);
    }
});
    server.on("/connect", HTTP_POST, handleConnect);
    server.begin();
}

void loop() {

/*
     // Print preferences
    Serial.println("WiFi SSID: " + preferences.getString("wifi_ssid"));
    Serial.println("WiFi Password: " + preferences.getString("wifi_password"));
    Serial.println("User Password: " + preferences.getString("user_password"));
*/
    
   

    //Deletedata(); //call deleted preference data
    
   

    if (user_password.length() == 0) {
        // User password is empty,
         
         // Check if there are any connected stations to the ESP32's access point
                int stationCount = WiFi.softAPgetStationNum();

                if (stationCount > 0) {
                    
                        digitalWrite(ledPin, HIGH); // Turn the LED on
                } 
                else {
                    blinkLED(ledPin);
                   
                }
       
    } else{
         if (WiFi.status() == WL_CONNECTED) {
        //if wifi is coonected 
         digitalWrite(ledPinRed, HIGH); // Turn the LED on

        }else{
       blinkLED(ledPinRed);
       
        }
    }

//----momentary switch-------
     int currentState = digitalRead(switchPin); // Read the current state of the switch
  // Check if the switch state has changed
    if (currentState != previousState) {
        delay(50); // Debounce delay

        // Check if the new state of the switch is pressed (LOW)
        if (currentState == LOW) {
            // Toggle the LED state
                if (WiFi.status() == WL_CONNECTED) {
                //if wifi is coonected do nothing
                }else{
                        // User password is empty, configure ESP32 as an Access Point
                          WiFi.softAP(ap_ssid, ap_password);
                        }
        }
    previousState = currentState; // Update the previous state for the next iteration
    delay(100); // Add a small delay for stability
    }
//__________________________________


    

}


void handleConnect(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_POST) {
        // Retrieve form data
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();
            String user_password = request->getParam("user_password", true)->value();
            
            // Save WiFi credentials to preferences
            preferences.putString("wifi_ssid", ssid);
            preferences.putString("wifi_password", password);
            preferences.putString("user_password", user_password);

            // Connect to WiFi
            Serial.println("Connecting to WiFi...");
            WiFi.begin(ssid.c_str(), password.c_str());

            // Wait for connection
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 10) {
                delay(500);
                Serial.print(".");
                attempts++;
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("Connected to WiFi");
                request->send(200, "text/plain", "Connected to WiFi");
            } else {
                Serial.println("Failed to connect to WiFi");
                request->send(500, "text/plain", "Failed to connect to WiFi");
            }
        } else {
            request->send(400, "text/plain", "Invalid request parameters");
        }
    } else {
           Serial.println("Invalid HTTP method");
        request->send(405, "text/plain", "Method Not Allowed");
    }
}


