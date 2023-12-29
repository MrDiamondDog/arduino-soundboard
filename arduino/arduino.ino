#include <OneBitDisplay.h>

#define NUM_DISPLAYS 5
#define OLED_SIZE OLED_128x64
#define FONT FONT_12x16
#define FONT_SMALL FONT_8x8

OBDISP obd[NUM_DISPLAYS];

bool ready = false;

class Fader {
public:
  int volume = 0;
  int id = 0;
  int pot = A0;
  char* name = "Empty";

  Fader(int id, int pot) {
    this->id = id;

    this->pot = pot;

    pinMode(pot, INPUT);
    volume = read();
  }

  int read() {
    return map(analogRead(pot), 0, 1023, 100, 0);
  }

  void updateVolume() {
    int newVal = read();

    if (newVal <= 3) newVal = 0;

    if (abs(volume - newVal) >= 2) {
      volume = newVal;
      if (Serial) sendUpdate();
    }
  }

  void updateScreen() {
    if (name == "Empty") {
      obdFill(&obd[id], 0x00, 1);
      return;
    }
    obdFill(&obd[id], 0x00, 1);
    obdWriteString(&obd[id], 0, 0, 0, name, FONT, 0xff, 1);
  }

  void sendUpdate() {
    if (name == "Empty") return;
    Serial.println("changevolume " + String(replaceString(name, " ", "_")) + " " + String(volume));
  }


  char* replaceString(const char* str, const char* find, const char* replace) {
    int findLen = strlen(find);
    int replaceLen = strlen(replace);
    int resultLen = strlen(str) + (replaceLen - findLen) * countOccurrences(str, find) + 1;

    char* result = new char[resultLen];
    result[0] = '\0';  // Initialize the result string as an empty string

    const char* pos = str;
    const char* prevPos = str;

    while ((pos = strstr(pos, find)) != nullptr) {
      strncat(result, prevPos, pos - prevPos);  // Append the portion before 'find'
      strcat(result, replace);                  // Append the replacement string
      prevPos = pos + findLen;                  // Move the previous position after the found substring
      pos = prevPos;                            // Move the current position after the found substring
    }

    strcat(result, prevPos);  // Append the remaining portion of the original string

    return result;
  }

  int countOccurrences(const char* str, const char* find) {
    int count = 0;
    const char* pos = str;

    while ((pos = strstr(pos, find)) != nullptr) {
      count++;
      pos += strlen(find);  // Move the position after the found substring
    }

    return count;
  }
};

Fader faders[5] = {
  Fader(0, A10),
  Fader(1, A11),
  Fader(2, A12),
  Fader(3, A13),
  Fader(4, A14)
};


#define RESET_PIN -1
// let ss_oled figure out the display address
#define OLED_ADDR -1
// don't rotate the display
#define FLIP180 0
// don't invert the display
#define INVERT 0
// Bit-Bang the I2C bus
#define USE_HW_I2C 0

uint8_t scl_list[NUM_DISPLAYS] = { A0, A2, A4, A6, A8 };
uint8_t sda_list[NUM_DISPLAYS] = { A1, A3, A5, A7, A9 };


int scroll = 0;

int backButton = 12;
int backButtonState;
int lastBackButtonState = LOW;
unsigned long backLastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long backDebounceDelay = 50;    // the debounce time; increase if the output flickers

int forwardButton = 3;
int forwardButtonState;
int lastForwardButtonState = LOW;
unsigned long forwardLastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long forwardDebounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  pinMode(backButton, INPUT);
  pinMode(forwardButton, INPUT);

  int rc;

  for (int i = 0; i < NUM_DISPLAYS; i++) {
    rc = obdI2CInit(&obd[i], OLED_SIZE, OLED_ADDR, FLIP180, INVERT, 0, sda_list[i], scl_list[i], RESET_PIN, 100000L);

    Serial.print("Screen ");
    Serial.print(i);
    Serial.print(" started with code ");
    Serial.println(rc);

    if (rc != OLED_NOT_FOUND) {
      obdSetTextWrap(&obd[i], true);

      obdFill(&obd[i], 0x00, 1);

      // obdWriteString(&obd[i], 0, -1, -1, "Screen ", FONT, 0xff, 1);
      // char number[16];
      // itoa(i + 1, number, 10);
      // obdWriteString(&obd[i], 0, -1, -1, number, FONT, 0xff, 1);
    }
  }


  Serial.println("ready");
  while (!ready) {
    receiveData();
  }

  Serial.println("req processes " + String(scroll));
}

