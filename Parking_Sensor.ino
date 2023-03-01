
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <DHT.h>

#define LCD_PIN 13
#define BUTTON_PIN 3

#define DHTPIN 7
#define DHTTYPE DHT22

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
DHT dht(DHTPIN,DHTTYPE);

const int trigPin  = 2; // The pin for sending the trigger signal of the ultrasonic sensor, can be any digital pin
const int echoPin = 4;  // The pin for receiving the echo signal of the ultrasonic sensor, can be any digital pin

// The pin to which the red diode is connected, which signals that the distance 
// between the ultrasonic sensors and obstacles below a certain limit
const int ledPinRed =  12; 
int ledStateRed = LOW; 

// The pin to which the green diode is connected, which signals that the distance 
// between the ultrasonic sensors and obstacles above a certain limit
const int ledPinGreen =  8;
int ledStateGreen = LOW; 

float temp;     // variable temp into which the temperature value measured by the DHT22 sensor is entered
float hum;      // variable hum into which the air humidity value measured by the DHT22 sensor is entered
float soundsp;  // the speed of sound propagation, which is obtained after taking into account the values ​​of temperature and air humidity in m/s
float soundcm;  // sound propagation speed in cm/ms
float distance; // the distance between the ultrasonic sensor and the obstacle
float duration; // the time it takes for the sound to travel the distance from the trigger signal sending pin, bounce off the obstacle, and return to the echo signal receiving pin

volatile byte lcdState = LOW; // variable lcdState with the help of which we re-establish communication with the LCD display

// function definition that returns the value assigned to the shouldUpdateLc variable
bool updateLcd() {
  static unsigned long lastTime = 0;
  unsigned long time = millis();
  bool ret = false;
  if (time - lastTime > 500) {
    ret = true;
    lastTime = time;
  }
  return lcdState && ret;
}

void setup() {
  
  // initialize  serial  communication:
  Serial.begin(9600);

  pinMode(LCD_PIN, OUTPUT); // set the digital pin LCD_PIN as output
  pinMode(BUTTON_PIN, INPUT);

  // attachInterrupt(interrupt, function, mode) specifies a function to call when an external interrupt occurs
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), lightLcd, RISING);

  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  Serial.println("Pocetak 1.");

  dht.begin(); // establishing communication with the DHT22 sensor for measuring temperature and air humidity
}

void loop()
{
  static byte oldLcdState = lcdState;
  if (lcdState && !oldLcdState) {
    lcd.begin(20,4); // establishing communication with the LCD display
  }
  oldLcdState = lcdState;
  
  // measurement of temperature and air humidity using the DHT22 sensor
  temp=dht.readTemperature(); 
  hum=dht.readHumidity();

  // calculation of the value of the speed of sound taking into account the temperature and humidity of the air
  soundsp=341.4+0.606*temp+0.0124*hum;

  // converting sound speed from M/S to cm/ms
  soundcm=soundsp/10000;
  
  //  establish variables for duration  of  the ping,
  //  and the distance  result  in  centimeters:
  long duration, cm;
  //  The sensor  is  triggered by  a HIGH  pulse of  10  or  more  microseconds.
  //  Give  a short LOW pulse beforehand  to  ensure  a clean HIGH  pulse:

  // Since  you use this  pin on  the Arduino board to  send  a
  // trigger pulse to  the sonar sensor, configure the pin to  an  OUTPUT
  pinMode(trigPin,  OUTPUT);  
  
  digitalWrite(trigPin, LOW); //First reset the pin to  LOW.
  delayMicroseconds(2);       //Wait  for 2 micro seconds

  // Send the trigger pulse by  setting its voltage to  HIGH
  // HIGH  means its voltage will  be  5V  in  transistor-transistor logic (TTL).
  digitalWrite(trigPin, HIGH);
  
  delayMicroseconds(10); //Hold the pulse for 10  microsecond (you  can change  this)

  // Then  set it  back  to  LOW (0V)
  // Read  the signal  from  the sensor: a HIGH  pulse whose
  // duration  is  the time  (in microseconds) from  the sending
  // of  the ping  to  the reception of  its echo  off of  an  object.
  digitalWrite(trigPin, LOW); 

  // Since the voltage of  the echo  pin of  the sensor  has to  be  read
  // by  this  pin,  configure it  as  an  INPUT pin.
  pinMode(echoPin,  INPUT); 


  // get  the duration  this  echo  pin kept  its voltage at  HIGH  (5V)
  duration  = pulseIn(echoPin,  HIGH); 

  // the distance that the sound travels is equal to twice the distance of the ultrasonic sensor from the obstacle
  cm=(duration/2)*soundcm;
  bool shouldUpdateLcd = updateLcd();

  if (shouldUpdateLcd) {
    Serial.print("Distance: ");
    lcd.setCursor(2,0);
    lcd.print("");
    lcd.setCursor(3,2);
    lcd.print("Distance:");
    Serial.print("Distance: ");
  }

 // if the LCD display is off, turn off both the green and red diodes
 if (!lcdState) {
  ledStateRed = LOW;
  digitalWrite(ledPinRed, ledStateRed);

  ledStateGreen = LOW;
  digitalWrite(ledPinGreen, ledStateGreen);
 }

 // if the LCD display is on and if the distance from the obstacle is less than 100 cm, 
 // turn on the red diode and measure the distance to the obstacle

 else if(cm<10)
 {
  ledStateRed = HIGH;
  digitalWrite(ledPinRed, ledStateRed);

  ledStateGreen = LOW;
  digitalWrite(ledPinGreen, ledStateGreen);

  if (shouldUpdateLcd) {
    lcd.print("00");
    lcd.print(cm);
    lcd.setCursor(15,2);
    lcd.print("cm");
  

    Serial.print("00");
    Serial.print(cm);
    Serial.print("cm");
    Serial.println("");
    Serial.println("");
  }
 }

 else if(cm>10&&cm<100)  
 {
  ledStateRed = HIGH;
  digitalWrite(ledPinRed, ledStateRed);

  ledStateGreen = LOW;
  digitalWrite(ledPinGreen, ledStateGreen);

  if (shouldUpdateLcd) {
    lcd.print("0");
    lcd.print(cm);
    lcd.setCursor(15,2);
    lcd.print("cm");
  
    Serial.print("0");
    Serial.print(cm);
    Serial.print("cm");
    Serial.println("");
    Serial.println("");
  }
 }
 
  // if the LCD display is turned on and if the distance from the obstacle is greater than 100 cm, 
  // and less than 400 cm turn on the green diode and measure the distance to the obstacle
  else if(cm>=100&&cm<=400)
  {

  ledStateRed = LOW;
  digitalWrite(ledPinRed, ledStateRed);

  ledStateGreen = HIGH;
  digitalWrite(ledPinGreen, ledStateGreen);

  if (shouldUpdateLcd) {
    lcd.print(cm);
    lcd.setCursor(15,2);
    lcd.print("cm");
  
    Serial.print("0");
    Serial.print(cm);
    Serial.print("cm");
    Serial.println("");
    Serial.println("");
   }
  }
  
}

// function definition that changes the state of the LCD display
void lightLcd() {
  static unsigned long lastTime = 0;
  unsigned long curTime = millis();
  if (curTime - lastTime > 200) {
    lcdState = !lcdState;
    digitalWrite(LCD_PIN, lcdState);
  }
  lastTime = curTime;
}
