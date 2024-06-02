#include <DHT.h>
#include <LCDIC2.h>         //LCD패널 라이브러리
#include <Stepper.h>        //스텝모터 단일제어 라이브러리

#define DHTPIN 14
#define DHTTYPE DHT22 

LCDIC2 lcd(0x27, 16, 2); //LCD패널 

DHT dht(DHTPIN, DHTTYPE);
//---------------전역 변수---------------

// 현재 스텝을 저장하기 위한 변수
int currentStep = 0;

// 현재 발판 높이를 저장하기 위한 변수
int currentLevel = 0;


bool didFinishedRotation = false;

// 현재 온도를 저장하기 위한 변수
float currentTemp = 0.0;

// 정지 요청이 있는지 확인하는 변수
bool isAutoStopRequested = false;
bool isFastStopRequested = false;
bool isDeoStopRequested = false;
bool isDryStopRequested = false;

// 스텝모터 회전 스텝 수
const int stepsPerRevolution = 200;  

//-----------------------------------------------------------------------

// 냉풍기  1
const int ENA_COOLER = 7; 
const int IN1_COOLER = 6; 
const int IN2_COOLER = 5;

// 냉풍기 2
const int ENB_COOLER = 2;
const int IN3_COOLER = 4;
const int IN4_COOLER = 3;

// 열풍기 1
const int ENA_HEATER = 13;
const int IN1_HEATER = 12;
const int IN2_HEATER = 11;

// 열풍기 2
const int ENB_HEATER = 8;
const int IN3_HEATER = 9;
const int IN4_HEATER = 10;

// 환풍기
const int ENA_VENTILATOR = 40;
const int IN1_VENTILATOR = 41;
const int IN2_VENTILATOR = 42;

// 플라즈마 모듈
const int PLASMA_MODULE = 16;

// 축 회전용 스텝모터
Stepper shaftRotator(stepsPerRevolution, 50, 51, 52, 53); 

// 발판 상하 운동용 스텝모터
Stepper upDownMover1(stepsPerRevolution, 36, 37, 38, 39);

// 문 개폐용 DC모터
const int ENA_DOOR = 46;
const int IN1_DOOR = 47;
const int IN2_DOOR = 48;


//------------------------------------기본 셋팅 함수------------------------------------

// 냉풍기 설정 함수
void settingCooler() {
  pinMode(ENA_COOLER, OUTPUT);
  pinMode(IN1_COOLER, OUTPUT);
  pinMode(IN2_COOLER, OUTPUT);
  pinMode(IN3_COOLER, OUTPUT);
  pinMode(IN4_COOLER, OUTPUT);
  pinMode(ENB_COOLER, OUTPUT);

  digitalWrite(IN1_COOLER, LOW);
  digitalWrite(IN2_COOLER, LOW);
  digitalWrite(IN3_COOLER, LOW);
  digitalWrite(IN4_COOLER, LOW);

  analogWrite(ENA_COOLER, 0);
  analogWrite(ENB_COOLER, 0);
}

// 열풍기 설정 함수
void settingHeater() { 
  pinMode(ENA_HEATER, OUTPUT);
  pinMode(IN1_HEATER, OUTPUT);
  pinMode(IN2_HEATER, OUTPUT);
  pinMode(IN3_HEATER, OUTPUT);
  pinMode(IN4_HEATER, OUTPUT);
  pinMode(ENB_HEATER, OUTPUT);

  digitalWrite(IN1_HEATER, LOW);
  digitalWrite(IN2_HEATER, LOW);
  digitalWrite(IN3_HEATER, LOW);
  digitalWrite(IN4_HEATER, LOW);

  analogWrite(ENA_HEATER, 0);
  analogWrite(ENB_HEATER, 0);
}

// 환풍기 설정 함수
void settingVentilator() {
  pinMode(ENA_VENTILATOR, OUTPUT);
  pinMode(IN1_VENTILATOR, OUTPUT);
  pinMode(IN2_VENTILATOR, OUTPUT);

  analogWrite(ENA_VENTILATOR, 0);
  digitalWrite(IN1_VENTILATOR, LOW);
  digitalWrite(IN2_VENTILATOR, LOW);
}

