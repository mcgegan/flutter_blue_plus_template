/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <BLE2901.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID      "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

#define CP_DUTY_UUID      "bcaf4dee-e9b1-4f61-8bfa-340554dc63fa"
#define CP_VOLTAGE_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CP_STATUS_UUID    "ee17c408-d77e-4a12-9357-fd4e26206d8f"
#define PP_VOLTAGE_UUID   "6e99e63f-55a5-4a1e-9788-ed2c44101750"
#define PP_STATUS_UUID    "1f3ef3d7-18be-44e5-82f7-ff8d498f4e4b"

#define CP_STATUS_A 0 //Va=12V
#define CP_STATUS_B 1 //Va=9V
#define CP_STATUS_C 2 //Va=6V
#define CP_STATUS_D 3 //Va=3V
#define CP_STATUS_E 4 //Va=0V
#define CP_STATUS_F 5 //Va=-12V

#define PD_5V_FAIL                  0
#define PD_INLET_FAIL               1
#define PD_CONNECTOR_OPEN           2
#define PD_CONNECTOR_READY          3
#define PD_CONNECTOR_SWITCH_OPEN    4

#define PP_ADC_PIN 2
#define CP_ADC_PIN 4
#define CP_PWM_PIN 16
#define CP_PWM_FREQUENCY 1000

void cpPWMSetup();
void cpPWMUpdate();
float cpVoltageRead();
float ppVoltageRead();
int getCpStatus();
int getPpStatus();
int setpCPStatusCharacteristic();
int setpPPStatusCharacteristic();
// V2G variables begin
int CPDuty = 50;
float cpVoltage = 0;
float ppVoltage = 0;
int cpStatus = 0;
int ppStatus = 0;
char CPDutyStringBuffer[256];
char CPVoltageStringBuffer[256];
char CPStatusStringBuffer[256];
char PPVolageStringBuffer[256];
char PPStatusStringBuffer[256];

String CPDutyString = String(CPDuty);
String CPVoltageString = "CPVoltage";
String CPStatusString = "CP Status";
String PPVoltageString = "PPVolatge";
String PPStatusString = "PPStatus";
// V2G variables end

// BLE variables begin
BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLE2901 *descriptor_2901 = NULL;

BLECharacteristic *pCPDutyCharacteristic = NULL;
BLECharacteristic *pCPVoltageCharacteristic = NULL;
BLECharacteristic *pCPStatusCharacteristic = NULL;
BLECharacteristic *pPPVoltageCharacteristic = NULL;
BLECharacteristic *pPPStatusCharacteristic = NULL;
// BLE variables end

// BLE status variable begin
bool deviceConnected = false;
bool oldDeviceConnected = false;
// BLE status variable end

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Bie V2G PilotEmu");
  pServer = BLEDevice::createServer();
  //pServer->setCallbacks(BLEServerCallbacks *pCallbacks)
  pServer->setCallbacks(new MyServerCallbacks());

  pService = pServer->createService(SERVICE_UUID);

  pCPDutyCharacteristic = pService->createCharacteristic(
    CP_DUTY_UUID, 
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_WRITE | 
    BLECharacteristic::PROPERTY_NOTIFY
    //BLECharacteristic::PROPERTY_NOTIFY | 
    //BLECharacteristic::PROPERTY_INDICATE
  );
  //pCPDutyCharacteristic->addDescriptor(new BLE2902());
  //descriptor_2901 = new BLE2901();
  //descriptor_2901->setDescription("CPDutyCharacteristic description.");
  //descriptor_2901->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
  //pCPDutyCharacteristic->addDescriptor(descriptor_2901);

  pCPVoltageCharacteristic =
    pService->createCharacteristic(
      CP_VOLTAGE_UUID, 
      BLECharacteristic::PROPERTY_READ | 
      BLECharacteristic::PROPERTY_NOTIFY);
  pCPStatusCharacteristic =
    pService->createCharacteristic(
      CP_STATUS_UUID, 
      BLECharacteristic::PROPERTY_READ | 
      BLECharacteristic::PROPERTY_NOTIFY);
  pPPVoltageCharacteristic =
    pService->createCharacteristic(
      PP_VOLTAGE_UUID, 
      BLECharacteristic::PROPERTY_READ | 
      BLECharacteristic::PROPERTY_NOTIFY);
  pPPStatusCharacteristic =
    pService->createCharacteristic(
      PP_STATUS_UUID, 
      BLECharacteristic::PROPERTY_READ | 
      BLECharacteristic::PROPERTY_NOTIFY);

  pCPDutyCharacteristic->setValue("50");
  pCPVoltageCharacteristic->setValue(CPVoltageString);
  pCPStatusCharacteristic->setValue(CPStatusString);
  pPPVoltageCharacteristic->setValue(PPVoltageString);
  pPPStatusCharacteristic->setValue(PPStatusString);

  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  //pAdvertising->setScanResponse(true);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  cpPWMSetup();
  //analogReadResolution(12);//default 12bit
  //analogSetAttenuation(ADC_11db);//default ADC_11db
}

