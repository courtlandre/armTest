#define REMOTEXY_MODE__SOFTSERIAL
#include <SoftwareSerial.h>
#include <HUSKYLENS.h>
#include <Arduino.h>
#include <AccelStepper.h>
#include <RemoteXY.h>
#define REMOTEXY_SERIAL_RX 4
#define REMOTEXY_SERIAL_TX 5
#define REMOTEXY_SERIAL_SPEED 9600

#define motorInterfaceType 1
#define arm1Dir 7
#define arm1Step 8
#define arm2Dir 10
#define arm2Step 11
#define arm3Dir 12 // Base
#define arm3Step 13

#define endStopPin A0 // endstop
int endStopState = 0; // endstop
int endStopChecked = 0;

// A0-A5 as digital pin, reference A0 instead of just 0


// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =
  { 255,3,0,201,0,36,0,10,13,0,
  67,4,3,5,95,5,2,26,201,4,
  48,56,7,7,50,2,26,4,48,66,
  7,7,50,2,26,4,48,76,7,7,
  50,2,26 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t slider_1; // =-100..100 slider position 
  int8_t slider_2; // =-100..100 slider position 
  int8_t slider_3; // =-100..100 slider position 

    // output variables
  char distance[201];  // string UTF8 end zero 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////


// Create a new instance of the AccelStepper class:
AccelStepper arm1Stepper = AccelStepper(motorInterfaceType, arm1Step, arm1Dir);
AccelStepper arm2Stepper = AccelStepper(motorInterfaceType, arm2Step, arm2Dir);
AccelStepper arm3Stepper = AccelStepper(motorInterfaceType, arm3Step, arm3Dir);

HUSKYLENS huskylens;
HUSKYLENSResult result;
int ID1 = 1;

void setup() {
  RemoteXY_Init ();
  pinMode(endStopPin, INPUT); // endstop
  // HUSKYLENS 
  Serial.begin(115200);
    Wire.begin();
    while (!huskylens.begin(Wire))
    {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
        Serial.println(F("2.Please recheck the connection."));
        delay(100);
    }
     huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION); //Switch the algorithm to line tracking.
  // Set the maximum speed in steps per second:
  arm1Stepper.setMaxSpeed(100);
  arm2Stepper.setMaxSpeed(100);
  arm3Stepper.setMaxSpeed(100);
  arm1Stepper.setAcceleration(5);
  arm2Stepper.setAcceleration(5);
  arm3Stepper.setAcceleration(5);
}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
        sprintf (RemoteXY.distance,"from block");
    }
    else if (result.command == COMMAND_RETURN_ARROW){
      //String str = "Arrow:xOrigin"
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
        sprintf (RemoteXY.distance,"from arrow");
    }
    else{
        strcpy (RemoteXY.distance, "Object unknown!");
        Serial.println("Object unknown!");
    }
}
void setZeroPosition(){
  
  endStopState = digitalRead(endStopPin);
  if (endStopState == HIGH) {
    // SWITCH NOT PRESSED:
    arm2Stepper.moveTo(200);
    arm2Stepper.runToPosition();

  } else {
    // SWITCH PRESSED:
    arm2Stepper.setCurrentPosition(0);
    endStopChecked = 1;
  }
}

void loop() {
  RemoteXY_Handler ();
  // Check endstops
  arm2Stepper.moveTo(200);
    arm2Stepper.runToPosition();
  //if (endStopChecked == 0)
  //{
   // setZeroPosition();
  //}
  
  

  // Set the speed in steps per second:
  //arm1Stepper.setSpeed(RemoteXY.slider_1);
  //arm2Stepper.setSpeed(RemoteXY.slider_2);
  //arm3Stepper.setSpeed(RemoteXY.slider_3);
  // Step the motor with a constant speed as set by setSpeed():
  //arm1Stepper.runSpeed();
  //arm2Stepper.runSpeed();
  //arm3Stepper.runSpeed();

  // HUSKYLENS
  int32_t xTarget;
  int32_t yTarget;
  if (!huskylens.request(ID1)) {Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));}
  else if(!huskylens.isLearned()) {Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));}
  else if(!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
  else
  {
      result = huskylens.read();
      printResult(result);

      xTarget = (int32_t)result.xTarget;
      yTarget = (int32_t)result.yTarget;

  }

  //check distance method, call move method from check distance, wait some time for move

}