// 플라즈마모듈 설정 함수
void settingPlasmaModule() {
  pinMode(PLASMA_MODULE, OUTPUT);
}

// 모터 설정 함수
void settingMotor() {
  pinMode(ENA_DOOR, OUTPUT);
  pinMode(IN1_DOOR, OUTPUT);
  pinMode(IN2_DOOR, OUTPUT);

  digitalWrite(IN1_DOOR, LOW);
  digitalWrite(IN2_DOOR, LOW);
  analogWrite(ENA_DOOR, 0);
}

//----------------------------------------정지 함수--------------------------------------

// 정지 함수
void stopAll() {
  stopCooler();
  stopHeater();
  stopVentilator();
  stopPlasmaModule();
}

// 자동 모드 정지
void stopAutoMode() {
  isAutoStopRequested = true;
  stopAll();
  Serial1.println("AUTOSTOP:0");
  Serial.println("Debug : Stop AutoMode");
}

// 쾌속 모드 정지
void stopFastMode() {
  isFastStopRequested = true;
  stopAll();
  Serial1.println("FASTSTOP:0");
  Serial.println("Debug : Stop FastMode");
}

// 탈취 모드 정지
void stopDeodorizeMode() {
  isDeoStopRequested = true;
  stopAll();
  Serial.println("DEOSTOP:0");
  Serial.println("Debug : Stop DeodorizeMode");
}

// 건조 모드 정지
void stopDryMode() {
  isDryStopRequested = true;
  stopAll();
  Serial1.println("DRYSTOP:0");
  Serial.println("Debug : Stop DryMode");
}

// 냉풍기 정지
void stopCooler() {
  digitalWrite(IN1_COOLER, LOW);
  digitalWrite(IN2_COOLER, LOW);

  digitalWrite(IN3_COOLER, LOW);
  digitalWrite(IN4_COOLER, LOW);
}

// 열풍기 정지
void stopHeater() {
  digitalWrite(IN1_HEATER, LOW); 
  digitalWrite(IN2_HEATER, LOW);

  digitalWrite(IN3_HEATER, LOW); 
  digitalWrite(IN4_HEATER, LOW);
}

// 환풍기 정지
void stopVentilator() {
  digitalWrite(IN1_VENTILATOR, LOW);
  digitalWrite(IN2_VENTILATOR, LOW);
  analogWrite(ENA_VENTILATOR, 0);
}

// 플라즈마모듈 정지
void stopPlasmaModule() {
  digitalWrite(PLASMA_MODULE, LOW);
}

// 냉풍기 ON 함수
void runCooler() {
  analogWrite(ENA_COOLER, 255); 
  digitalWrite(IN1_COOLER, LOW); 
  digitalWrite(IN2_COOLER, HIGH);
  
  analogWrite(ENB_COOLER, 255); 
  digitalWrite(IN3_COOLER, LOW); 
  digitalWrite(IN4_COOLER, HIGH);
}

// 열풍기 ON 함수
void runHeater() {
  analogWrite(ENA_HEATER, 255); 
  digitalWrite(IN1_HEATER, HIGH);
  digitalWrite(IN2_HEATER, LOW);

  analogWrite(ENB_HEATER, 255);
  digitalWrite(IN3_HEATER, LOW); 
  digitalWrite(IN4_HEATER, HIGH);

}

// 환풍기 ON 함수
void runVentilator() {
  analogWrite(ENA_VENTILATOR, 255);
  digitalWrite(IN1_VENTILATOR, HIGH);
  digitalWrite(IN2_VENTILATOR, LOW);
}

// 플라즈마모듈 실행 함수
void runPlasmaModule() {
  digitalWrite(PLASMA_MODULE, HIGH);
}

