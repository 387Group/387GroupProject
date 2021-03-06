#include <SD.h>
#include <Servo.h>
Servo middle1, left, right;
int lightB = 7;
int lightG = 6;
int lightR = 5;
int masterPin = 4;
int isMaster = 0;
int msgToken = 0;

int incomingByte = 0;
char gBtMsg[256];
char gBTAdr[13];
char gBtCmd[256];

int gBtKnownMACTotal = 2;
char* gBtKnownMAC[2]; //This is set to hold only two MAC adresses

File gLogFile;

void SdInitLog(void){
  SD.begin(10);
}

void SdLog(char* i_pBtCmd){
  if(strlen(i_pBtCmd) > 0){
    File gLogFile = SD.open("btlog.txt",FILE_WRITE);
    if(gLogFile){
      gLogFile.println(i_pBtCmd);
      gLogFile.close();
    }
  }
}

void BtReceive(void){
  bool keepReading = true;
  int index = 0;
  
  gBtMsg[0] = '\0';
  
  while(keepReading){
    keepReading = false;
    
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      if(incomingByte != 13){
        gBtMsg[index++] = incomingByte;
        keepReading = true;
      }
    }
  }
  
  gBtMsg[index] = '\0';
}

void BtSend(char* i_pBtCmd, bool i_ln = true){

  if(i_ln){
    Serial.println(i_pBtCmd);
  } else{
    Serial.print(i_pBtCmd);
  }
  
  delay(100);
  BtReceive();
  
  //Debug start
//  SdLog("\\");
//  SdLog(i_pBtCmd);
//  SdLog(gBtMsg);
//  SdLog("/");
  //Debug end
}


void BtInit(void){
  isMaster = digitalRead(masterPin); 
  bool btConnect = false;
  digitalWrite(lightR,HIGH);

  gBtKnownMAC[0] = "0006667D98D8"; // Change this to one of your MAC addresses
  gBtKnownMAC[1] = "0006667BACCE"; // and this one too
  
  SdInitLog();
  SdLog("#### start ####");
 
  Serial.begin(115200); 
  BtSend("$$$", false);
  
  if(isMaster == LOW){
    SdLog(">>>> Set To Master <<<<");

    BtSend("SM,1");

    BtSend("I,5");
    BtSend("---"); 
   
    while(!btConnect){
      delay(100);
      BtReceive();
      int msgLen = strlen(gBtMsg);
      if(msgLen > 0){
        
        if(msgLen >= 12){
          char* doneMsg = &gBtMsg[msgLen - 12];
          
          gBtMsg[12] = '\0';
          bool foundKnownMAC = false;
          
          for(int i = 0; i < gBtKnownMACTotal && !foundKnownMAC; i++){
            if(!strcmp(gBtMsg, gBtKnownMAC[i])){
              foundKnownMAC = true;
            }
          }
          
          if(!strcmp(doneMsg, "Inquiry Done")){
            digitalWrite(lightB,HIGH);

            SdLog(doneMsg);
            
            if(foundKnownMAC){              
              gBtCmd[0] = 'C';
              gBtCmd[1] = ',';
              
              for(int i = 0; i < 12; i++){
                gBtCmd[i + 2] = gBtMsg[i];
              }
              
              gBtCmd[15] = '\0';
                  
              BtSend("$$$", false);
              BtSend(gBtCmd);
              BtSend("---");      

              delay(2000);
              digitalWrite(lightG,HIGH);
              
              while(!btConnect){
                delay(1000);
                BtSend("$$$", false);
                BtSend("GK");
                
                int numVal = 0;
      
                if(strlen(gBtMsg) > 0) {
                  numVal= atoi(gBtMsg);
                }
                
                if(numVal == 1){
                  btConnect = true;
                  SdLog("Is connected !!!!!!");
                }
                BtSend("---");
              }
            }
          }
        }
      }
    }
    
    digitalWrite(lightG,LOW);
    digitalWrite(lightB,LOW);
    msgToken = 1;
  } else{
    SdLog(">>>> Set To Slave <<<<");
    BtSend("SM,0");
    BtSend("---"); 
    
    while(!btConnect){
      delay(1000);

      BtSend("$$$", false);
      BtSend("GK");
      
      int numVal = 0;
      
      if(strlen(gBtMsg) > 0) {
        numVal= atoi(gBtMsg);
      }
      
      if(numVal == 1){                  
        btConnect = true;
        SdLog("Is connected !!!!!!");
      }

      BtSend("---");              
    }
  }
  
  SdLog("#### end ####");
  digitalWrite(lightR,LOW);
}

void setup() {
  middle1.attach(6);
  left.attach(10);
  right.attach(9);

  pinMode(lightR, OUTPUT);
  pinMode(lightG, OUTPUT);
  pinMode(lightB, OUTPUT);
  pinMode(isMaster, INPUT);
  
  digitalWrite(lightR,LOW);
  digitalWrite(lightG,LOW);
  digitalWrite(lightB,HIGH);
  delay(500);
  digitalWrite(lightB,LOW);
  digitalWrite(lightR,HIGH);
  delay(500);
  digitalWrite(lightR,LOW);
  digitalWrite(lightG,HIGH);
  delay(500);
  digitalWrite(lightG,LOW);  
  
  BtInit();
  

}

void receiveMsg(){  
  for(int i = 0; i < 10; i++){
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      if(incomingByte == '0'){//initial
          middle1.write(90);
          left.write(125);
          right.write(135);
      }
      else if(incomingByte == '1'){//up&left
          middle1.write(160);
          left.write(160);
          right.write(160);
      }
      else if(incomingByte == '2'){//up
          middle1.write(90);
          left.write(160);
          right.write(160);
      }
      else if(incomingByte == '3'){//up right
          middle1.write(20);
          left.write(160);
          right.write(160);   
      }
      else if(incomingByte == '4'){//left
          middle1.write(160);
          left.write(125);
          right.write(135);
      }
      else if(incomingByte == '5'){//right
          middle1.write(20);
          left.write(125);
          right.write(135);
      }
      else if(incomingByte == '6'){//down&left
          middle1.write(160);
          left.write(90);
          right.write(115);
      }
      else if(incomingByte == '7'){//down
          middle1.write(90);
          left.write(90);
          right.write(115);
      }
      else if(incomingByte == '8'){//down right
          middle1.write(20);
          left.write(90);
          right.write(115);
       }
    }
    
    delay(100);
  }
}

void sendMsg(){
  digitalWrite(lightG,LOW);  
  delay(50);  
  Serial.print("R");
  digitalWrite(lightG,HIGH);
  delay(50);  
  Serial.print("R");
  digitalWrite(lightG,LOW);  
  delay(50);  
  digitalWrite(lightG,HIGH);
  Serial.print("R");
}

void loop() 
{  
  if(msgToken == 1){
    //digitalWrite(lightG,HIGH);
    sendMsg();
    delay(1000);
  }else{
    //digitalWrite(lightB,HIGH);
    receiveMsg();
  }
}

