#include <SD.h>
int pin5=5;
int pin7=7;
int pin8=8;
int pin12=12;
int masterPin = 4;
int isMaster = 0;
int msgToken = 0;
int message =0;
int a=0;
int b=0;
int c=0;
int d=0;

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
  //digitalWrite(lightR,HIGH);

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
            //digitalWrite(lightB,HIGH);

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
              //digitalWrite(lightG,HIGH);
              
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
    
    //digitalWrite(lightG,LOW);
    //digitalWrite(lightB,LOW);
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
  //digitalWrite(lightR,LOW);
}

void setup() {
  pinMode(pin5, INPUT);
  pinMode(pin7, INPUT);
  pinMode(pin8, INPUT);
  pinMode(pin12,INPUT);
  pinMode(isMaster, INPUT);

  
  BtInit();
}

void receiveMsg(){  
  for(int i = 0; i < 10; i++){
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      if(incomingByte == 'R'){
       // digitalWrite(lightB, LOW);
        delay(50);
       // digitalWrite(lightB,HIGH);
      }
    }
    
    delay(100);
  }
}

void sendMsg(int msg){
 // digitalWrite(lightG,LOW);  
  delay(50);  
  Serial.print(msg);
 // digitalWrite(lightG,HIGH);
  delay(50);  
  Serial.print(msg);
 // digitalWrite(lightG,LOW);  
  delay(50); 
 // digitalWrite(lightG,HIGH);
  Serial.print(msg);
}

void loop() 
{
  a=digitalRead(pin5);
  b=digitalRead(pin7);
  c=digitalRead(pin12);
  d=digitalRead(pin8);
  if((a==0) && (b==0) && (c==0) && (d==0)){
     message=0;
  }
  else if((a==1) && (b==0) && (c==0) && (d==0)){
     message=1;
  }
  else if((a==0) && (b==1) && (c==0) && (d==0)){
     message=2;
  }
  else if((a==1) && (b==1) && (c==0) && (d==0)){
     message=3;
  }
  else if((a==0) && (b==0) && (c==1) && (d==0)){
     message=4;
  }
  else if((a==1) && (b==0) && (c==1) && (d==0)){
     message=5;
  }
  else if((a==0) && (b==1) && (c==1) && (d==0)){
     message=6;
  }
  else if((a==1) && (b==1) && (c==1) && (d==0)){
     message=7;
  }
  else if((a==0) && (b==0) && (c==0) && (d==1)){
     message=8;
  }  
  if(msgToken == 1){
    //digitalWrite(lightG,HIGH);
    sendMsg(message);
    delay(1000);
  }else{
    //digitalWrite(lightB,HIGH);
    receiveMsg();
  }
}