// 온습도 모듈로부터 온습도를 읽어오는 함수
void readDHT() {
    float temp = dht.readTemperature();  // 온도 읽기
    float humidity = dht.readHumidity();  // 습도 읽기

    currentTemp = temp;

    String data = "DHT:" + String(temp) + "," + "HUMID:" + String(humidity);

    Serial.println(data);
    lcd.setCursor(2, 0);
    lcd.print("TEMP-" + String(temp) + "C");
    lcd.setCursor(2, 1);
    lcd.print("HUMID-" + String(round(humidity)) + "%");
}

// 문 열림 (실험으로 모터 속도와 시간 재설정 필요)
void openDoor() {
  analogWrite(ENA_DOOR, 80);
  Serial.println("Debug : UP");
  digitalWrite(IN1_DOOR, LOW);
  digitalWrite(IN2_DOOR, HIGH);

  delay(400);

  analogWrite(ENA_DOOR, 0);
  digitalWrite(IN1_DOOR, LOW);
  digitalWrite(IN2_DOOR, LOW);

  Serial.println("Debug : Did Finished Open Door");
  Serial1.println("READY_TO_CLOSE:0");
}

// 문 닫힘
void closeDoor() {
  Serial.println("Debug : DOWN");
  analogWrite(ENA_DOOR, 40);
  digitalWrite(IN1_DOOR, HIGH);
  digitalWrite(IN2_DOOR, LOW);

  delay(500);

  analogWrite(ENA_DOOR, 0);
  digitalWrite(IN1_DOOR, LOW);
  digitalWrite(IN2_DOOR, LOW);

  Serial.println("Debug : Did Finished Close Door");
  Serial1.println("CLOSE:0");

}

// 발판 상하 이동 함수
void upDown() {
    upDownMover1.setSpeed(40);

    if (currentLevel == 0) {   // 현재 1단 발판 위치가 1단이라면
      upDownMover1.step(-180); // 위치 바꾸기
      currentLevel = 1;
    }
    else {                     // 현재 1단 발판 위치가 2단이라면
      upDownMover1.step(+180); // 위치 다시 바꾸기
      currentLevel = 0;
    }
}

//-----------------------------------------------------------



//----------------------------------------제어 함수--------------------------------------

