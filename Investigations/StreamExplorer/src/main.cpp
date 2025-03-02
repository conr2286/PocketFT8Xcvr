#include <Arduino.h>

String s;

class Foo: public Stream {

  public:
  void bar(const char* str) {
      printf("%s\n", str);
  }

  size_t write(uint8_t b) {
      s += b;
  }

  int available() {
      return 1;
  }

  int read() {
      return 0;
  }

  int peek() {
      return 0;
  }
};

// put function declarations here:
int myFunction(int, int);

void setup() {
    Foo obj;

    Serial.printf("Starting...\n");

    obj.bar("Some text");

    printf("s = %s\n", s.c_str());
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