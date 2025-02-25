#include <Arduino.h>
#include <SD.h>

// #include <map>
#include <ArduinoJson.h>

#include <ostream>
#include <string>

//                      RAM1.var    RAM1.code   RAM1.padding    RAM1.free
// Blinky:              3808         6240        26528           487712
// std::string          11072        68764       29540           414912
// std::iostream        66016       303932       23748           130592
// std::unordered_map   12096        70016       28288           413888
// std::map             11072        70180       28124           414912
// both maps            12096        71360       26944           413888
// ArduinoJson mem       4896        17952       14816           486624
// ArduinoJson SD        8640        54952       10548           450112

// put function declarations here:
int myFunction(int, int);

void setup() {
    std::string foo = "stuff";

    JsonDocument doc;
    File file = SD.open("foo.json", FILE_READ);
    char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
    DeserializationError error = deserializeJson(doc, file);

    const char* sensor = doc["sensor"];
    long time = doc["time"];
    double latitude = doc["data"][0];
    double longitude = doc["data"][1];

    // put your setup code here, to run once:
    int result = myFunction(2, 3);
}

void loop() {
    // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
    return x + y;
}