#include <Arduino.h>
#include <AsyncTCP.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <Button2.h>

#define BUTTON_A_PIN  14
#define BUTTON_B_PIN  13
#define BUTTON_C_PIN  15
#define drv_sleep  18

bool motor_start = false;
bool fast_move_run = false;
bool fast_move_stop = false;
bool motor_back_run = false;
bool motor_back_stop = false;

Button2 buttonA, buttonB, buttonC;
AsyncWebServer server(80);
Preferences preferences;

const char* ssid = "OpenInjector";
const char* password = "123456789";

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

unsigned int compresstime;
unsigned int maintaintime;
unsigned int pumpbacktime;

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void motorloop() {
  if (motor_start) {
    ledcWrite(0, 0);
    ledcWrite(1, 1010);
    delay(compresstime);
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    delay(maintaintime);
    ledcWrite(0, 1010);
    ledcWrite(1, 0);
    delay(pumpbacktime);
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    motor_start = 0;
  }
  if (fast_move_run) {
    ledcWrite(0, 0);
    ledcWrite(1, 1010);
    fast_move_run = !fast_move_run;
  }
   if (fast_move_stop) {
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    fast_move_stop = !fast_move_stop;
  }

  if (motor_back_run) {
    ledcWrite(1, 0);
    ledcWrite(0, 1010);
    motor_back_run = !motor_back_run;
  }
  if (motor_back_stop) {
    ledcWrite(1, 0);
    ledcWrite(0, 0);
    motor_back_stop = !motor_back_stop;
  }
}
void entersetuppage() {
  WiFi.softAP(ssid, password);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/script.js", "text/plain");
  });
  server.on("/getpara", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(compresstime) + "," + String(maintaintime) + "," + String(pumpbacktime));
  });

  server.on("/leave.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    server.end();
    preferences.putUInt("compresstime", compresstime);
    preferences.putUInt("maintaintime", maintaintime);
    preferences.putUInt("pumpbacktime", pumpbacktime);

    WiFi.mode(WIFI_OFF);
    btStop();
  });
  server.on("/leavewithoutsave.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    server.end();
    WiFi.mode(WIFI_OFF);
    btStop();
  });


  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      compresstime = inputMessage.toInt();
    }
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      maintaintime = inputMessage.toInt();
    }
    else if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      pumpbacktime = inputMessage.toInt();
    }
    else {
      inputMessage = "No message sent";
    }
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.onNotFound(notFound);
  server.begin();
}
void handler(Button2& btn) {
  if (btn == buttonA) {
    if (btn.getClickType() == LONG_CLICK) {
      entersetuppage();
    }
    else {
      motor_start = 1;
    }
  }
  else if (btn == buttonB) {
      Serial.println("B clicked");
    }
  else if (btn == buttonC) {
      Serial.println("C clicked");
  }
}
void pressedhandler(Button2& btn) {
  if (btn == buttonB) {
    Serial.println("B pressed");
    fast_move_run = true;
  }
  else if (btn == buttonC) {
    Serial.println("C pressed");
    motor_back_run = true;
  }
}
void releasedhandler(Button2& btn) {
  if (btn == buttonB) {
    Serial.println("B released");
    fast_move_stop = true;
  }
  else if (btn == buttonC) {
    Serial.println("C released");
    motor_back_stop = true;
  }
}
void doubleclickhandler(Button2& btn) {
if (btn == buttonC) {
    Serial.println("C double clicked");
    entersetuppage();
  }
}
void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  preferences.begin("OpenInjector", false);
  compresstime = preferences.getUInt("compresstime", 300);
  maintaintime = preferences.getUInt("maintaintime", 100);
  pumpbacktime = preferences.getUInt("pumpbacktime", 200);

  buttonA.begin(BUTTON_A_PIN);
  buttonA.setDebounceTime(100);
  buttonA.setClickHandler(handler);
  // buttonA.setLongClickHandler(handler);

  buttonB.begin(BUTTON_B_PIN);
  buttonB.setDebounceTime(100);
  buttonB.setClickHandler(handler);
  buttonB.setPressedHandler(pressedhandler);
  buttonB.setReleasedHandler(releasedhandler);

  buttonC.begin(BUTTON_C_PIN);
  buttonC.setDebounceTime(100);
  buttonC.setClickHandler(handler);
  buttonC.setPressedHandler(pressedhandler);
  buttonC.setReleasedHandler(releasedhandler);
  buttonC.setDoubleClickHandler(doubleclickhandler);


  pinMode(drv_sleep, OUTPUT);
  digitalWrite(drv_sleep, LOW);

  ledcSetup(0, 1000, 10);
  ledcSetup(1, 1000, 10);
  ledcAttachPin(23, 0);
  ledcAttachPin(19, 1);
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  digitalWrite(drv_sleep, HIGH);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  buttonA.loop();
  buttonB.loop();
  buttonC.loop();
  motorloop();
}