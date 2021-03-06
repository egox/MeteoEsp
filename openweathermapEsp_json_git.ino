#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "LedControl.h"
#include <UniversalTelegramBot.h>

LedControl lc = LedControl(D4, D2, D3, 1);

// Initialize Wifi connection to the router
char ssid[] = "XXX"; //  your network SSID (name)
char pass[] = "XXXX"; //  your network pwd (name)
//int keyIndex = 0;            // your network key Index number (needed only for WEP)

// Initialize Telegram BOT
#define BOTtoken "XXXXXX"  // your Bot Token (Get from Botfather)
String botname = "meteoesp";

WiFiClientSecure client;
//bool debug = false;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 500; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

String apiKey = "XXXXX"; //to be updated
String clima;

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "api.openweathermap.org";    // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):

WiFiClient client1;

void setup() {

  /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
  */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 4);
  /* and clear the display */
  lc.clearDisplay(0);


  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // attempt to connect to Wifi network:


  int tentativo = 1;

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    smileyFace(tentativo);
    tentativo++;
    if (tentativo == 6) {
      tentativo = 1;
    }
    // wait 1 seconds for connection:
    delay(1000);


  }
  Serial.println("Connected to wifi");
}

void setface(byte *face) {
  //  lc.clearDisplay(0);
  //  delay(500);

  for (int k = 0; k < 3; k++) {
    for (int j = 0; j < 15; j++) {
      lc.setIntensity(0, j);
      for (int i = 0; i < 15; i++) {
        lc.setColumn(0, i, face[7 - i]);
      }
      delay(50);
    }
    for (int j = 14; j > -1; j--) {
      lc.setIntensity(0, j);
      for (int i = 0; i < 15; i++) {
        lc.setColumn(0, i, face[7 - i]);
      }
      delay(50);
    }
    //delay(200);
    //  lc.clearDisplay(0);
  }
}

void smileyFace(int face) {

  byte rainled[8] = {
    B00000000,
    B00110110,
    B00000000,
    B01101100,
    B00000000,
    B00110110,
    B00000000,
    B01101100
  };

  byte rainled2[8] = {
    B00000000,
    B00010000,
    B01010100,
    B01000100,
    B00010000,
    B01010100,
    B01000100,
    B00000000
  };

  byte clearled[8] = {
    B00000000,
    B01010100,
    B00111000,
    B01111100,
    B00111000,
    B01010100,
    B00000000,
    B00000000
  };

  byte cloudsled[8] = {
    B00001100,
    B00011100,
    B00111100,
    B00111100,
    B00011100,
    B00001100,
    B00011100,
    B00001100
  };

  byte snowled[8] = {
    B00000000,
    B00101010,
    B01101011,
    B00011100,
    B01111111,
    B00011100,
    B01101011,
    B00101010
  };

  byte stormled[8] = {
    B00000000,
    B00000000,
    B01001100,
    B00101010,
    B00011001,
    B00001000,
    B00000000,
    B00000000
  };

  byte face1[8] = {
    B00000000,
    B11000011,
    B00000000,
    B00011000,
    B00000000,
    B10000001,
    B01111110,
    B00000000
  };

  byte face2[8] = {
    B11000011,
    B11000011,
    B00000000,
    B00011000,
    B00000000,
    B01111110,
    B01000010,
    B01111110
  };

  byte face3[8] = {
    B11000000,
    B11000011,
    B00000000,
    B00011000,
    B00000000,
    B00000000,
    B01111110,
    B00001100
  };

  if (face == 1)
    setface(cloudsled);
  else if (face == 2)
    setface(clearled);
  else if (face == 3)
    setface(snowled);
  else if (face == 4)
    setface(rainled);
  else if (face == 5)
    setface(stormled);

}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      //Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }

//  if (status != WL_CONNECTED) {
//    Serial.println("disconnesso");
//    ESP.reset();
//  }

  delay(1000);
}

void getWeather(String location, String chat_id) {

  //client.stop();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client1.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client1.print("GET /data/2.5/weather?");
    client1.print("q=" + location);
    client1.print("&apiKey=" + apiKey);
    client1.println("&units=metric");

    client1.println("Host: api.openweathermap.org");
    client1.println("Connection: close");
    client1.println();
  } else {
    Serial.println("unable to connect");
  }

  delay(1000);
  String line = "";

  while (client1.connected()) {
    line = client1.readStringUntil('\n');
    Serial.println(line);

    //json example:
    //{"coord":{"lon":7.69,"lat":45.07},"weather":[{"id":741,"main":"Fog","description":"fog","icon":"50n"}],"base":"stations","main":{"temp":0.41,"pressure":1015,"humidity":81,"temp_min":-13,"temp_max":8},"visibility":10000,"wind":{"speed":3.1,"deg":240},"clouds":{"all":0},"dt":1493670000,"sys":{"type":1,"id":5795,"message":0.0045,"country":"IT","sunrise":1493612225,"sunset":1493663769},"id":3165524,"name":"Torino","cod":200}


    DynamicJsonBuffer jsonBuffer;

    // You can use a String as your JSON input.
    // WARNING: the content of the String will be duplicated in the JsonBuffer.

    JsonObject& root = jsonBuffer.parseObject(line);


    // You can get a String from a JsonObject or JsonArray:
    // No duplication is done, at least not in the JsonBuffer.

    //int meteo = root["main"]["humidity"];  //humidity value
    String weather = root["weather"][0]["main"];  //main weather conditions
    String city = root["name"];  //città interrogata

    //Serial.println(meteo);
    Serial.println(city);
    Serial.println(weather);


    if (weather == "Clouds") {
      smileyFace(1);
      Serial.println("Clouds = face 1");
      clima = "nuvoloso, che tristezza!";
    }

    else if (weather == "Clear") {
      smileyFace(2);
      Serial.println("Clear = face 2");
      clima = "sereno, era ora!";
    }

    else if (weather == "Snow") {
      smileyFace(3);
      Serial.println("Snow = face 3");
      clima = "nevica, buon Natale!";
    }

    else if (weather == "Rain" || "Drizzle") {
      smileyFace(4);
      Serial.println("Rain = face 4");
      clima = "piove, mai 'na gioia!";
    }

    else if (weather == "Thunderstorm") {
      smileyFace(5);
      Serial.println("Storm = face 5");
      clima = "tempesta ed impeto, meglio non uscire!";
    }

    else {
      smileyFace(1);
      Serial.println("Altro= face 1 = clouds");
      clima = "variabile, vedi un po' tu!";
    }

    bot.sendChatAction(chat_id, "typing");

    //bot.sendMessage(chat_id, city);
    bot.sendMessage(chat_id, clima);

    client1.stop();
  }
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  //Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    Serial.print("Chat id ");
    Serial.println(chat_id);
    //Serial.print("Text - ");
    //Serial.println(text);

    //bot.sendChatAction(chat_id, "typing");
    //bot.sendMessage(chat_id, text);

    if (text == "/start") {
      String welcome = "Ciao " + from_name + ", questo è " + botname + " Telegram Bot.\n";
      welcome += "Questo bot ti fa conoscere il meteo della città che ti interessa\n";
      welcome += "... basta scriverne qui sotto il nome!\n";
      bot.sendMessage(chat_id, welcome);
    }

    else {
      getWeather(text, chat_id);
    }
  }
}
