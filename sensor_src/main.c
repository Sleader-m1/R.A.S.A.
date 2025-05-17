#include <Arduino.h>
#include <MHZ19.h>
#include <GyverBME280.h>
#include <FastBot2.h>
#include <CharPlot.h>
#include <WiFi.h>

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <PubSubClient.h>

// #define WIFI_SSID "TP-Link_89FF"
// #define WIFI_PASS "yuriy147950"
#define WIFI_SSID "Abdullaev internet"
#define WIFI_PASS "12345678"
#define BOT_TOKEN "--------------------------------"
#define CHAT_ID1 "390056126"
#define CHAT_ID2 "626236748"
#define CHAT_ID3 "873902520"

const char* mqtt_server = "192.168.43.162";///////////////////////////////////////////////

WiFiClient espClient;
PubSubClient client(espClient);


FastBot2 bot;
MHZ19 mhz;
GyverBME280 bme;

float co2_30[30] = {0}, co2_120[30] = {0};
float temp_30[30] = {0}, temp_120[30] = {0};
float co2_30_mean, co2_120_mean, temp_30_mean, temp_120_mean;
int saveattempts;
int save30, save120;

int curCo2;
float curTemp, curHum, curPres;

uint8_t samples;
int co2buffer;
float tempbuffer, humbuffer, presbuffer;

uint8_t calib;

uint32_t sendperiod = 20000;
uint32_t plotperiod = 15000;
uint32_t getdataperiod = 10000;
uint32_t sendtimer, plottimer, getdatatimer;


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

/*
void showMainMenu()
{
  fb::Message msg1("МЕНЮ", CHAT_ID1);
  fb::Message msg2("МЕНЮ", CHAT_ID3);
  fb::Menu menu;
  // задаётся в CSV: горизонтальный разделитель ; вертикальный - \n
  menu.text = "Графики;Период отправки;ПУСТО";
  menu.resize = 1;
  menu.placeholder = "АМОГУС";
  msg1.setMenu(menu); // подключить меню
  msg2.setMenu(menu); // подключить меню
  bot.sendMessage(msg1, CHAT_ID1);
  bot.sendMessage(msg2, CHAT_ID3);
}

void handleMessage(fb::Update &u)
{
  if (u.isMessage())
  {
    String text = u.message().text().decodeUnicode();
    if (text == "/start")
    {
      bot.sendMessage(fb::Message("Привет! Я бот-метеостанция\nЯ работаю по белому списку\nотправь свой id @okwell", u.message().from().id()));
      //showMainMenu();
    }
    if (text == ("Графики"))
    {
      bot.deleteMessage(u.message().chat().id(), u.message().id());
      fb::Message msg("Выберите график: ", u.message().from().id());
      fb::Menu menu;
      menu.text = "Температура 30 мин;Температура 1 час\nCO2 30 мин;CO2 1 час;Выход";
      menu.resize = 1;
      menu.placeholder = "АМОГУС2";
      msg.setMenu(menu); // подключить меню
      bot.sendMessage(msg);
    }
    if (text == ("CO2 30 мин"))
    {
      bot.sendMessage(fb::Message(CharPlot<COLON_X2>(co2_30, 30, 15, 0), u.message().from().id()));
    }
    if (text == ("CO2 1 час"))
    {
      bot.sendMessage(fb::Message(CharPlot<COLON_X2>(co2_120, 30, 15, 0), u.message().from().id()));
    }
    if (text == ("Температура 30 мин"))
    {
      bot.sendMessage(fb::Message(CharPlot<COLON_X2>(temp_30, 30, 15, 0), u.message().from().id()));
    }
    if (text == ("Температура 1 час"))
    {
      bot.sendMessage(fb::Message(CharPlot<COLON_X2>(temp_120, 30, 15, 0), u.message().from().id()));
    }
    if (text == ("Выход"))
    {
      showMainMenu();
    }
    if (text == "Период отправки")
    {
      bot.deleteMessage(u.message().chat().id(), u.message().id());
      fb::Message msg("Выберите период:", u.message().from().id());
      fb::Menu menu;
      menu.text = "15 секунд;1 минута;5 минут;Выход";
      menu.resize = 1;
      menu.placeholder = "АМОГУС3";
      msg.setMenu(menu); // подключить меню
      bot.sendMessage(msg);
    }
    if (text == ("15 секунд"))
    {
      sendperiod = 15000;
    }
    if (text == ("1 минута"))
    {
      sendperiod = 60000;
    }
    if (text == ("5 минут"))
    {
      sendperiod = 300000;
    }
  }
}

void updateh(fb::Update &u)
{
  Serial.println("---NEW MESSAGE---");
  Serial.print("FROM: ");
  Serial.println(u.message().from().username());
  Serial.print("ID: ");
  Serial.println(u.message().from().id());
  Serial.print("TEXT: ");
  Serial.println(u.message().text().decodeUnicode());
  Serial.println("");

  // #1
  // отправить обратно в чат (эхо)
  // bot.sendMessage(fb::Message(u.message().text(), u.message().chat().id()));

  // #2
  if (u.isMessage())
    handleMessage(u);
}*/

