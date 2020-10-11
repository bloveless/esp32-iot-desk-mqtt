#include "Arduino.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "TFMiniS.h"
#include <EEPROM.h>
#include "environment.h"

#include "state_machine.h"

#define DEVICE_ID "77078ff2-0684-11eb-a8e7-0242ac130002"
#define DEVICE_COMMAND_TOPIC "/esp32_iot_desk/77078ff2-0684-11eb-a8e7-0242ac130002/command"

#define UP_PWM_PIN 26
#define UP_PWM_CHANNEL 0
#define DOWN_PWM_PIN 25
#define DOWN_PWM_CHANNEL 1
#define PWM_FREQUENCY 5000
#define PWM_RESOLUTION 8

WiFiClient espClient;
PubSubClient client(espClient);
StateMachine *state_machine;
TFMiniS tfminis;
uint8_t current_desk_height;

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, SSID_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  if (messageTemp == "1") {
    state_machine->requestStateChange(ADJUST_TO_PRESET_1_HEIGHT_STATE);
  }

  if (messageTemp == "2") {
    state_machine->requestStateChange(ADJUST_TO_PRESET_2_HEIGHT_STATE);
  }

  if (messageTemp == "3") {
    state_machine->requestStateChange(ADJUST_TO_PRESET_3_HEIGHT_STATE);
  }

  Serial.println();
}

void updateDeskHeight(void* parameter) {
    for(;;) {
        Measurement m = tfminis.triggerMeasurement();

        current_desk_height = m.distance;

        Serial.print("Current desk height: ");
        Serial.println(current_desk_height);

        delay(1000);
    }
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(TFMINIS_BAUDRATE);
  EEPROM.begin(3);

  tfminis.begin(&Serial2);
  tfminis.setFrameRate(0);

  ledcSetup(UP_PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(UP_PWM_PIN, UP_PWM_CHANNEL);

  ledcSetup(DOWN_PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(DOWN_PWM_PIN, DOWN_PWM_CHANNEL);

  state_machine = new StateMachine();
  state_machine->begin(&current_desk_height, UP_PWM_CHANNEL, DOWN_PWM_CHANNEL);

  setup_wifi();

  client.setServer(MQTT_SERVER_DOMAIN, MQTT_SERVER_PORT);
  client.setCallback(callback);

  xTaskCreate(
      updateDeskHeight,     // Function that should be called
      "Update Desk Height", // Name of the task (for debugging)
      1024,                 // Stack size
      NULL,                 // Parameter to pass
      5,                    // Task priority
      NULL                  // Task handle
  );
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe(DEVICE_COMMAND_TOPIC, 1);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  state_machine->processCurrentState();
}
