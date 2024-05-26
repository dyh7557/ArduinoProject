#include <LiquidCrystal_I2C.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRc522DriverPinSimple.h>
#include <MFRC522Debug.h>

class LiquidCrystal_I2C lcd(0x27, 16, 2);
class MFRC522DriverPinSimple sda_pin(53);
class MFRC522DriverSPI driver {sda_pin};
class MFRC522 mfrc522 {driver};

String MASTER_CARD_UID {""};
const uint8_t SW_PIN {31U};
const uint8_t SERVO_PIN {8U};
bool button_state {false};
bool is_pushed {false};
uint8_t MODE {0U};
enum RGB {
  RED = 9U,
  GREEN,
  BLUE
};

String getId(){ // 입력된 RFID 카드의 UID 반환
  String Id = "";

  for(int i{0}; i < 4; i++)
  {
    Id += String(mfrc522.uid.uidByte[i], HEX);
  }
  Id.toUpperCase();
  mfrc522.PICC_HaltA();

  return Id;
}

void rgb_set(uint8_t red, uint8_t green, uint8_t blue){ // rgb_led 세팅용 함수
  analogWrite(RED, red);
  analogWrite(GREEN, green);
  analogWrite(BLUE, blue);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(SW_PIN, INPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  mfrc522.PCD_Init();
  lcd.init();
  lcd.home();
  lcd.backlight();

}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.clear();
  if(MODE == 0){ // 기본상태
    if(MASTER_CARD_UID == ""){
      lcd.print("EMPTY");
      rgb_set(0, 0, 0);
    }   
    else
    {
      lcd.print("LOCK");
      rgb_set(0, 150, 0);
    }
    if(mfrc522.PICC_IsNewCardPresent()){
      if(!mfrc522.PICC_ReadCardSerial()) return;
      rgb_set(0, 0, 0);
      if(getId() == MASTER_CARD_UID)
      {
        lcd.clear();
        lcd.print("OPEN!");
        rgb_set(0, 0, 150);
        for(int i {0}; i < 255; i += 5)
        {
          analogWrite(SERVO_PIN, i);
          delay(50UL);
        }
        delay(1000UL);
        MODE = 2;
        return;
      }
      else
      {
        lcd.clear();
        lcd.print("WRONG CARD!");
        rgb_set(150, 0, 0);
        delay(1000UL);
        return;
      }
    }

    is_pushed = digitalRead(SW_PIN);
    delay(10UL);
    if(is_pushed){
      if(!button_state){
        button_state = true;
        MODE = 1;
        return;
      }
    }
    else{
      button_state = false;
    }
  }
  
  else if(MODE == 1){ // 버튼을 눌렀을 때 (카드 등록 및 삭제)
    rgb_set(150, 150, 0);
    if(MASTER_CARD_UID == ""){
      lcd.print("REGISTER");
    }
    else {
      lcd.print("CARD RESET");
    }
    if(mfrc522.PICC_IsNewCardPresent()){
      if(!mfrc522.PICC_ReadCardSerial()) return;
      if(MASTER_CARD_UID == ""){ // 등록된 카드가 없을 때 -> 카드 등록
        MASTER_CARD_UID = getId();
        lcd.clear();
        lcd.print("SUCCESS!");
        rgb_set(0, 0, 150);
        delay(1000UL);
      }
      else { //등록되있는 카드가 있을 때 -> 카드 삭제
        if(MASTER_CARD_UID == getId()){
          MASTER_CARD_UID = "";
          lcd.clear();
          lcd.print("RESET COMPLETE");
          rgb_set(0, 0, 150);
          delay(1000UL);
        }

        else {
          lcd.clear();
          lcd.print("WRONG CARD!");
          rgb_set(150, 0, 0);
          delay(1000UL);
        }
      }
      MODE = 0;
      return;
    }

    is_pushed = digitalRead(SW_PIN);
    delay(10UL);
    if(is_pushed){
      if(!button_state){
        button_state = true;
        lcd.clear();
        lcd.print("CANCEL");
        delay(1000UL);
        rgb_set(150, 0, 0);
        MODE = 0;
        return;
      }
    }
    else{
      button_state = false;
    }
  }

  else if(MODE == 2){ //카드를 찍었을 때
    lcd.print("USE");
    rgb_set(150, 150, 150);
    
    is_pushed = digitalRead(SW_PIN);
    delay(10UL);
    if(is_pushed){
      if(!button_state){
        button_state = true;
        lcd.clear();
        lcd.print("CLOSE");
        for(int i {255}; i >= 0; i -= 5)
        {
          analogWrite(SERVO_PIN, i);
          delay(50UL);
        }
        delay(1000UL);
        MODE = 0;
        return;
      }
    }
    else{
      button_state = false;
    }
  }
}
