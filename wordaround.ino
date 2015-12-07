#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include "pitches.h"

// Configurable
const byte pointsToWin = 3; // point to win a game
const byte preloadWords = 5; // number of words to load into memory

// Don't change below
LiquidCrystal lcd(6, 7, 5, 8, 3, 2);
char words[preloadWords][32];
unsigned short wordIndex = 0;
short prevWordIndex = -1;
char phrase[32];
byte roundLength = 10; // set between 50 to 70 seconds

boolean registeredPress = false;
boolean pressed = false;
boolean buttonA, buttonB, buttonC;
boolean gaveTeamPoint = false;
byte pointTotal = 0;

short state = 0; // 0 = welcome, 1 = playing, 2 = select team, 3 = done
short prevState = -1;
unsigned long roundStart = 0;
byte Apoints, Bpoints;
byte redraws = 0;
unsigned long filePosition;
int notelength = 60;

byte smiley[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};
byte curvedbeta[8] = {16,16,10,13,9,9,22};
byte f[8] = {3,4,4,14,4,4,24};
byte g[8] = {31,31,4,31,31,14,17};
byte a[8] = {14,10,14,10,10,0,31};

// mission critical:
// http://onlinesequencer.net/155305
unsigned short action[] = {NOTE_D5,NOTE_D5,NOTE_D6,0,
                           NOTE_A5,0,0,NOTE_GS5,
                           0,NOTE_G5,0,NOTE_F5,
                           0,NOTE_D5,NOTE_F5,NOTE_G5,
                           NOTE_C5,NOTE_C5,NOTE_D6,0,
                           NOTE_A5,0,0,NOTE_GS5,
                           0,NOTE_G5,0,NOTE_F5,
                           0,NOTE_D5,NOTE_F5,NOTE_G5,
                           NOTE_B4,NOTE_B4,NOTE_D6,0,
                           NOTE_A5,0,0,NOTE_GS5,
                           0,NOTE_G5,0,NOTE_F5,
                           0,NOTE_D5,NOTE_F5,NOTE_G5,
                           NOTE_AS4,NOTE_AS4,NOTE_D6,0,
                           NOTE_A5,0,0,NOTE_GS5,
                           0,NOTE_G5,0,NOTE_F5,
                           0,NOTE_D5,NOTE_F5,NOTE_G5};
short megalovania = -1;
byte t = 0;
// http://onlinesequencer.net/123623
unsigned short victory[] = {NOTE_C5,NOTE_E5,NOTE_G5,NOTE_C6,
                            NOTE_G5,NOTE_E6,NOTE_G6,0,
                            0,NOTE_E6,0,0,
                            NOTE_GS4,NOTE_C5,NOTE_DS5,NOTE_GS5,
                            NOTE_C6,NOTE_DS6,NOTE_GS6,0,
                            0,NOTE_DS6,0,0,
                            NOTE_AS4,NOTE_D5,NOTE_F5,NOTE_AS5,
                            NOTE_D6,NOTE_F5,NOTE_AS6,NOTE_C6,
                            NOTE_D6,NOTE_F5,NOTE_AS6,0,
                            0,0,NOTE_AS6,NOTE_AS6,
                            NOTE_C7};
short victoryProgress = -1;
unsigned short startup[] = {NOTE_C5,NOTE_C5,0,NOTE_F5,
                            NOTE_FS5,NOTE_F5,NOTE_G5};
short startupProgress = -1;
unsigned short roundover[] = {NOTE_E5,NOTE_E5,NOTE_D5,NOTE_D5,
                              NOTE_D5,0,NOTE_D5,NOTE_D5};
short roundoverProgress = -1;

void setup() {
  // voodoo timer (http://letsmakerobots.com/node/28278) for ticker and music
  noInterrupts(); // disable all interrupts
  TCCR1A = TCCR1B = TCNT1 = 0;
  OCR1A = 16000000/256/8; // 8Hz
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS12); // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  interrupts(); // enable all interrupts
  randomSeed(analogRead(10)); // pin 10 is free, so seed random using that
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.createChar(0, smiley);
  lcd.createChar(1, curvedbeta);
  lcd.createChar(2, f);
  lcd.createChar(3, g);
  lcd.createChar(4, a);
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
    lcd.print("No or invalid");
    lcd.setCursor(0,1);
    lcd.print("SD on digital 4");
    // don't do anything more:
    return;
  }
  roundStart = millis();
  startupProgress = 0;
}