void loop() {
  for (int i = 0; i < sizeof(faders) / sizeof(faders[0]); i++) {
    faders[i].updateVolume();
  }

  int backReading = digitalRead(backButton);
  int forwardReading = digitalRead(forwardButton);

  if (backReading != lastBackButtonState) {
    // reset the debouncing timer
    backLastDebounceTime = millis();
  }

  if ((millis() - backLastDebounceTime) > backDebounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (backReading != backButtonState) {
      backButtonState = backReading;

      // only toggle the LED if the new button state is HIGH
      if (backButtonState == HIGH) {
        scroll--;
        if (scroll < 0) scroll = 0;
        Serial.print("Back button ");
        Serial.println(scroll);

        Serial.print("req processes ");
        Serial.println(scroll);
      }
    }
  }


  if (forwardReading != lastForwardButtonState) {
    // reset the debouncing timer
    forwardLastDebounceTime = millis();
  }

  if ((millis() - forwardLastDebounceTime) > forwardDebounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (forwardReading != forwardButtonState) {
      forwardButtonState = forwardReading;

      // only toggle the LED if the new button state is HIGH
      if (forwardButtonState == HIGH) {
        scroll++;
        Serial.print("Forward button ");
        Serial.println(scroll);

        Serial.print("req processes ");
        Serial.println(scroll);
      }
    }
  }

  receiveData();

  lastBackButtonState = backReading;
  lastForwardButtonState = forwardReading;
}

const char terminator = ';';
int dataIndex = 0;

void receiveData() {
  static String inputBuffer = ""; // Buffer to store received data

  // Check if there is data available to read
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();

    // If the terminator character ";" is not received, add the character to the buffer
    if (incomingChar != terminator) {
      inputBuffer += incomingChar;
    }
    else {
      // Process the received data when the terminator character is received
      // For example, you can print it to the Serial monitor
      Serial.println("Received data: " + inputBuffer);

      parseCommand(inputBuffer.c_str());

      // Clear the buffer for the next data
      inputBuffer = "";
    }
  }
}

void parseCommand(char* receivedData) {
  if (startsWith(receivedData, "response")) {
    char* args = receivedData + 9;

    char** processes = splitString(splitString(args, " /")[0], "|");
    int len = atoi(splitString(args, "/")[1]);

    for (int i = NUM_DISPLAYS; i > len - 1; i--) {
      faders[i].name = "Empty";
      faders[i].updateScreen();
    }

    if (processes != nullptr) {
      for (int i = 0; i < len; i++) {
        if (processes[i] == nullptr) {
          break;
        }

        char* process = processes[i];

        Serial.print(i);
        Serial.print(" ");
        Serial.print("Process: '");
        Serial.print(process);
        Serial.println("'");

        faders[i].name = process;
        faders[i].updateScreen();
      }
    }
  }
  else if (startsWith(receivedData, "scroll")) {
    char* args = receivedData + 7;
    scroll = atoi(args);
  }
  else if (startsWith(receivedData, "ready")) {
    ready = true;
  }
  else if (startsWith(receivedData, "update processes")) {
    Serial.print("req processes ");
    Serial.println(scroll);
  }
  else if (startsWith(receivedData, "disconnect")) {
    Serial.println("Backend disconnected");

    char* args = receivedData + 11;

    for (int i = 0; i < NUM_DISPLAYS; i++) {
      obdFill(&obd[i], 0x00, 1);
    }

    obdWriteString(&obd[0], 0, -1, -1, "Backend", FONT, 0xff, 1);
    obdWriteString(&obd[1], 0, 0, 0, "Disconnect", FONT, 0xff, 1);
    obdWriteString(&obd[1], 0, 0, 32, args, FONT, 0xff, 1);
    obdWriteString(&obd[2], 0, -1, -1, "Move fader 0 to mute to shut off", FONT_SMALL, 0xff, 1);

    while (faders[0].volume != 0) {
      faders[0].updateVolume();
    }

    Serial.end();

    for (int i = 0; i < NUM_DISPLAYS; i++) {
      obdFill(&obd[i], 0x00, 1);
    }
  }
  else if (startsWith(receivedData, "disconnectinfo")) {
    char* args = receivedData + 15;

    obdWriteString(&obd[3], 0, -1, -1, args, FONT_SMALL, 0xff, 1);
  }
  else if (startsWith(receivedData, "heartbeat")) {
    Serial.println("heartbeat");
  }

  memset(receivedData, 0, sizeof(receivedData) / sizeof(receivedData[0]));
}





bool startsWith(const String& str, const String& prefix) {
  // Check if the prefix is longer than the input string
  if (prefix.length() > str.length()) {
    return false;
  }

  // Compare each character of the prefix and input string
  for (size_t i = 0; i < prefix.length(); i++) {
    if (str.charAt(i) != prefix.charAt(i)) {
      return false;
    }
  }

  return true;
}

char** splitString(const char* input, char* delimiter) {
  char** parts = nullptr;
  char* token;
  int numParts = 0;

  // Make a copy of the input string since strtok modifies the original string
  char inputCopy[strlen(input) + 1];
  strcpy(inputCopy, input);

  // Count the number of parts first
  token = strtok(inputCopy, delimiter);
  while (token != nullptr) {
    numParts++;
    token = strtok(nullptr, delimiter);
  }

  // Allocate memory for the array of pointers to split strings
  parts = (char**)malloc(numParts * sizeof(char*));
  if (parts == nullptr) {
    return nullptr;  // Memory allocation failed
  }

  // Reset the strtok pointer to split the string again
  strcpy(inputCopy, input);
  token = strtok(inputCopy, delimiter);
  int partIndex = 0;

  // Split the string and store the pointers to the split strings
  while (token != nullptr && partIndex < numParts) {
    parts[partIndex] = strdup(token);  // Duplicate the token string
    partIndex++;
    token = strtok(nullptr, delimiter);
  }

  return parts;
}