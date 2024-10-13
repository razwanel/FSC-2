#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Keypad.h>
#include  <Wire.h>

#define buzzerPin 10
#define lightsPin 11
#define gasLeakLEDPin 12

bool gasLeak;
bool intruder;
bool motion;

bool gasLeakEN = true;
bool intruderEN = true;
bool motionEN = true;

int pirState = LOW; 

String adminPassword = "123456";
String userPassword = "654321";

// LCD Setup
LiquidCrystal_I2C lcd(0x27,16,2);  

// keypad Setup
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

enum UserRole {
    NONE,   // No user authenticated
    ADMIN,  // Admin user authenticated
    USER    // Regular user authenticated
};

enum Alarm{
    LOCKED,
    UNLOCKED
};

Alarm state = LOCKED;

UserRole role = NONE;  // Global variable to store the authenticated role

UserRole Authenticate(){
    //DisplayLCD("A - Admin | B - User");
    lcd.setCursor(0,0);
    lcd.print("A - Admin");
    lcd.setCursor(0,1);
    lcd.print("B - User");
    String password = "";
    String stars = "";
    
    char c = ReadKeyChar();
            while(c == NO_KEY) {
                c = ReadKeyChar();
            }

    if(c == 'A') {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter password");
        for(int i = 0; i <6; i++){
            c = ReadKeyChar();
            while(c == NO_KEY) {
                c = ReadKeyChar();
            }
            password += c;
            stars += "*";
            lcd.clear();
            lcd.print(stars);
        }

        if(password == adminPassword) 
        {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Admin granted");
        return ADMIN;
        }
    }
    if(c == 'B'){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter password:");
        for(int i = 0; i <6; i++){
            c = ReadKeyChar();
            while(c == NO_KEY) {
                c = ReadKeyChar();
            }
            password += c;

            stars += "*";
            lcd.clear();
            lcd.print(stars);
        }

        if(password == userPassword) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("User granted");
          return USER;
          }
    }
    return NONE;
}

void setup() {
  // LCD Setup
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  // pins setup
  pinMode(lightsPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(gasLeakLEDPin, OUTPUT);

  // Serial com Setup
  Serial.begin(9600);
  role = Authenticate();
  
}

void loop() {

    char x = ReadFromMaster();



    switch (x) {
    case 'A':
      intruder = true;
      motion = true;
      gasLeak = true;
      break;
    case 'B':
      intruder = true;
      motion = true;
      gasLeak = false;
      break;
    case 'C':
      intruder = true;
      motion = false;
      gasLeak = true;
      break;
    case 'D':
      intruder = true;
      motion = false;
      gasLeak = false;
      break;
    case 'E':
      intruder = false;
      motion = true;
      gasLeak = true;
      break;
    case 'F':
      intruder = false;
      motion = true;
      gasLeak = false;
      break;
    case 'G':
      intruder = false;
      motion = false;
      gasLeak = true;
      break;
    case 'H':
      intruder = false;
      motion = false;
      gasLeak = false;
      break;
    }


    // Handle break-ins

    if(state == LOCKED)
    {
        if((motion && motionEN) | (intruder && intruderEN)) 
        {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Break-in alarm!");
        digitalWrite(buzzerPin, HIGH);
        }
        
    }

    
    // Motion sensor lights
    else if(state == UNLOCKED)
    {
        if(motion && motionEN) digitalWrite(lightsPin, HIGH);
        else digitalWrite(lightsPin, LOW);
    }

    if(gasLeak && gasLeakEN)
    {
        digitalWrite(buzzerPin, HIGH);
        digitalWrite(gasLeakLEDPin, HIGH);
    }
    else digitalWrite(gasLeakLEDPin, LOW);

    // Turn off buzzer:
    if( !gasLeak && ( (state == UNLOCKED) | (state == LOCKED && !motion && !intruder) ) )
    {
        digitalWrite(buzzerPin, LOW);
    }
    lcd.clear();
    if(motion && motionEN)
    {
    lcd.setCursor(0,0);
    lcd.print("motion detected" );
    }
    if(gasLeak && gasLeakEN){
      lcd.setCursor(0,1);
      lcd.print("gas leak" );
    }
    delay(100);

    // CONTROL PANEL if you press * (locked/ unlocked)
      char c = ReadKeyChar();
      int i = 0;
          while((c == NO_KEY) && (i < 100) ) {
              c = ReadKeyChar();
              delay(1);
              i++;
          }
      if((c == '*') && (role == ADMIN))
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("1 for LOCKED");
        lcd.setCursor(0,1);
        lcd.print("2 for UNLOCKED");

        c = ReadKeyChar();
        while(c == NO_KEY){
          c = ReadKeyChar();
        }
        if(c == '1'){
          state = LOCKED;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("LOCKED");
        }
        else if(c == '2'){
          state = UNLOCKED;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("UNLOCKED");
        }
      }
      
      if(c == 'A')
      {
        if(motionEN) printLCD("Motion sens ON",0);
        else printLCD("Motion sens OFF",0);
        printLCD("0 - OFF | 1 - ON",1);
        c = ReadKeyChar();
        while(c == NO_KEY) {
              c = ReadKeyChar();
          }
        if(c == '0') motionEN = false;
        if(c == '1') motionEN = true;
      }

      if(c == 'B')
      {
        if(intruderEN) printLCD("Intruder sens ON",0);
        else printLCD("Intruder sens OFF",0);
        printLCD("0 - OFF | 1 - ON",1);
        c = ReadKeyChar();
        while(c == NO_KEY) {
              c = ReadKeyChar();
          }
        if(c == '0') intruderEN = false;
        if(c == '1') intruderEN = true;
      }

      if(c == 'C')
      {
        if(gasLeakEN) printLCD("Gas sensor ON",0);
        else printLCD("Gas sensor OFF",0);
        printLCD("0 - OFF | 1 - ON",1);
        c = ReadKeyChar();
        while(c == NO_KEY) {
              c = ReadKeyChar();
          }
        if(c == '0') gasLeakEN = false;
        if(c == '1') gasLeakEN = true;
      }
}
void printLCD(String mesaj, int rand)
{
  lcd.setCursor(0,rand);
  lcd.print(mesaj);
}

char ReadFromMaster(){
    
    while(Serial.available() == 0){
        delay(1);
    }

    char c = Serial.read();
    if(!isControl(c)) return c;
    else return 'Z';
}

char ReadKeyChar(){
    return keypad.getKey();
}

void DisplayLCD(String mesaj){
        if(mesaj.length() > 32){ 
            while(mesaj.length() > 32){
                lcd.print(mesaj.substring(0,31));
                mesaj.remove(0,1);
                delay(200);
            }
        }
        lcd.print(mesaj);
    }