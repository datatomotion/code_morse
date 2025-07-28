/*
   =========================================
   Projet : Décodeur Morse tactile
   Matériel : ESP32 Vroom, LCD 16x2 I2C, capteur tactile HW-136
   Auteur : Data To Motion
   =========================================
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int touchPin = 4;

// Seuils en millisecondes
const unsigned long dotThreshold = 250;     // < 250 ms = point
const unsigned long deleteThreshold = 1000; // ≥ 1 sec = effacement
const unsigned long letterPause = 600;      // Pause = fin de lettre

String morseBuffer = "";
unsigned long pressStart = 0;
bool isPressed = false;
unsigned long lastRelease = 0;

// Position curseur LCD
int cursorCol = 0;
int cursorRow = 0;

struct MorseCode {
  const char* code;
  char letter;
};

MorseCode morseTable[] = {
  {".-", 'A'},   {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
  {".", 'E'},    {"..-.", 'F'}, {"--.", 'G'},  {"....", 'H'},
  {"..", 'I'},   {".---", 'J'}, {"-.-", 'K'},  {".-..", 'L'},
  {"--", 'M'},   {"-.", 'N'},   {"---", 'O'},  {".--.", 'P'},
  {"--.-", 'Q'}, {".-.", 'R'},  {"...", 'S'},  {"-", 'T'},
  {"..-", 'U'},  {"...-", 'V'}, {".--", 'W'},  {"-..-", 'X'},
  {"-.--", 'Y'}, {"--..", 'Z'},
  {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'},
  {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'},
  {"---..", '8'}, {"----.", '9'}
};

char decodeMorse(String code) {
  for (int i = 0; i < sizeof(morseTable) / sizeof(MorseCode); i++) {
    if (code == morseTable[i].code) {
      return morseTable[i].letter;
    }
  }
  return '?';
}

// Affichage d’un caractère avec gestion du curseur
void printLetter(char c) {
  lcd.setCursor(cursorCol, cursorRow);
  lcd.print(c);

  cursorCol++;
  if (cursorCol >= 16) {
    cursorCol = 0;
    cursorRow++;
    if (cursorRow >= 2) {
      cursorRow = 0;
    }
  }
}

void clearScreen() {
  lcd.clear();
  cursorCol = 0;
  cursorRow = 0;
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Data To Motion");
  delay(1500);
  lcd.clear();

  pinMode(touchPin, INPUT);
}

void loop() {
  int touchState = digitalRead(touchPin);

  // Début d’appui
  if (touchState == HIGH && !isPressed) {
    isPressed = true;
    pressStart = millis();
  }

  // Relâchement
  if (touchState == LOW && isPressed) {
    isPressed = false;
    unsigned long pressDuration = millis() - pressStart;

    // Effacer si appui long (≥ 1 seconde)
    if (pressDuration >= deleteThreshold) {
      clearScreen();
    }
    // Sinon enregistrer dot ou dash
    else {
      if (pressDuration < dotThreshold) {
        morseBuffer += ".";
      } else {
        morseBuffer += "-";
      }
    }

    lastRelease = millis();
  }

  // Fin de lettre après pause
  if (!isPressed && morseBuffer.length() > 0) {
    if (millis() - lastRelease > letterPause) {
      // 1 point seul = espace
      if (morseBuffer == "......") {
        printLetter(' ');
      } else {
        char decoded = decodeMorse(morseBuffer);
        printLetter(decoded);
      }
      morseBuffer = "";
    }
  }
}
