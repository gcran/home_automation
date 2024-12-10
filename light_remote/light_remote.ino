#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

const char *ID = "light";  // Name of our device, must be unique
const char *CMD_TOPIC = "light/command";  // Topic to subcribe to
const char *STA_TOPIC = "light/status";  // Topic to subcribe to

const char* broker = "hab.local";
WiFiClient wclient;

PubSubClient client(wclient); // Setup MQTT client
char command_str[50];

long prev_time = 0;

// Define WiFiManager Object
WiFiManager wm;

const int RST_PIN = 0;  
const int RELAY_PIN = 5;
bool curr_state = LOW;  

void configModeCallback(WiFiManager *myWiFiManager)
// Called when config mode launched
{
  Serial.println("Entered Configuration Mode");
 
  Serial.print("Config SSID: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
 
  Serial.print("Config IP Address: ");
  Serial.println(WiFi.softAPIP());
}

// Connect to WiFi network
void setup_wifi() {
  // Change to true when testing to force configuration every time we run
  bool forceConfig = false;

  // Explicitly set WiFi mode
  WiFi.mode(WIFI_STA);
 
  // Reset settings (only for development)
  //wm.resetSettings();
 
  // Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  //set timeout for network connection attempt
  wm.setConnectTimeout(30);
 
  if (forceConfig)
    // Run if we need a configuration
  {
    if (!wm.startConfigPortal("ESP_CONFIG"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
  }
  else
  {
    if (!wm.autoConnect("ESP_CONFIG"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // if we still have not connected restart and try all over again
      ESP.restart();
      delay(5000);
    }
  }

  // If we get here, we are connected to the WiFi 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Reconnect to client
void reconnect() {
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (client.connect(ID)) {
    Serial.println("connected");
    Serial.print("Publishing to: ");
    Serial.println(STA_TOPIC);
    Serial.println('\n');
    client.subscribe(CMD_TOPIC);
    Serial.print("Subscribing to: ");
    Serial.println(CMD_TOPIC);
    Serial.println('\n');
    // digitalWrite(LIGHT, 1);

  } else {
    Serial.println(" Connection failed, retrying");
    // Wait 5 seconds before retrying
    // delay(5000);
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == String(CMD_TOPIC)) {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      curr_state = HIGH;
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      curr_state = LOW;
    }
    digitalWrite(RELAY_PIN, curr_state);
  }
}

void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud
  setup_wifi(); // Connect to network  
  delay(100);
  pinMode(RELAY_PIN, OUTPUT);
  
  client.setServer(broker, 1883);

  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:

  if(digitalRead(RST_PIN) == LOW) {
    Serial.println("Reset button pushed");
    wm.resetSettings();
    setup_wifi();
  }

  if (!client.connected())  // Reconnect if connection is lost
  {
    reconnect();
  }
  client.loop();
  long curr_time = millis();
  if ((curr_time - prev_time) > 1000) {
    sprintf(command_str, "%lu", curr_state);
    client.publish(STA_TOPIC, command_str);
    Serial.println(curr_state);
    prev_time = curr_time;
  }
  



}
