#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>


const char* ap_ssid = "SMARTDB";
const char* ap_password = "SMARTDB1";
const int switchPin = 5; //momentary is connected to GPIO 2
const int ledPin = 16; // GPIO 17 pin connected to the LED
const int ledPinRed = 4; // GPIO 4 pin connected to the LED
const int Relay1 = 16; // GPIO 16 pin connected to the relay module
const int BC547 = 2; //for transistor to serve as a lash switch for timer 555

IPAddress staticIP(192, 168, 1, 2); // Static IP address you want to assign
IPAddress gateway(192, 168, 1, 1);    // IP address of your router
IPAddress subnet(255, 255, 255, 0);    // Subnet mask of your network


AsyncWebServer server(80);
Preferences preferences;
String user_password; // Declare user_password globally
String lashkey; // Declare user_password globally
String quickcode; // Declare user_password globally

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


const char *passhtmlContent = R"(
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
      <p>Enter your recovery password</span>
    <form id="wifiForm" action="/connect" method="post">
        <input type="password" name="user_password" placeholder="enter user password"><br><br>
        <br>
        <button type="submit">Connect</button>
    </form>
</body>
</html>
)";





void handleConnect(AsyncWebServerRequest *request);
void lashConnect(AsyncWebServerRequest *request);

//delete data function
void Deletedata() {
    preferences.remove("wifi_ssid");
    preferences.remove("wifi_password");
    preferences.remove("user_password");
   
    Serial.println("Data deleted.");
    preferences.remove("lash_key");
    preferences.remove("Q_code");
}


// Variable for storing the previous state of the switch
//int previousState = LOW;

//blinking led function for pin 4

void blinkLED(int pin) {
    static unsigned long previousMillis = 0;
    static bool ledState = LOW;
    const unsigned long interval = 500; // less than a second interval

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


//for the momentary switch
unsigned long pressStartTime = 0;  // Variable to store the time when the button was pressed
const unsigned long longPressDuration = 4000;  // Duration in milliseconds to consider a press as "long"


void setup() {
    Serial.begin(115200);
    delay(100); // Delay to allow time for serial monitor to initialize
    
  // Connect to WiFi with static IP configuration
  WiFi.config(staticIP, gateway, subnet);

   // Connect to aces point  with static IP configuration
  WiFi.softAPConfig(staticIP, gateway, subnet);

    // Initialize unique identifier for this ESP32 device
  String uniqueIdentifier = "GhostDB_Alpha";

  pinMode(Relay1, OUTPUT);          // Set the relay pin as an output
  pinMode(ledPin, OUTPUT);          // Set the LED pin as an output
  pinMode(ledPinRed, OUTPUT);       // Set the LED pin as an output
  pinMode(switchPin, INPUT_PULLUP); // Set the switch pin as an input with internal pull-up resistor
  digitalWrite(ledPin, LOW);        // Ensure the LED is initially off

  preferences.begin("my-app", false); // Open preferences with the given namespace

  // Print saved user password
  user_password = preferences.getString("user_password", "");
  quickcode = preferences.getString("Q_code", "");
  lashkey = preferences.getString("lash_key", "");
  Serial.print(lashkey);
  Serial.print(quickcode);
  Serial.println(user_password);


  if(lashkey.equals("clicked")){
    preferences.putString("Q_code", "00"); //insert into Q_code in flash memory 
    WiFi.softAP(ap_ssid, ap_password);
      

  }else{
            // Check if the user password is empty
        //user_password = preferences.getString("user_password", "");
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
            

        }
  
   if(lashkey.equals("clicked")){
                // Initialize web server routes
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            // Check the User-Agent header to determine if the request is from a browser or your app
            String userAgent = request->header("User-Agent");
            if (userAgent.indexOf("Ghost-Switch") != -1) {
                   // Respond with plain text for requests from your Android app
                        request->send(200, "text/plain", "node renew");
                
            } else {
                // Respond with HTML for requests from web browsers
                request->send(200, "text/html", passhtmlContent);
            }
        });
            server.on("/renew", HTTP_POST, lashConnect);
            server.begin();

   }else{

                  // Initialize web server routes
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            // Check the User-Agent header to determine if the request is from a browser or your app
            String userAgent = request->header("User-Agent");
            if (userAgent.indexOf("Ghost-Switch") != -1) {
                     request->send(200, "text/plain", "node");

                
            } else {
                // Respond with HTML for requests from web browsers
                request->send(200, "text/html", htmlContent);
            }
        });
            server.on("/connect", HTTP_POST, handleConnect);
            server.begin();

    }
  
}

