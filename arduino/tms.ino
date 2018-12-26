#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "DHT.h"

#define VBRT_PIN     2
#define ODOR_PWM_PIN 3
#define ODOR_PIN     4
#define BT1_PIN      5
#define ECHO_PIN     6
#define TRIG_PIN     7
#define DHT_PIN      8
#define SERVO_PIN    9
#define HUMAN_PIN   10
#define SOUND_PIN   11
#define LED_PIN     13
#define CDS_PIN     A0
#define PRS_PIN     A1
#define ODOR_AN_PIN A2
#define CLOSING_PIN A3

#define BT_ON        LOW
#define BT_OFF       HIGH
#define DHT_TYPE     DHT11
#define ACM1602_ADDR 0x3F
#define PRESS_R      5.1        // 圧力スイッチの抵抗値[KΩ]
#define LCD_CREAR    "                "

struct CensorData {
  double  wave;
  double  temp;
  double  humi;
  int     cds;
  double  lt_temp;
  double  lt_humi;
  int     lt_cds;
  int     human;
  int     sound;
  double  pres;
  int     vibration;
  int     closing;
  int     odor;
  boolean first_set = true;
};

Servo knock_servo;
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(ACM1602_ADDR, 16, 2);
CensorData cd;

void setup() {
  Serial.begin(9600);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(BT1_PIN, INPUT_PULLUP);
  pinMode(VBRT_PIN, INPUT_PULLUP);
  pinMode(ODOR_PWM_PIN, OUTPUT);
  pinMode(ODOR_PIN, OUTPUT);

  knock_servo.attach(SERVO_PIN);

  dht.begin();

  lcd.init();
  lcd.backlight();
  disp_message("Welcome to the", "toilet heaven!");
}

int do_knock(int count) {
  for (int i = 0; i < count; i += 1) {
    for (int servo_pos = 89; servo_pos <= 150; servo_pos += 1) {
      knock_servo.write(servo_pos);
      delay(10);
    }
    knock_servo.write(90);
    delay(500);
  }

  for (int i = 0; i < 10; i += 1) {
    if (do_vibration_censor() == HIGH) {
      return HIGH;
    }
    delay(1000);
  }
  return LOW;
}

void do_dht_censor(float *result) {
  result[0] = dht.readHumidity();
  result[1] = dht.readTemperature();
  return result;
}

double do_wave_censor() {
  double duration = 0;
  double distance = 0;
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, HIGH );
  delayMicroseconds( 1000 );
  digitalWrite(TRIG_PIN, LOW );
  duration = pulseIn( ECHO_PIN, HIGH );
  if (duration > 0) {
    duration = duration / 2;
    distance = duration * 340 * 100 / 1000000;
  }
  return (distance);
}

int do_cds_censor() {
  return analogRead(CDS_PIN);
}

int do_human_censor() {
  return digitalRead(HUMAN_PIN);
}

int do_sound_censor() {
  return digitalRead(SOUND_PIN);
}

double do_press_censor() {
  double Vo, Rf, fg, kg;
  int ain = analogRead(PRS_PIN);
  Vo = ain * 5.0 / 1024;
  Rf = PRESS_R * Vo / (5.0 - Vo);
  fg = 880.79 / Rf + 47.96;
  kg = (fg / 4448) * 10;
  return kg;
}

int do_vibration_censor() {
  return digitalRead(VBRT_PIN);
}

int do_closing_censor() {
  int st = HIGH;
  if (analogRead(CLOSING_PIN) > 15) {
    st = LOW;
  }
  return st;
}

int do_odor_censor() {
  for (int i = 0; i <= 3; i++) {
    delay(242);
    digitalWrite(ODOR_PIN, HIGH);
    delay(8);
    digitalWrite(ODOR_PIN, LOW);
  }
  delay(237);
  digitalWrite(ODOR_PWM_PIN, HIGH);
  delay(3);
  int val = analogRead(ODOR_AN_PIN);
  delay(2);
  digitalWrite(ODOR_PWM_PIN, LOW);
  digitalWrite(ODOR_PIN, HIGH);
  delay(8);
  digitalWrite(ODOR_PIN, LOW);

  return val;
}

