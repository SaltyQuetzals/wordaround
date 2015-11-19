#include <LiquidCrystal.h>
#include <string.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;
//String strings[] = {"Gulliver's Travels", "Major Barbara", "Mrs. Dalloway", "Frankenstein", "Fences", "The Turn of the Screw", "Bleak House", "Doctor Faustus", "Catcher in the Rye", "A Tale of Two Cities", "Little Women", "To Kill a Mockingbird", "Oedipus Rex", "Pride and Prejudice", "Anthony and Cleopatra", "As You Like It", "Hamlet", "Henry IV, Parts I and II", "Henry V", "Julius Caesar"};// "King Lear", "Macbeth", "Merchant of Venice", "A Midsummer Night's Dream", "Much Ado About Nothing", "Othello", "Richard III", "Romeo and Juliet", "The Tempest", "Twelfth Night", "Winter’s Tale", "Antigone", "The Grapes of Wrath", "Jude the Obscure", "The Jungle", "Portrait of a Lady", "Rosencrantz and Guildenstern Are Dead", "A Raisin in the Sun", "All the Pretty Horses", "Anna Karenina", "Red Badge of Courage", "As I Lay Dying", "The Color Purple", "The Glass Menagerie", "Native Son", "Song of Solomon", "A Streetcar Named Desire", "The Crucible", "War and Peace", "One Day in the Life of Ivan Denisovich", "Of Mice and Men", "Old Man and the Sea", "All the King’s Men", "Ethan Frome", "The Road", "Lord Jim", "August Wilson", "Madame Bovary", "Medea", "The Merchant of Venice", "Invisible Man", "Wuthering Heights", "Great Expectations", "Heart of Darkness", "King Lear", "Crime and Punishment", "Lord of the Flies", "Jane Eyre", "The Adventures of Huckleberry Finn", "Moby Dick", "The Scarlet Letter", "Their Eyes Were Watching God", "The Awakening", "Catch-22", "Billy Budd", "The Great Gatsby", "Beloved", "Cat’s Cradle", "Slaughterhouse Five", "A Handmaid’s Tale", "A Farewell to Arms", "For Whom the Bell Tolls", "The Sun Also Rises", "The Tempest", "Things Fall Apart", "Who’s Afraid of Virginia Woolf?", "The Sound and the Fury", "A Doll’s House", "An Enemy of the People", "A Modest Proposal", "Death of a Salesman", "A Passage to India", "Candide", "Light in August", "One Day in the Life of Ivan Denisovich", "One Hundred Years of Solitude"};
char words[10][32];
int wordIndex = 0;
int prevWordIndex = -1;
char phrase[32];
int buttonPin = 9;
int buttonState = 0;
boolean registeredPress = false;
LiquidCrystal lcd(6, 7, 5, 8, 3, 2);

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  lcd.clear();
  lcd.print("Init SD card");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.print("No or invalid SD");
    // don't do anything more:
    return;
  }
  lcd.clear();
  lcd.print("SD card initialized");
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("words.txt");
  // if the file is available, write to it:
  if (dataFile) {
    int i = 0;
    int j = 0;
    while (dataFile.available() && i < sizeof(words) / sizeof(char[32])) {
      char c = dataFile.read();
      if (c != '\n') {
        words[i][j] = c;
        j++;
      } else {
        i++;
        j = 0;
      }
      Serial.println(words[i]);
    }
    dataFile.close();
  } else {
    lcd.clear();
    lcd.print("Failed to read words.txt");
  }
  lcd.clear();
  lcd.print("Randomizing words");
  const size_t n = sizeof(words) / sizeof(char);
  /*for (size_t i = 0; i < n - 1; i++) {
    size_t j = random(1, n - i);
    char* t = strings[i];
    strings[i] = strings[j];
    strings[j] = t;
  }*/
  lcd.clear();
  lcd.print("Done!");
}
void loop() {
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
  if (wordIndex != prevWordIndex) { 
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
      //Serial.println(phrase);
    }
  }
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH && !registeredPress) {
    // turn LED on:
    prevWordIndex = wordIndex;
    wordIndex++;
    registeredPress = true;
  } else {
    registeredPress = false;
  }
  if (wordIndex == sizeof(words) / sizeof(char[32]) || phrase[2] == '\0')  {
    wordIndex = 0;
  }

  delay(125);
  lcd.clear();
  //k++;

}
