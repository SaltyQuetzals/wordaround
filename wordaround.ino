#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include "pitches.h"

// Constants
const int pointsToWin = 10;
const int roundLength = 10; // in seconds

LiquidCrystal lcd(6, 7, 5, 8, 3, 2);
char words[5][32];
int wordIndex = 0;
int prevWordIndex = -1;
char phrase[32];

int buttonAstate, buttonBstate, buttonCstate;
boolean registeredPress = false;
boolean pressed = false;
boolean buttonA, buttonB, buttonC;
boolean gaveTeamPoint = false;
int pointTotal = 0;

int state = 0; // 0 = start, 1 = playing, 2 = select team, 3 = done
int prevState = -1;
int roundStart = 0;
int Apoints, Bpoints;

byte smiley[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};

void setup() {
  randomSeed(analogRead(10)); // pin 10 is free
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.createChar(0, smiley);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  lcd.clear();
  lcd.print("Initializing");
  lcd.setCursor(0,1);
  lcd.print("SD card");
  // see if the card is present and can be initialized:
  if (!SD.begin(4)) { // SD on pin 4
    lcd.clear();
    lcd.print("No or invalid SD");
    // don't do anything more:
    return;
  }
  reread();
  lcd.clear();
  lcd.print("SD card");
  lcd.setCursor(0,1);
  lcd.print("initialized");
  roundStart = millis();
  Apoints = Bpoints = 0;
}

void loop() {
  buttonA = buttonB = buttonC = pressed = false;
  buttonA = analogRead(3) > 500 ? true : false; // value are either 0 or 1023
  buttonB = analogRead(4) > 500 ? true : false; // so map 1023 to HIGH
  buttonC = analogRead(5) > 500 ? true : false; // and anything below to LOW
  /*Serial.print(analogRead(3));
  Serial.print(" ");
  Serial.print(analogRead(4));
  Serial.print(" ");
  Serial.print(analogRead(5));
  Serial.print(" ");
  Serial.print(buttonA);
  Serial.print(" ");
  Serial.print(buttonB);
  Serial.print(" ");
  Serial.print(buttonC);
  Serial.println();*/
  if ((buttonA || buttonB || buttonC) && !registeredPress) {
    registeredPress = true;
    pressed = true;
  } else if (!(buttonA || buttonB || buttonC)) {
    registeredPress = false;
  }
  /*Serial.print(millis());
  Serial.print(" - ");
  Serial.print(roundStart);
  Serial.print(" = ");
  Serial.println(millis() - roundStart);*/
  if (state != prevState || pressed || state == 1 || state == 2) {
    prevState = state;
    switch (state) {
      case 0: // start
        lcd.clear();
        lcd.print("Press any button");
        lcd.setCursor(0,1);
        lcd.print("to start. ");
        //lcd.write(byte(0));
        lcd.write(0b01111110);
        if (pressed) {
          state = 1;
          roundStart = millis();
        }
        break;
      case 1: // playing
        if ((millis() - roundStart)/1000 > roundLength) {
          state = 2;
          playNote(NOTE_FS5, 1000);
        } else {
          /* Ticks:
          until 50%: every second
          until 30%: every 500ms
          until 10%: every 100ms
          5 seconds left: every 50ms
          */
          int elapsed = (millis() - roundStart)/10, // round off to 10s
              elapsedP = (millis() - roundStart)/10/roundLength; // as percentage
          if (elapsedP < 50 && elapsed % 100 == 0) {
            playNote(NOTE_C4, 20);
          } else if (elapsedP >= 50 && elapsedP < 70 && elapsed % 50 == 0) {
            playNote(NOTE_C4, 20);
          } else if (elapsedP >= 70 && elapsedP < 90 && elapsed % 20 == 0) {
            playNote(NOTE_C4, 20);
          } else if (elapsedP >= 90 && elapsed % 10 == 0) {
            playNote(NOTE_C4, 20);
          }
        }
        if (wordIndex != prevWordIndex) {
          int currentLength = 0;
          lcd.clear();
          for (int i = 0; i < sizeof(phrase) / sizeof(char); i++) {
            phrase[i] = '\0';
            lcd.print(' ');
          }
          for (int i = 0; i < sizeof(phrase) / sizeof(char); i++) {
            if (words[wordIndex][i] == '\0' || words[wordIndex][i] == '\r') {
              break;
            } else {
              phrase[i] = words[wordIndex][i];
              currentLength++;
            }
          }
          int line = 0;
          lcd.clear();
          for (int i = 0; i < currentLength; i++)  {
            if (i == 15 && phrase[i+1] != ' ' && line == 0) {
              int j = 0;
              for (int k = 15; k > 0; k--)  {
                if (phrase[k] == ' ')  {
                  j = k;
                  break;
                }
              }
              lcd.setCursor(j, line);
              for (int k = j; k < 16; k++)  {
                lcd.print(" ");
              }
              i = j;
              line = 1;
              lcd.setCursor(0, line);
            } else {
              lcd.print(phrase[i]);
            }
          }
          prevWordIndex = wordIndex;
        }
        if (pressed && buttonC) {
          // go to the next word:
          wordIndex++;
        }
        if (wordIndex == sizeof(words) / sizeof(char[32]) || phrase[0] == '\0')  {
          prevWordIndex = -1;
          wordIndex = 0;
          reread();
        }
        break;
      case 2: // select team
        if (true) {
          if (gaveTeamPoint) {
            if (buttonC) {
              prevWordIndex = -1;
              roundStart = millis();
              gaveTeamPoint = false;
              state = 1; // another round
            }
            lcd.clear();
            lcd.write(0b01111111);
            lcd.print("A:");
            lcd.print(Apoints);
            lcd.print(" vs. B:");
            lcd.print(Bpoints);
            lcd.write(0b01111110);
            lcd.setCursor(0,1);
            lcd.print("Round start!");
          } else {
            if (buttonA || buttonB) {
              if (buttonA) {
                Apoints++;
              } else if (buttonB) {
                Bpoints++;
              }
              pointTotal = Apoints + Bpoints;
              gaveTeamPoint = true;
              if (Apoints > pointsToWin || Bpoints > pointsToWin) {
                state = 3; // someone won
              }
            } 
            lcd.clear();
            lcd.print("Round over.");
            lcd.setCursor(0,1);
            lcd.write(0b01111111);
            lcd.print("A:");
            lcd.print(Apoints);
            lcd.print(" vs. B:");
            lcd.print(Bpoints);
            lcd.write(0b01111110);
          }
        }
        break;
      case 3: // someone won!
        lcd.clear();
        lcd.print("Game over.");
        lcd.setCursor(0,1);
        if (Apoints > pointsToWin) {
          lcd.print("Team A won!");
        } else if (Bpoints > pointsToWin) {
          lcd.print("Team B won!");
        }
        // play win sound
        if (buttonC) { // reset game
          roundStart = millis();
          Apoints = Bpoints = 0;
          pointTotal = 0;
          state = 1;
        }
      default:
        // nothing here
      break;
    }
  }
  delay(5);
}