void disp_message(String message1, String message2) {
  lcd.setCursor(0, 0);
  lcd.print(LCD_CREAR);
  lcd.setCursor(0, 1);
  lcd.print(LCD_CREAR);
  lcd.setCursor(0, 0);
  lcd.print(message1);
  lcd.setCursor(0, 1);
  lcd.print(message2);
}

int split(String data, char delimiter, String *dst) {
  int index = 0;
  int arraySize = (sizeof(data) / sizeof((data)[0]));
  int datalength = data.length();
  for (int i = 0; i < datalength; i++) {
    char tmp = data.charAt(i);
    if ( tmp == delimiter ) {
      index++;
      if ( index > (arraySize - 1)) return -1;
    }
    else dst[index] += tmp;
  }
  return (index + 1);
}

void command_proc() {
  if (Serial.available() <= 0) {
    return;
  }

  String command_buf = Serial.readStringUntil('\n');
  if ( command_buf.startsWith("read")) {
    censor_read();
    serial_out();

  } else if ( command_buf.startsWith("knock")) {
    disp_message("Hello!", "Fucking man!!");
    String buf[3];
    split(command_buf, ',', buf);
    boolean result = do_knock(buf[1].toInt());
    String answer_buf = "knock_answer," + String(result);
    Serial.println(answer_buf);

    disp_message("Happy feces!", "");

  } else if ( command_buf.startsWith("provoke")) {
    disp_message("Hurry up!", "Mather fucker!!");
    String buf[3];
    split(command_buf, ',', buf);
    do_knock(buf[1].toInt());
  }
}

void censor_read() {
  if (!cd.first_set) {
    cd.lt_temp = cd.temp;
    cd.lt_humi = cd.humi;
    cd.lt_cds = cd.cds;
  }

  cd.wave = do_wave_censor();
  float ret[2];
  do_dht_censor(ret);
  cd.temp = ret[1];
  cd.humi = ret[0];
  cd.cds = do_cds_censor();
  if (cd.first_set) {
    cd.first_set = false;
    cd.lt_temp = cd.temp;
    cd.lt_humi = cd.humi;
    cd.lt_cds = cd.cds;
  }
  cd.human = do_human_censor();
  cd.sound = do_sound_censor();
  cd.pres = do_press_censor();
  cd.vibration = do_vibration_censor();
  cd.closing = do_closing_censor();
  cd.odor = do_odor_censor();
}

void serial_out() {
  String buf = "closing," + String(cd.closing) + ";human," + String(cd.human)
               + ";vibration," + String(cd.vibration) + ";sound," + String(cd.sound)
               + ";wave," + String(cd.wave) + ";temperature," + String(cd.temp)
               + ";lt_temperature," + String(cd.lt_temp) + ";humidity," + String(cd.humi)
               + ";lt_humidity," + String(cd.lt_humi) + ";brightness," + String(cd.cds)
               + ";lt_brightness," + String(cd.lt_cds) + ";odor," + String(cd.odor)
               + ";press," + String(cd.pres);
  Serial.println(buf);
}

void censor_test() {
  censor_read();
  Serial.print(cd.wave);
  Serial.println("cm");
  Serial.print(cd.temp);
  Serial.print("C / ");
  Serial.print(cd.humi);
  Serial.println("%");
  Serial.print("cds: ");
  Serial.println(cd.cds);
  Serial.print("human: ");
  Serial.println(cd.human);
  Serial.print("sound: ");
  Serial.println(cd.sound);
  Serial.print("press: ");
  Serial.print(cd.pres);
  Serial.println("kg");
  Serial.print("vibration: ");
  Serial.println(cd.vibration);
  Serial.print("door: ");
  Serial.println(cd.closing);
  Serial.print("odor: ");
  Serial.println(cd.odor);
}

void loop() {
  command_proc();

  int buttonState = 0;
  buttonState = digitalRead(BT1_PIN);
  if (buttonState == BT_ON) {
    censor_test();
    do_knock(2);
  }

  int close_st = do_closing_censor();
  if ( close_st == HIGH && cd.closing == LOW) {
    cd.closing = HIGH;
    disp_message("Welcome to the", "toilet heaven!");
  }
  if ( close_st == LOW && cd.closing == HIGH) {
    cd.closing = LOW;
    disp_message("Good bye!", "and see you.");
  }
}
