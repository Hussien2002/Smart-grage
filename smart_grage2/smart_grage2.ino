#include <IRremote.hpp>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <IRremote.h>
#define ldr_pin A3    //ldr pin
#define ir1_pin 11    //enter ir
#define remotee A2    //remote ir
#define ir2_pin A1    //exit ir
#define servo_pin 10  //servo pin
#define ledPin 2      //led system
#define buzzer 12     //buzzer
#define flame A0    //flame sensor

int space;
int total = 5;
Servo serv;
int start_system = 0;
const byte ROWS = 4;
const byte COLS = 4;
IRrecv irrecv(remotee);
decode_results results;
int ir1_reading;
int ir2_reading;
int bool1 = 0;
int bool2 = 0;
bool bool3 = true;
bool bool4 = true;
bool flag_ldr = true;
bool flag_flame = true;
unsigned long start_time=0;
unsigned long end_time=0;
unsigned long car_time=0;
int  flag_timer = 0;
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

byte rowPins[ROWS] = { 9, 8, 7, 6 };
byte colPins[COLS] = { 5, 4, 3 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change the address (0x27 or 0x3F) if needed


char passcode1[] = "1234";  // كلمة المرور لتشغيل ال LED
char passcode2[] = "5678";  // كلمة المرور لإيقاف ال LED
char enteredPasscode[100] = "";

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(ir1_pin, INPUT);
  pinMode(ir2_pin, INPUT);
  pinMode(ldr_pin, INPUT);
  pinMode(flame, INPUT);
  pinMode(buzzer, OUTPUT);

  lcd.init();
  lcd.backlight();
 
  space = total;
  lcd.setCursor(0, 0);
  lcd.print(" Welcome to ");
  lcd.setCursor(0, 1);
  lcd.print("     future ");
  delay(2000);
  lcd.clear();
 lcd.print(" Enter PassWord ");

  Serial.begin(9600);
  irrecv.enableIRIn();  // Start the receiver

  serv.attach(servo_pin);
  serv.write(90);
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key == '#') {
      if (strcmp(enteredPasscode, passcode1) == 0) {
        lcd.clear();
        lcd.print("   open garage ");
        delay(1000);
        lcd.clear();
        clearEnteredPasscode();
        start_system = 1;


      } else if (strcmp(enteredPasscode, passcode2) == 0) {
        lcd.clear();
        lcd.print("   out of system ");
        delay(1000);
        lcd.clear();
        clearEnteredPasscode();
        start_system = 0;
      } else {
        lcd.clear();
        lcd.print("Incorrect!");
        delay(2000);
        lcd.clear();
        lcd.print("Enter Passcode:");
        clearEnteredPasscode();
      }
    } else {
      if (strlen(enteredPasscode) < 4) {
        lcd.setCursor(strlen(enteredPasscode), 1);
        lcd.print('*');
        enteredPasscode[strlen(enteredPasscode)] = key;
        enteredPasscode[strlen(enteredPasscode) + 1] = '\0';
      }
    }
  }
  //click third button to start system
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume();  // Receive the next value
    Serial.println(start_system);

    if (results.value == 0xBBCD5C52) {
      start_system = 1;
    }
    delay(100);
  }

  //start system
  if (start_system) {
int h =digitalRead(remotee);
 lcd.setCursor(0, 0);
  lcd.print("empty places: ");
  lcd.print(space);
  

    //smart servo
    ir1_reading = digitalRead(ir1_pin);
    ir2_reading = digitalRead(ir2_pin);
    if (ir1_reading == LOW && bool1 == 0) {
      if (space > 0) {
        bool1 = 1;
        if (bool2 == 0) {
          serv.write(0);
          space = space - 1;
        }
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("     future ");
        lcd.setCursor(0, 1);
        lcd.print("     is full ");
        delay(2000);
        lcd.clear();
      }
      delay(150);
    }
    if (ir2_reading == LOW && bool2 == 0) {
      bool2 = 1;
      if (bool1 == 0) {
        serv.write(0);
        space = space + 1;
      }
      delay(150);
    }
    if (bool1 == 1 && bool2 == 1) {
      delay(1000);
      serv.write(90);
      bool1 = 0, bool2 = 0;
    }

    //smart remote
    if (irrecv.decode(&results)) {
      Serial.println(results.value, HEX);
      irrecv.resume();  // Receive the next value
      Serial.println(start_system);
      //first button to toggle led
      if (results.value == 0xF18AEB1F) {
        bool3 = !bool3;
        if (!bool3) {
          digitalWrite(ledPin, HIGH);
          flag_ldr = false;
          flag_flame = false;
        } else {
          digitalWrite(ledPin, LOW);
          flag_ldr = true;
          flag_flame = true;
        }
      }

      // middle button to toggle servo

      else if (results.value == 0x62828C97) {
        bool4 = !bool4;
        if (!bool4) {
          serv.write(0);
        } else {
          serv.write(90);
        }
      }

      //third button to close system

      else if (results.value == 0xBBCD5C52) {
        start_system = 0;
      }
    }
    delay(100);



    //ldr sensor


    int value = analogRead(ldr_pin);
    Serial.print("value");

    Serial.println(value);

    if (flag_ldr) {
      if (value > 450) {
        delay(150);
        digitalWrite(ledPin, HIGH);  //The LED turns ON in Dark.

      } else {
        delay(150);
        digitalWrite(ledPin, LOW);  //The LED turns OFF in Light.
      }
      delay(150);
    }
    //flame sensor
    
    int flamesensorvalue = analogRead(flame);
    Serial.print("flamesensorvalue");
    Serial.println(flamesensorvalue);
    if (flag_flame) {
      if (flamesensorvalue < 300) {
        digitalWrite(buzzer, HIGH);
        lcd.clear();
        lcd.print("emergency");
        serv.write(0);

        digitalWrite(ledPin, HIGH);
        delay(300);
        digitalWrite(ledPin, LOW);
        delay(300);

      } else {
        digitalWrite(buzzer, LOW);
        //serv.write(90);

      }
    }
    //smart timer
if(h==0&&flag_timer==0){
    flag_timer=1;
    start_time=millis();
    Serial.print("time of car1 :");
    Serial.print(start_time/1000);
    Serial.println(" s");
  }
  else if (h==1&&flag_timer==1){
    flag_timer=0;
car_time=millis()-start_time;
Serial.print("time of car2 :");
    Serial.print(car_time/1000);
    Serial.println(" s");
    lcd.setCursor(0,1);
    lcd.print("price:");
    lcd.setCursor(0,1);
    lcd.print(car_time/1000*0.1);
    lcd.setCursor(3,1);
    lcd.print(" EG");
    delay(3000);
    lcd.clear();
    
  }

    
  }
}





void clearEnteredPasscode() {
  for (int i = 0; i < 4; i++) {
    enteredPasscode[i] = '\0';
  }
}