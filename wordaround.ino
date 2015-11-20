#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;
char words[5][32];
int wordIndex = 0;
int prevWordIndex = -1;
char phrase[32];
int buttonPin = 9;
int buttonState = 0;
boolean registeredPress = false;
LiquidCrystal lcd(6, 7, 5, 8, 3, 2);

void setup() {
  randomSeed(analogRead(10)); // pin 10 is free
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
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
  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.print("No or invalid SD");
    // don't do anything more:
    return;
  }
  lcd.clear();
  lcd.print("SD card");
  lcd.setCursor(0,1);
  lcd.print("initialized");
  reread();
}
void loop() {
  Serial.println(wordIndex);
  Serial.println(prevWordIndex);
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
    //Serial.println(phrase);
  }
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH && !registeredPress) {
    // turn LED on:
    wordIndex++;
    registeredPress = true;
  } else {
    registeredPress = false;
  }
  if (wordIndex == sizeof(words) / sizeof(char[32]) || phrase[0] == '\0')  {
    prevWordIndex = -1;
    wordIndex = 0;
    reread();
  }
  delay(125);

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
    dataFile.seek(
      (int)
      random(
        dataFile.size()-(
          35*sizeof(words) / sizeof(char[32])
        )
      )
    );
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
    lcd.print("Failed to read words.txt");
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