void loop() {

/*
     // Print preferences
    Serial.println("WiFi SSID: " + preferences.getString("wifi_ssid"));
    Serial.println("WiFi Password: " + preferences.getString("wifi_password"));
    Serial.println("User Password: " + preferences.getString("user_password"));


Serial.println("User Password: " + preferences.getString("user_password"));
 Serial.print(lashkey);
    Serial.print(quickcode);
    delay(2000);
    
 
 quickcode = preferences.getString("Q_code", "");
    lashkey = preferences.getString("lash_key", "");
     // Print values to serial monitor
  Serial.print("lashkey: ");
  Serial.println(lashkey);
  Serial.print("quickcode: ");
  Serial.println(quickcode);
  */  
   

    //Deletedata(); //call deleted preference data
    delay(3000);
    


 if (quickcode.equals("00"))
    {
         // Check if there are any connected stations to the ESP32's access point
            int stationCount = WiFi.softAPgetStationNum();
             blinkLED(ledPin);
              blinkLED(ledPinRed);


            delay(160000); //delay for 3 minutes so user can change the credentials
            preferences.remove("lash_key");
            preferences.remove("Q_code");
             //WiFi.mode(WIFI_STA);

            delay(2000);
    
              // belowis code code to toggle transitor BC547 with a pin on and off to turn on timer 555
                    digitalWrite(BC547, HIGH);
                    delay(200);
                    digitalWrite(BC547, LOW);
                    delay(4000);

    }

    if (user_password.length() == 0)
        {

            // Check if there are any connected stations to the ESP32's access point
            int stationCount = WiFi.softAPgetStationNum();

            if (stationCount > 0)
            {

                digitalWrite(ledPin, HIGH); // Turn the LED on
            }
            else
            {
                blinkLED(ledPin);
            }
        }
        else
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                //if wifi is coonected 
                digitalWrite(ledPinRed, HIGH); // Turn the LED on

                }else{
            blinkLED(ledPinRed);
            
                }
            }




//----momentary switch-------
   // Read the state of the input pin
  int buttonState = digitalRead(switchPin);

  // Check if the button is pressed
  if (buttonState == LOW) {
    // Record the start time of the press
    if (pressStartTime == 0) {
      pressStartTime = millis();
    }

    // Check if the press duration is longer than the defined long press duration
    if (millis() - pressStartTime >= longPressDuration) {
    
      // Turn on the LED
      digitalWrite(ledPin, HIGH);

       if (WiFi.status() != WL_CONNECTED) {
                    digitalWrite(ledPin, HIGH);
                    digitalWrite(ledPinRed, HIGH);
                    preferences.putString("lash_key", "clicked"); //insert into lash_key in flash memory  
                    delay(2000);

                    // belowis code code to toggle transitor BC547 with a pin on and off to turn on timer 555
                    digitalWrite(BC547, HIGH);
                    delay(200);
                    digitalWrite(BC547, LOW);
                    delay(4000);
                  
                 
                }
    }
  } 

  
//__________________________________






    

}


void handleConnect(AsyncWebServerRequest *request) {
    // Check if the HTTP method of the request is POST
    if (request->method() == HTTP_POST) {
        // Retrieve form data
        if (request->hasParam("ssid", true) && request->hasParam("password", true)  && request->hasParam("user_password", true)) {
            // Retrieve the values of the parameters "ssid", "password", and "user_password"
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();
            String user_password = request->getParam("user_password", true)->value();
            
            // Save WiFi credentials to preferences
            preferences.putString("wifi_ssid", ssid);
            preferences.putString("wifi_password", password);
            preferences.putString("user_password", user_password);

            // Connect to WiFi using the retrieved SSID and password
            Serial.println("Connecting to WiFi...");
            WiFi.begin(ssid.c_str(), password.c_str());

            // Wait for connection
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 10) {
                delay(500);
                Serial.print(".");
                attempts++;
            }

            // Check if WiFi connection is successful
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("Connected to WiFi");
                // Send a response indicating successful connection
                request->send(200, "text/plain", "Connected to WiFi");
            } else {
                Serial.println("Failed to connect to WiFi");
                // Send a response indicating failure to connect
                request->send(500, "text/plain", "Failed to connect to WiFi");
            }
        } else {
            // Send a response indicating invalid request parameters if any of the required parameters are missing
            request->send(400, "text/plain", "Invalid request parameters");
        }
    } else {
        // Send a response indicating invalid HTTP method if the method is not POST
        Serial.println("Invalid HTTP method");
        request->send(405, "text/plain", "Method Not Allowed");
    }
}



void lashConnect(AsyncWebServerRequest *request) {
    if(request->method() == HTTP_POST) {
        // Retrieve form data
        if ( request->hasParam("user_password", true)) {
            
           //get data from input and from preferences
            String user_password = request->getParam("user_password", true)->value();
            String old_user_password = preferences.getString("user_password", "");

             //check if input matches whats in preferences
            if (user_password.equals(old_user_password))
            {
                Deletedata();
                request->send(200, "text/plain", "success. Reboot Ghost DB and create new credentials");
            }else {
                request->send(200, "text/plain", "recovery password does not match");
                    }

          
        } else {
            request->send(400, "text/plain", "Invalid request parameters");
        }
    } else {
           Serial.println("Invalid HTTP method");
        request->send(405, "text/plain", "Method Not Allowed");
    }
}