void loop() {
  buttonA = buttonB = buttonC = pressed = false; // reset
  buttonA = analogRead(3) > 500 ? true : false; // value are either 0 or 1023
  buttonB = analogRead(4) > 500 ? true : false; // so map > 500 to HIGH
  buttonC = analogRead(5) > 500 ? true : false; // and anything below to LOW
  if ((buttonA || buttonB || buttonC) && !registeredPress) {
    registeredPress = true; // press and hold triggers only once
    pressed = true;
  } else if (!(buttonA || buttonB || buttonC)) { // keyup
    registeredPress = false;
  }
  if (state != prevState || pressed || state == 1 || redraws > 0) { // avoid flashing LCD
    prevState = state;
    if (redraws > 0) redraws--;
    switch (state) {
      case 0: // welcome
        lcd.clear();
        lcd.print("WordAround v2");
        lcd.write(byte(1)); // little symbol
        lcd.setCursor(0,1);
        lcd.print("Press ");
        lcd.write(0b01111110); // ->
        lcd.print(" button!");
        if (buttonC && pressed) { // reset for game
          prevWordIndex = -1;
          reread();
          state = 1;
          roundStart = millis();
          Apoints = Bpoints = pointTotal = 0;
          gaveTeamPoint = false;
          megalovania = victoryProgress = roundoverProgress = -1;
          t = 0;
          roundLength = random(50, 70);
        } else if (buttonA && buttonB && pressed) {
          state = 4; // :^)
        }
        break;
      case 1: // playing
        if ((millis() - roundStart)/1000 > roundLength) {
          state = 2; // time's out!
          roundoverProgress = 0;
        }
        if (wordIndex != prevWordIndex) { // avoid flickering
          int currentLength = 0;
          lcd.clear();
          for (int i = 0; i < sizeof(phrase) / sizeof(char); i++) {
            phrase[i] = '\0'; // clear array
            lcd.print(' '); // and clear LCD
          }
          for (int i = 0; i < sizeof(phrase) / sizeof(char); i++) {
            if (words[wordIndex][i] == '\0') {
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
          // fetch new words if needed
          prevWordIndex = -1;
          wordIndex = 0;
          reread();
        }
        break;
      case 2: // select team that won
        if (gaveTeamPoint) {
          lcd.clear();
          lcd.write(0b01111111); // <-
          lcd.print("A:");
          lcd.print(Apoints);
          lcd.print(" vs. B:");
          lcd.print(Bpoints);
          lcd.write(0b01111110); // ->
          if (roundoverProgress == sizeof(roundover)/sizeof(short)+9) {
            if (buttonC) {
              prevWordIndex = -1;
              roundStart = millis();
              gaveTeamPoint = false;
              state = 1; // start another round
              reread(); // new words
              roundoverProgress = -1;
              redraws+=2;
            }
            lcd.setCursor(0,1);
            lcd.print("Press next!");
          }
        } else {
          if (buttonA || buttonB) {
            roundLength = random(50, 70);
            if (buttonA) { // award points
              Apoints++;
            } else if (buttonB) {
              Bpoints++;
            }
            pointTotal = Apoints + Bpoints;
            gaveTeamPoint = true;
            if (Apoints >= pointsToWin || Bpoints >= pointsToWin) {
              state = 3; // someone won
            }
            if (megalovania == -1 && Apoints >= pointsToWin-1 && Bpoints >= pointsToWin-1) {
              megalovania = 0; // it's getting close! is everybody determined enough?
            }
            redraws++;
          } 
          lcd.clear();
          lcd.print("Round over.");
          lcd.setCursor(0,1);
          lcd.write(0b01111111); // <-
          lcd.print("A:");
          lcd.print(Apoints);
          lcd.print(" vs. B:");
          lcd.print(Bpoints);
          lcd.write(0b01111110); // ->
        }
        break;
      case 3: // someone won!
        lcd.clear();
        lcd.print("Game over.");
        lcd.setCursor(0,1);
        if (Apoints >= pointsToWin) {
          lcd.print("Team A won!");
        } else if (Bpoints >= pointsToWin) {
          lcd.print("Team B won!");
        }
        if (buttonC) { // restart game
          state = 0;
        } else if (victoryProgress == -1) {
          victoryProgress = 0;
          delay(3000); // wait at least 3 seconds before continueing
        }
        lcd.print(' ');
        lcd.write(0b01111110);
        break;
      case 4: // easter egg, needs change
        lcd.clear();
        lcd.print("lots of love");
        lcd.setCursor(0,1);
        // waiting for more submissions...
        lcd.write(byte(2));
        lcd.write(byte(3));
        lcd.write(byte(4));
        if (buttonC) {
          state = 0;
        }
        break;
      default:
        // nothing here
      break;
    }
  }
  delay(5);
}

ISR(TIMER1_COMPA_vect) { // timer runs at 8Hz via magic
  if (startupProgress > -1 && startupProgress < sizeof(startup) / sizeof(short)) {
    // startup music
    playNote(startup[startupProgress], 100);
    startupProgress++;
  } else if (state == 1) {
    /* Ticks:
    until 50%: every second
    until 30%: every 500ms
    until 10%: every 100ms
    5 seconds left: every 50ms
    */
    if (megalovania == -1) {
      int elapsed = (millis() - roundStart)/10, // round off to 10s
          elapsedP = (millis() - roundStart)/10/roundLength; // as percentage
      if (elapsedP < 50 && t % 8 == 0) {
        playNote(NOTE_C4, 10);
      } else if (elapsedP >= 50 && elapsedP < 70 && t % 4 == 0) {
        playNote(NOTE_C4, 10);
      } else if (elapsedP >= 70 && elapsedP < 90 && t % 2 == 0) {
        playNote(NOTE_C4, 10);
      } else if (elapsedP >= 90) {
        playNote(NOTE_C4, 10);
      }
    } else {
      // if teams are close, give determination
      if (action[megalovania] > 0) playNote(action[megalovania], 50);
      megalovania++;
      if (megalovania == sizeof(action)/sizeof(short)) megalovania = 0;
    }
  } else if (state == 3 && victoryProgress > -1 &&
             victoryProgress < sizeof(victory)/sizeof(short)) {
    // victory music
    playNote(victory[victoryProgress], 50);
    victoryProgress++;
  } else if (state == 2 && roundoverProgress > -1 &&
             roundoverProgress < (sizeof(roundover)/sizeof(short)+9)) {
    // round over music
    if (roundoverProgress > 8) {
      playNote(roundover[roundoverProgress-9], 50);
    } else if (roundoverProgress == 0) {
      playNote(NOTE_C4, 1000);
    }
    roundoverProgress++;
    redraws++;
  }
  t++;
  if (t == 8) t = 0;
}

void reread() {
  File dataFile = SD.open("words.txt");
  // if the file is available, write to it:
  if (dataFile) {
    for (int i = 0; i < sizeof(words) / sizeof(char[32]); i++) {
      for (int j = 0; j < 32; j++) {
        words[i][j] = '\0';
      }
      filePosition = random(dataFile.size()); // random location
      char c;
      dataFile.seek(filePosition);
      while (dataFile.peek() != '\n' && filePosition > 0) {
        dataFile.seek(filePosition--); // find start of words
      }
      c = dataFile.read();
      while (dataFile.available() && (c == '\n' || c == '\r')) {
        c = dataFile.read(); // to first actual letter
      }
      if (dataFile.position() == 2) {
        dataFile.seek(dataFile.position()-2);
      } else {
        dataFile.seek(dataFile.position()-1);
      }
      int j = 0;
      while (dataFile.available() && j < 32) {
        c = dataFile.read();
        if (c == '\n') {
          j = 32; // stop reading
        } else {
          if (c != '\r') {
            words[i][j] = c;
            j++;
          }
        }
      }
      // skip words > 32 characters
      if (!(c == '\n' || c == '\r')) i--;
    }
    // only one file can be open at a time
    dataFile.close();
  } else {
    lcd.clear();
    lcd.print("Failed to read");
    lcd.setCursor(0,1);
    lcd.print("words.txt");
  }
}

void playNote(int note, int len) {
  tone(10, note, len);
}