void reread() {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("words.txt");
  boolean hasStarted = false;
  // if the file is available, write to it:
  if (dataFile) {
    for (int i = 0; i < sizeof(words) / sizeof(char[32]); i++) {
      for (int j = 0; j < 32; j++) {
        words[i][j] = '\0';
      }
    }
    int i = 0;
    int j = 0;
    int filePosition = (int)random(
      dataFile.size()-(
        35*sizeof(words) / sizeof(char[32])
      )
    );
    dataFile.seek(filePosition);
    while (dataFile.available() && dataFile.peek() != '\n' && filePosition > 0) {
      dataFile.seek(filePosition--);
    }
    while (dataFile.available() && i < sizeof(words) / sizeof(char[32])) {
      char c = dataFile.read();
      if (c != '\r') {
        if (!hasStarted) {
          if (c == '\n') {
            hasStarted = true;
          }
        } else {
          if (c != '\n') {
            words[i][j] = c;
            j++;
          } else {
            i++;
            j = 0;
          }
        }
      }
      //Serial.println(words[i]);
    }
    dataFile.close();
  } else {
    lcd.clear();
    lcd.print("Failed to read");
    lcd.setCursor(0,1);
    lcd.print("words.txt");
  }
  /*int n = sizeof(words) / sizeof(char[32]);
  for (int i = 0; i < n - 1; i++) {
    int j = random(1, n - i);
    char* t = words[i];
    words[i] = words[j];
    words[j] = t;
  }*/
  if (!hasStarted) {
    reread();
  }
}

void playNote(int note, int len) {
  tone(10, note, len);
  //delay(len);
  //noTone(10);
}