// 자동 모드 실행
void autoMode() {
  Serial1.println("AUTOSTART:0"); // 블루투스로 자동모드 시작 알림
  Serial.println("Debug : Auto Start.."); // 디버깅
  isAutoStopRequested = false; // 자동모드 정지요청 == 거짓

  shaftRotator.setSpeed(10); // 스텝 모터 속도

  runHeater();
  runCooler();
  runPlasmaModule();

  // 0 -> -180도
  for (int step = 0; step >= -432; step--) {
    shaftRotator.step(-1);        // 축 회전용 모터를 1스텝씩 회전
    currentStep = step;           // 현재 스텝 변수에 할당
    Serial.println(currentStep);  // 현재 스텝 시리얼 모니터를 통해 확인
    if (currentStep % 108 == 0 && (isAutoStopRequested)) { 
      // 0, 60, 120, 180, -60, -120 중 하나의 값이면서 정지요청이 있을 경우에 정지
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  stopHeater();

  if (isAutoStopRequested) {
    return;
  }

  // -180 -> 0도
  for (int step = -432; step <= 0; step++) {
    shaftRotator.step(1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isAutoStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  stopCooler();
  stopPlasmaModule();
  runVentilator();

  upDown(); // 축이 반바퀴 회전했을 때 상하 위치 이동

  if (isAutoStopRequested) {
    return;
  }

  // 0 -> 180도
  for (int step = 0; step <= 432; step++) {
    shaftRotator.step(1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isAutoStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  stopVentilator();

  // 180 -> 0도
  for (int step = 432; step >= 0; step--) {
    shaftRotator.step(-1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isAutoStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  upDown();

  Serial.println("Debug : AutoMode Finished");
}

// 쾌속 모드 실행
void fastMode() {
  Serial1.println("FASTSTART:0"); // 블루투스로 자동모드 시작 알림
  Serial.println("Debug : Fast Start.."); // 디버깅
  isFastStopRequested = false; // 자동모드 정지요청 == 거짓

  shaftRotator.setSpeed(10); // 스텝 모터 속도

  runHeater();
  runCooler();
  runPlasmaModule();

  // 0 -> -180도
  for (int step = 0; step >= -432; step--) {
    shaftRotator.step(-1);        // 축 회전용 모터를 1스텝씩 회전
    currentStep = step;           // 현재 스텝 변수에 할당
    Serial.println(currentStep);  // 현재 스텝 시리얼 모니터를 통해 확인
    if (currentStep % 108 == 0 && (isFastStopRequested)) { 
      // 0, 60, 120, 180, -60, -120 중 하나의 값이면서 정지요청이 있을 경우에 정지
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  if (isFastStopRequested) {
    return;
  }

  // -180 -> 0도
  for (int step = -432; step <= 0; step++) {
    shaftRotator.step(1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isFastStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  stopCooler();
  stopPlasmaModule();
  stopHeater();
  runVentilator();

  upDown(); // 축이 반바퀴 회전했을 때 상하 위치 이동

  if (isFastStopRequested) {
    return;
  }

  // 0 -> 180도
  for (int step = 0; step <= 432; step++) {
    shaftRotator.step(1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isFastStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  stopVentilator();

  // 180 -> 0도
  for (int step = 432; step >= 0; step--) {
    shaftRotator.step(-1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isFastStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  upDown();
  Serial.println("Debug : FastMode Finished");
}

// 탈취 모드 실행
void deodorizeMode() {
  Serial1.println("DEOSTART:0"); // 블루투스로 자동모드 시작 알림
  Serial.println("Debug : Deodorize Start.."); // 디버깅
  isDeoStopRequested = false; // 자동모드 정지요청 == 거짓

  shaftRotator.setSpeed(10); // 스텝 모터 속도

  runCooler();
  runPlasmaModule();

  // 0 -> -180도
  for (int step = 0; step >= -432; step--) {
    shaftRotator.step(-1);        // 축 회전용 모터를 1스텝씩 회전
    currentStep = step;           // 현재 스텝 변수에 할당
      
    if (currentStep % 108 == 0 && (isDeoStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  if (isDeoStopRequested) {
    return;
  }

  // -180 -> 0도
  for (int step = -432; step <= 0; step++) {
    shaftRotator.step(1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isDeoStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  stopCooler();
  stopPlasmaModule();
  runVentilator();

  upDown();

  if (isDeoStopRequested) {
    return;
  }

  // 0 -> 180도
  for (int step = 0; step <= 432; step++) {
    shaftRotator.step(1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isDeoStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  stopVentilator();

  // 180 -> 0도
  for (int step = 432; step >= 0; step--) {
    shaftRotator.step(-1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isDeoStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  upDown();

  // 디버깅용
  Serial.println("Debug : DeodorizeMode Finished");
}

// 건조 모드 실행
void dryMode() {
  Serial1.println("DRYSTART:0"); // 블루투스로 자동모드 시작 알림
  Serial.println("Debug : Dry Start.."); // 디버깅
  isDryStopRequested = false; // 자동모드 정지요청 == 거짓

  shaftRotator.setSpeed(10); // 스텝 모터 속도

  runHeater();

  // 0 -> -180도
  for (int step = 0; step >= -432; step--) {
    shaftRotator.step(-1);        // 축 회전용 모터를 1스텝씩 회전
    currentStep = step;           // 현재 스텝 변수에 할당
      
    if (currentStep % 108 == 0 && (isDryStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  if (isDryStopRequested) {
    return;
  }

  // -180 -> 0도
  for (int step = -432; step <= 0; step++) {
    shaftRotator.step(1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isDryStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  upDown();

  if (isDeoStopRequested) {
    return;
  }

  // 0 -> 180도
  for (int step = 0; step <= 432; step++) {
    shaftRotator.step(1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isDryStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  stopHeater();
  runVentilator();

  // 180 -> 0도
  for (int step = 432; step >= 0; step--) {
    shaftRotator.step(-1);
    currentStep = step;
      
    if (currentStep % 108 == 0 && (isDeoStopRequested)) {
      Serial.println("Debug : Current Step : " + currentStep);
      return;
    }
  }

  stopVentilator();

  upDown();

  // 디버깅용
  Serial.println("Debug : DeoMode Finished");
}

// 신발 수납&추출
void shoesInOut(int shoesNum) {

  /*
  신발 넘버 1 => 0     :  2단
  신발 넘버 2 => -120  :  2단
  신발 넘버 3 => +120  :  2단

  신발 넘버 4 => -60   :  1단
  신발 넘버 5 => +60   :  1단
  신발 넘버 6 => -180  :  1단
  */

  int shoesStep = 0;
  
  int elevateStep = 0;

  switch (shoesNum) {
    case 1:
      shoesStep = 0;
      elevateStep = 0;
    case 2:
      shoesStep = -216;
      elevateStep = 0;
    case 3:
      shoesStep = +216;
      elevateStep = 0;
    case 4:
      shoesStep = -108;
      elevateStep = 1;
    case 5:
      shoesStep = +108;
      elevateStep = 1;
    case 6:
      shoesStep = -324;
      elevateStep = 1;
    default:
      break;
  }

  int stepsToRotate = shoesStep - currentStep;

  shaftRotator.step(-stepsToRotate);

  currentStep = shoesStep;

  if (currentLevel == 0 && elevateStep == 1) {
  // 2단 1단 위치가 바뀌어야 하는 상황
    upDownMover1.step(-180);
    currentLevel = 1;

  } else if (currentLevel == 1 && elevateStep == 0) {
    // 2단 1단 위치가 바뀌어야 하는 상황
    upDownMover1.step(180);
    currentLevel = 0;
  }

  // 발판이 문 앞에 위치되면 문 열 준비가 완료되었다는 메세지를 블루투스를 통해 보내기
  Serial1.println("READY_TO_OPEN:0");
}


void btCommunication() {
  if (Serial1.available()) {
    Serial.println("Debug : Bluetooth Available");
    String command = Serial1.readString();

    if (command.equals("STOP")) {
      stopAll();
    } else if (command.equals("DHT:READ")) {
      readDHT();
    } else if (command.equals("AUTO:START")) {
      autoMode();
    } else if (command.equals("AUTO:STOP")) {
      stopAutoMode();
    } else if (command.equals("FASTAUTO:START")) {
      fastMode();
    } else if (command.equals("FASTAUTO:STOP")) {
      stopFastMode();
    } else if (command.equals("DEODORIZE:START")) {
      deodorizeMode();
    } else if (command.equals("DEODORIZE:STOP")) {    
      stopDeodorizeMode();
    } else if (command.equals("DRY:START")) {  
      dryMode();
    } else if (command.equals("DRY:STOP")) {
      stopDryMode();
    } else if (command.equals("DOOR:OPEN")) {
      openDoor();
    } else if (command.equals("DOOR:CLOSE")) {
      closeDoor();
    } else if (command.equals("SHOESIN:1")) {
      shoesInOut(1);
    } else if (command.equals("SHOESIN:2")) {
      shoesInOut(2);
    } else if (command.equals("SHOESIN:3")) {
      shoesInOut(3);
    } else if (command.equals("SHOESIN:4")) {
      shoesInOut(4);
    } else if (command.equals("SHOESIN:5")) {
      shoesInOut(5);
    } else if (command.equals("SHOESIN:6")) {
      shoesInOut(6);
    } else if (command.equals("SHOESOUT:1")) {
      shoesInOut(1);
    } else if (command.equals("SHOESOUT:2")) {
      shoesInOut(2);
    } else if (command.equals("SHOESOUT:3")) {
      shoesInOut(3);
    } else if (command.equals("SHOESOUT:4")) {
      shoesInOut(4);
    } else if (command.equals("SHOESOUT:5")) {
      shoesInOut(5);
    } else if (command.equals("SHOESOUT:6")) {
      shoesInOut(6);
    } 
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  dht.begin();
  lcd.begin();
  settingHeater();
  settingCooler();
  settingVentilator();
  settingPlasmaModule();
  settingMotor();
}
  
void loop() {
  btCommunication();
}