void loop() {
  if (deviceConnected) {
    if ((pCPDutyCharacteristic->getValue().toInt() >= 0) && 
        (pCPDutyCharacteristic->getValue().toInt() <= 100)) {
      CPDuty = pCPDutyCharacteristic->getValue().toInt();
    }
    else {
      CPDuty = 0;
    }
    pCPDutyCharacteristic->setValue(String(CPDuty));
    pCPDutyCharacteristic->notify();
    //Serial.print("CPDuty Raw=");
    //Serial.print(pCPDutyCharacteristic->getValue());
    Serial.print("CPDuty=");
    Serial.println(CPDuty);
    //delay(10);
    

    cpPWMUpdate(CPDuty);

    // update cpVoltage
    cpVoltage = cpVoltageRead();
    pCPVoltageCharacteristic->setValue(String(cpVoltage));
    pCPVoltageCharacteristic->notify();
    Serial.print("cpVoltage=");
    Serial.println(cpVoltage);
    //delay(10);

    // update cpStatus
    getCpStatus();
    setpCPStatusCharacteristic();
    pCPStatusCharacteristic->notify();
    //delay(10);

    // update ppVoltage
    ppVoltage = ppVoltageRead();
    pPPVoltageCharacteristic->setValue(String(ppVoltage));
    pPPVoltageCharacteristic->notify();
    Serial.print("ppVoltage=");
    Serial.println(ppVoltage);
    //delay(10);

    // update ppStatus
    getPpStatus();
    setpPPStatusCharacteristic();
    pPPStatusCharacteristic->notify();
    // put your main code here, to run repeatedly:

    delay(2000);//delay mS
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}

void cpPWMSetup(){
  pinMode(CP_PWM_PIN, OUTPUT);
  analogWriteFrequency(CP_PWM_PIN, CP_PWM_FREQUENCY);
  analogWrite(CP_PWM_PIN, 0);
}

void cpPWMUpdate(int duty){
  analogWrite(CP_PWM_PIN, 255 * duty / 100);
}

float cpVoltageRead(){
  int value = analogRead(CP_ADC_PIN);
  return ((float)value)/146.0;
}

float ppVoltageRead(){
  int value = analogRead(PP_ADC_PIN);
  return (float)(value);
}

int getCpStatus(){
  if ((cpVoltage >= 11) && (cpVoltage<= 10)){
    cpStatus = CP_STATUS_A;
  }
  else if ((cpVoltage >= 8) && (cpVoltage <= 10)){
    cpStatus = CP_STATUS_B;
  }
  else if ((cpVoltage >= 5) && (cpVoltage <= 7)){
    cpStatus = CP_STATUS_C;
  }
  else if ((cpVoltage >= 2) && (cpVoltage <= 4)){
    cpStatus = CP_STATUS_D;
  }
  else if (cpVoltage <= 1){
    cpStatus = CP_STATUS_E;
  }
  else{
    cpStatus = CP_STATUS_A;
  }
  return cpStatus;
}

int getPpStatus(){
    if ((ppVoltage >= 5.15) && (ppVoltage <= 5.5))
    {
      ppStatus = PD_INLET_FAIL;
    }
    else if ((ppVoltage >= 3.45) && (ppVoltage < 5.15))//typically 4.5V
    {
      ppStatus = PD_CONNECTOR_OPEN;
    }
    else if ((ppVoltage >= 2.15) && (ppVoltage < 3.45))//typically 2.8V
    {
      ppStatus = PD_CONNECTOR_SWITCH_OPEN;
    }
    else if ((ppVoltage >= 0.85) && (ppVoltage < 2.15))//typically 1.5V
    {
      ppStatus = PD_CONNECTOR_READY;
    }
    else if (ppVoltage < 0.85)
    {
      ppStatus = PD_5V_FAIL;
    }
    else
    {
      ppStatus = PD_5V_FAIL;
    }
    return ppStatus;
}

int setpCPStatusCharacteristic(){
  if (cpStatus == CP_STATUS_A) {
    pCPStatusCharacteristic->setValue("CP_STATUS_A");
  }
  else if (cpStatus == CP_STATUS_B) {
    pCPStatusCharacteristic->setValue("CP_STATUS_B");
  }
  else if (cpStatus == CP_STATUS_C) {
    pCPStatusCharacteristic->setValue("CP_STATUS_C");
  }
  else if (cpStatus == CP_STATUS_D) {
    pCPStatusCharacteristic->setValue("CP_STATUS_D");
  }
  else if (cpStatus == CP_STATUS_E) {
    pCPStatusCharacteristic->setValue("CP_STATUS_E");
  }
  else if (cpStatus == CP_STATUS_F) {
    pCPStatusCharacteristic->setValue("CP_STATUS_F");
  }
  else {
    pCPStatusCharacteristic->setValue("CP_STATUS_NONE");
  }
  return 0;
}

int setpPPStatusCharacteristic(){
  if (ppStatus == PD_5V_FAIL) {
    //PPStatusString = "PD_5V_FAIL";
    pPPStatusCharacteristic->setValue("PD_5V_FAIL");
  }
  else if (ppStatus == PD_INLET_FAIL) {
    //PPStatusString = "PD_INLET_FAIL";
    pPPStatusCharacteristic->setValue("PD_INLET_FAIL");
  }
  else if (ppStatus == PD_CONNECTOR_OPEN) {
    //PPStatusString = "PD_CONNECTOR_OPEN";
    pPPStatusCharacteristic->setValue("PD_CONNECTOR_OPEN");
  }
  else if (ppStatus == PD_CONNECTOR_READY) {
    //PPStatusString = "PD_CONNECTOR_READY";
    pPPStatusCharacteristic->setValue("PD_CONNECTOR_READY");
  }
  else if (ppStatus == PD_CONNECTOR_SWITCH_OPEN) {
    //PPStatusString = "PD_CONNECTOR_SWITCH_OPEN";
    pPPStatusCharacteristic->setValue("PD_CONNECTOR_SWITCH_OPEN");
  }
  else {
    //PPStatusString = "PP_STATUS_NONE";
    pPPStatusCharacteristic->setValue("PP_STATUS_NONE");
  }
  //pPPStatusCharacteristic->setValue(PPStatusString);
  return 0;
}
