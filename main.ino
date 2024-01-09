#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h> 

#define RST_PIN     2
#define SS_PIN      10
#define InfraredPin 7
#define carSize     5
#define RFIDSize    4
#define LCDSize     16

LiquidCrystal_I2C lcd{0x27, 16, 2};
MFRC522 rfid(SS_PIN, RST_PIN);
Servo servo;

byte accessCard[carSize][RFIDSize];
int count = 0;
int index = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(InfraredPin, INPUT);
  lcd.init();
  lcd.backlight();
  SPI.begin();
  rfid.PCD_Init();
  servo.attach(6);
  servo.write(90);
}

void loop() {
  // put your main code here, to run repeatedly:

  // check parking space
  if ( isFull()){
    displayMessage("Full!Please wait", 0, 0);
    return;
  }

  // check detect
  if ( isObstacle()){

    // car is detected
    displayMessage("Welcome!"+ (carSize - count) +" spaces", 0, 0);
    displayMessage("Enter RFID.", 0, 1);

    // RFID card is close
    if ( isNewCardPresent() && readCardSerial()){

      // get UID of RFID
      byte* newCard = getRFIDCardUID();

      // RFID stop reading
      stopRead();

      // check RFID is store
      if(isStoredCard(newCard)){
        displayMessage("GoodBye!!", 0, 0);

        open()
        del(newCard);
      }
      else{
        displayMessage("Welcome!!", 0, 0);
        open();
        setNew(newCard);
      }
    } 
  }
  else{
    displayMessage("Hello!"+ (carSize - count) +" spaces", 0, 0);
    displayMessage("Parking lot.", index++, 1);
    if (index >= LCDSize) index = 0;
  }
  delay(1000);
}

bool isObstacle(){
  return !digitalRead(InfraredPin);
}

void displayMessage(const char *message, int index, int row){
  if (row == 0) lcd.clear();
  lcd.setCursor(index,row);
  lcd.print(message); 
}

bool isNewCardPresent() {
  return rfid.PICC_IsNewCardPresent();
}

bool readCardSerial() {
  return rfid.PICC_ReadCardSerial();
}

byte* getRFIDCardUID() {
  printCard(rfid.uid.uidByte);
  return rfid.uid.uidByte;
}

bool isStored(byte* newCard, byte* card){
  for ( byte i = 0; i < RFIDSize; i++){
    if ( card[i] != newCard[i]){
      return false;
    }
  }
  return true;
}

bool isStoredCard(byte* newCard){
  for ( int i = 0; i< count; i++){
    if (isStored( newCard, accessCard[i])) return true; 
  }
  return false;
}

void stopRead(){
  rfid.PICC_HaltA();
}

void setNew(byte* newCard){
  for ( int i = 0; i< RFIDSize; i++){
    accessCard[count][i] = newCard[i];
  }
  Serial.println("car in");
  count++;
}

void del(byte* card){
  for ( int i = 0; i< count; i++){
    if (isValid(accessCard[i], card)) {
      for ( int j = i+1; j< carSize; j++){
        for ( int k = 0; k< RFIDSize; k++){
          accessCard[j-1][k] = accessCard[j][k];
        }
      }
      break;
    } 
  }
  count--;
  Serial.println("car left");
}

void printCard(byte* card){
  for (int i = 0; i < RFIDSize; i++) {
    Serial.print(card[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void turn(int angle){
  servo.write(angle);
}

void open(){
  // RFID is valid, then open the door
  turn(0);

  // check detect
  while ( isObstacle()){
    delay(1000);
  }

  // car leave
  turn(90);
}

bool isFull(){
  return carSize <= count;
}