void connectWiFi()
{
  delay(2000);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (millis() > 15000)
      ESP.restart();
  }
  Serial.println("Connected");
  Serial.println(WiFi.localIP());
}

int mhzgetco2()
{
  static int filtVal;
  filtVal += (mhz.getCO2() - filtVal) / 4;

  return filtVal;
}

float bmegettemp()
{
  static float filtVal;
  filtVal += (bme.readTemperature() - filtVal) / 2;

  return filtVal;
}

float bmegethum()
{
  static float filtVal;
  filtVal += (bme.readHumidity() - filtVal) / 2;

  return filtVal;
}

float bmegetpres()
{
  static float filtVal;
  filtVal += (pressureToMmHg(bme.readPressure()) - filtVal) / 2;

  return filtVal;
}

void calibrate()
{
  bot.sendMessage(fb::Message("ABC Статус: ", CHAT_ID1));
  mhz.getABC() ? bot.sendMessage(fb::Message("ON", CHAT_ID1)) : bot.sendMessage(fb::Message("OFF", CHAT_ID1));
  bot.sendMessage(fb::Message("Калибровка..", CHAT_ID1));
  mhz.calibrate();
}

void setup()
{
  pinMode(32, OUTPUT);
  digitalWrite(32, HIGH);

  Serial.begin(115200);
  Serial.println(F("Starting..."));

  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  mhz.begin(Serial2);
  mhz.setRange(5000);
  mhz.autoCalibration(false);

  bme.begin();

  connectWiFi();

  client.setServer(mqtt_server, 1883);
  digitalWrite(32, LOW);

  /*bot.attachUpdate(updateh); // подключить обработчик обновлений
  bot.setToken(BOT_TOKEN);   // установить токен
  bot.setPollMode(fb::Poll::Long, 20000);

  // bot.sendMessage(fb::Message("START", CHAT_ID1));
  showMainMenu();*/
}

void loop()
{
  //bot.tick();

  if (!client.connected()) {
    reconnect();
  }
  
  if (WiFi.status() != WL_CONNECTED){
    connectWiFi();
  }

  if (millis() - getdatatimer > getdataperiod){
    getdatatimer = millis();
    curCo2 = mhzgetco2();
    curTemp = bmegettemp();
    curPres = bmegetpres();
    curHum = bmegethum();
  }

  if (millis() - sendtimer > sendperiod)
  {
    sendtimer = millis();
    String data;
    data = "CO2: " + String(curCo2) + " ppm, ";
    data += "t: " + String(curTemp, 1) + "C, ";
    data += "Hum: " + String(curHum, 1) + "%, ";
    data += "P: " + String(curPres, 0) + " mmHg";
    Serial.println("----DATA----");
    Serial.println(data);
    Serial.println("");

    StaticJsonDocument<80> doc;
    char output[80];

    doc["payload"]["t"] = curTemp;
    doc["p"] = curPres;
    doc["h"] = curHum;
    doc["g"] = curCo2;

    serializeJson(doc, output);
    Serial.println(output);
    client.publish("/home/sensors", output);

    digitalWrite(32, HIGH);
    delay(100);
    digitalWrite(32, LOW);

    //bot.sendMessage(fb::Message(data, CHAT_ID1));
    //bot.sendMessage(fb::Message(data, CHAT_ID3));
    // bot.sendMessage(fb::Message(CharPlot<COLON_X2>(arr, 30, 15, 0), CHAT_ID1));
    // bot.sendMessage(data, CHAT_ID2);
  }
  /*
  if (millis() - plottimer > plotperiod)
  {
    plottimer = millis();
    saveattempts++;
    co2_30_mean += curCo2 / 4;
    temp_30_mean += curTemp / 4;
    co2_120_mean += curCo2 / 8;
    temp_120_mean += curTemp / 8;
    if (!(saveattempts % 4))
    {
      co2_30[save30] = co2_30_mean;
      temp_30[save30] = temp_30_mean;
      co2_30_mean = 0;
      temp_30_mean = 0;
      save30++;
      if (save30 > 29)
      {
        save30 = 29;
        memcpy(&co2_30[0], &co2_30[1], sizeof(float) * 29);
        memcpy(&temp_30[0], &temp_30[1], sizeof(float) * 29);
      }

      if (!(saveattempts % 8))
      {
        saveattempts = 0;
        co2_120[save120] = co2_120_mean;
        temp_120[save120] = temp_120_mean;
        co2_120_mean = 0;
        temp_120_mean = 0;
        save120++;
        if (save120 > 29)
        {
          save120 = 29;
          memcpy(&co2_120[0], &co2_120[1], sizeof(float) * 29);
          memcpy(&temp_120[0], &temp_120[1], sizeof(float) * 29);
        }
      }
    }
  }*/
}
