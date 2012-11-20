// I wrote it according to the following datasheet:
// http://ww1.microchip.com/downloads/en/DeviceDoc/39622L.pdf

//pin out config
#define PGC 6
#define PGD 5
//#define EN 7 //PGM & MCLR
#define PGM 8
#define MCLR 7

#define P15 1
#define P12 1

String inputString = "";
boolean stringComplete = false;

unsigned long temp;

byte buffer[32];
byte address[3];

void setup(){
  Serial.begin(9600);
  pinMode(PGC,OUTPUT);
  pinMode(PGD,OUTPUT);
  pinMode(PGM,OUTPUT);
  pinMode(MCLR,OUTPUT);
  
 inputString.reserve(200);
 
}


void loop(){ 

  //turn on the chip (disable programming mode & diable reset)
  digitalWrite(PGM,LOW);
  digitalWrite(MCLR,HIGH);
  
} 



//////THE MAIN CODE///////////
//////////////////////////////
//////////////////////////////
void serialEvent() {
   
  while (Serial.available()) {

    char inChar = (char)Serial.read(); 
    
    if (inChar == 'X') { //buffering till X
      stringComplete = true;
      break;
    } 
    
    inputString += inChar;
    
  }
  
  
  if (stringComplete && inputString.charAt(0) == 'W'){ //WRITE
    
    if(inputString.length() != 69) {
      //Serial.println("Input is wrong, should be: W<address - 4 digit><32bytes>X"); 
      nullString();
      goto endofthisif;
      }
    
    address[2] = 0;
    address[1] = char2byte(inputString.charAt(2),inputString.charAt(1));
    address[0] = char2byte(inputString.charAt(4),inputString.charAt(3));
    
    nullBuffer(); // set everything FF
    
    for(int i=0;i<32;i++){
      buffer[i] = char2byte(inputString.charAt(2*i+6),inputString.charAt(2*i+5));
      } 
    
    nullString();
    
    // print address
    //Serial.print("Programming 32bytes,starting at:");
    //Serial.print("0x00");
    //if(address[1]<0x10) Serial.print("0"); Serial.print(address[1],HEX);
    //if(address[0]<0x10) Serial.print("0"); Serial.println(address[0],HEX);
    
    //for(int i=0;i<32;i++) { Serial.print(buffer[i],HEX); }
      
    //Serial.println("");
    
    ///Write
    
    digitalWrite(PGM,HIGH);
    digitalWrite(MCLR,HIGH);
    
    delay(1);

    programBuffer(address[2],address[1],address[0]);
    
    digitalWrite(PGM,LOW);
    digitalWrite(MCLR,LOW);
    
    //Serial.println("Programming complety");    
    }
  
  endofthisif:;
  
  
  
  
  if (stringComplete && inputString.charAt(0) == 'E'){ //Erase all
     digitalWrite(PGM,HIGH);
     digitalWrite(MCLR,HIGH);
     delay(1);
     
     ////erase 
     erase_all();
     
    digitalWrite(PGM,LOW);
    digitalWrite(MCLR,LOW);
     
     //Serial.println("Erase complety");
     
     nullString();
    }
    
    
    
    
    
    
    if (stringComplete && inputString.charAt(0) == 'R'){ //READ
      
      
      
         address[2] = char2byte(inputString.charAt(2),inputString.charAt(1));
         address[1] = char2byte(inputString.charAt(4),inputString.charAt(3));
         address[0] = char2byte(inputString.charAt(6),inputString.charAt(5));
         
         //read
         
    
    temp=0;

    temp = ((long)address[2])<<(16); //doesn't work with out (long)
    temp |= ((long)address[1])<<(8);
    temp |= (long)address[0];
    
    Serial.print(temp,HEX); Serial.print(":");  
    
    digitalWrite(PGM,HIGH);
    digitalWrite(MCLR,HIGH);
    delay(1);
    
    for(int i=0;i<32;i++){
        address[2] = byte( (temp&0xFF0000)>>16 );
        address[1] = byte( (temp&0xFF00)>>8 );
        address[0] = byte( temp&0xFF );
        Serial.print(readFlash(address[2],address[1],address[0]),HEX);
        Serial.print(" ");
        temp++;
       }
    
    Serial.println("");
    
    digitalWrite(PGM,LOW);
    digitalWrite(MCLR,LOW);
    
    nullString();
     
    }
    
    
    if (stringComplete && inputString.charAt(0) == 'C'){ //config
    
     digitalWrite(PGM,HIGH);
     digitalWrite(MCLR,HIGH);
     delay(1);
    
    configWrite( char2byte(inputString.charAt(1),'0'),char2byte(inputString.charAt(3),inputString.charAt(2)) );
    
    digitalWrite(PGM,LOW);
    digitalWrite(MCLR,LOW);
    nullString();
    
    }
    
    if (stringComplete && inputString.charAt(0) == 'D'){ //config
    
    if( checkIf_pic18f2550() ){
      delay(100); Serial.print("T");
    } else {
      delay(100); Serial.print("F");
    }
    
    
    digitalWrite(PGM,LOW);
    digitalWrite(MCLR,LOW);
    nullString();
    
    }
    
    //clear string incase the first CHAR isn't E,R,W
    if(inputString.charAt(0) != 'E' && inputString.charAt(0) != 'R' && inputString.charAt(0) != 'W' && inputString.charAt(0) != 'C' && inputString.charAt(0) != 'D') nullString();
  
}




byte readFlash(byte usb,byte msb,byte lsb){
  
  byte value=0;
  
  send4bitcommand(B0000);
  send16bit( 0x0e00 | usb ); //
  send4bitcommand(B0000);
  send16bit(0x6ef8);
  
  send4bitcommand(B0000);
  send16bit( 0x0e00 | msb ); //
  send4bitcommand(B0000);
  send16bit(0x6ef7);
  
  send4bitcommand(B0000);
  send16bit( 0x0e00 | lsb ); //
  send4bitcommand(B0000);
  send16bit(0x6ef6); 
  
  send4bitcommand(B1000); //
  
  pinMode(PGD,INPUT); digitalWrite(PGD,LOW);
  
  for(byte i=0;i<8;i++){ //read
     digitalWrite(PGC,HIGH);
     digitalWrite(PGC,LOW);
    }
    
    
for(byte i=0;i<8;i++){ //shift out
     digitalWrite(PGC,HIGH); 
     digitalWrite(PGC,LOW);
     if(digitalRead(PGD) == HIGH) value += 1<<i; //sample PGD
    } 
    
  return value;
 
  }


void send4bitcommand(byte data){
  pinMode(PGD,OUTPUT);
  for(byte i=0;i<4;i++){
    if( (1<<i) & data ) digitalWrite(PGD,HIGH); else digitalWrite(PGD,LOW); 
     digitalWrite(PGC,HIGH);
     digitalWrite(PGC,LOW);
    }

}

void send16bit(unsigned int data){
  pinMode(PGD,OUTPUT);
  for(byte i=0;i<16;i++){
    if( (1<<i) & data ) digitalWrite(PGD,HIGH); else digitalWrite(PGD,LOW); 
     digitalWrite(PGC,HIGH);
     digitalWrite(PGC,LOW);
    }

}


void erase_all(){ //for some reason the chip stops respone, just reconnect voltage
  
  send4bitcommand(B0000);
  send16bit(0x0e3c); //
  send4bitcommand(B0000);
  send16bit(0x6ef8);
  
  send4bitcommand(B0000);
  send16bit(0x0e00); //
  send4bitcommand(B0000);
  send16bit(0x6ef7);
  
  send4bitcommand(B0000);
  send16bit(0x0e05); //
  send4bitcommand(B0000);
  send16bit(0x6ef6);
  
  send4bitcommand(B1100);
  send16bit(0x3F3F);
  
  
  send4bitcommand(B0000);
  send16bit(0x0e3c); //
  send4bitcommand(B0000);
  send16bit(0x6ef8);
  
  send4bitcommand(B0000);
  send16bit(0x0e00); //
  send4bitcommand(B0000);
  send16bit(0x6ef7);
  
  send4bitcommand(B0000);
  send16bit(0x0e04); //
  send4bitcommand(B0000);
  send16bit(0x6ef6);
  
  send4bitcommand(B1100);
  send16bit(0x8F8F);
  
  send4bitcommand(B0000); //
  send16bit(0x0000); //
  
  delay(2);
  
  send4bitcommand(B0000); //
  send16bit(0x0000); //
  
  delay(2);
  
  digitalWrite(PGD,LOW);
  
  delay(2);
  
  digitalWrite(PGD,HIGH);
  
}


void configWrite(byte address,byte data){
  
  
  send4bitcommand(B0000);
  send16bit( 0x8ea6 );
  send4bitcommand(B0000);
  send16bit( 0x8ca6 );  
  
  send4bitcommand(B0000);
  send16bit( 0x0e30 ); //
  send4bitcommand(B0000);
  send16bit(0x6ef8);
  
  send4bitcommand(B0000);
  send16bit( 0x0e00 ); //
  send4bitcommand(B0000);
  send16bit(0x6ef7);
  
  send4bitcommand(B0000);
  send16bit( 0x0e00 | address ); //
  send4bitcommand(B0000);
  send16bit(0x6ef6); 
  
  //Serial.println("programming");
  
  send4bitcommand(B1111);
  if(address&0x01) send16bit( 0x0000 | data<<8 ); // if even MSB ignored, LSB read 
  else send16bit( 0x0000 | data ); // if odd MSB read, LSB ignored
  
  digitalWrite(PGC,HIGH);
  digitalWrite(PGC,LOW);
  
  digitalWrite(PGC,HIGH);
  digitalWrite(PGC,LOW);
  
  digitalWrite(PGC,HIGH);
  digitalWrite(PGC,LOW);  

  digitalWrite(PGC,HIGH);
  delay(1);
  digitalWrite(PGC,LOW);
  delayMicroseconds(100);
  
  send16bit(0x0000); 
  }
  
void programBuffer(byte usb,byte msb,byte lsb){ 
  
  if( (lsb&0x0F) | ((lsb&0xF0)>>4)%2){
   //Serial.println("Error: First digit (refere as HEX) have to be 0, and the second digit have to be even.");
   //Serial.println("valid examples: 0x000000 , 0x000060, 0x006320, 0x0063E0"); 
   //Serial.println("INvalid examples: 0x000004 , 0x000014, 0x006328, 0x0063EA");
   //Serial.println("0,2,4,6,8,A,C,E are even");
   return;
  }
  
  //step 1
  send4bitcommand(B0000);
  send16bit(0x8ea6); 
  send4bitcommand(B0000);
  send16bit(0x9ca6);
  
  //step 2
  send4bitcommand(B0000);
  send16bit( 0x0e00 | usb ); //
  send4bitcommand(B0000);
  send16bit(0x6ef8);
  
  send4bitcommand(B0000);
  send16bit( 0x0e00 | msb ); //
  send4bitcommand(B0000);
  send16bit(0x6ef7);
  
  send4bitcommand(B0000);
  send16bit( 0x0e00 | lsb ); //
  send4bitcommand(B0000);
  send16bit(0x6ef6);

  //step 3
  for(byte i=0;i<15;i++){
    send4bitcommand(B1101);
    send16bit( buffer[(2*i)+1]<<8 | buffer[(i*2)] );    
    } 
  
  //step 4
  send4bitcommand(B1111);
  send16bit( buffer[31]<<8 | buffer[30] );  
  
  //nop
  digitalWrite(PGC,HIGH);
  digitalWrite(PGC,LOW);
  
  digitalWrite(PGC,HIGH);
  digitalWrite(PGC,LOW);
  
  digitalWrite(PGC,HIGH);
  digitalWrite(PGC,LOW);  

  digitalWrite(PGC,HIGH);
  delay(1);
  digitalWrite(PGC,LOW);
  delayMicroseconds(100);
  
  send16bit(0x0000); 
  
  //done
  
  }


/////////////////////////////
//nothing special underhere:

void nullString(){
  inputString = "";
  stringComplete = false; 
  }
  
void nullBuffer(){
  for(byte i=0;i<32;i++) buffer[i] = 0xFF;
  }
  
byte char2byte(char lsb,char msb){
  
  byte result=0;
  
  switch(lsb){
    case '0': result = 0; break;
    case '1': result = 1; break;
    case '2': result = 2; break;
    case '3': result = 3; break;
    case '4': result = 4; break;
    case '5': result = 5; break;
    case '6': result = 6; break;
    case '7': result = 7; break;
    case '8': result = 8; break;
    case '9': result = 9; break;
    case 'A': result = 0xA; break;
    case 'B': result = 0xB; break;
    case 'C': result = 0xC; break;
    case 'D': result = 0xD; break;
    case 'E': result = 0xE; break;
    case 'F': result = 0xF; break;
  }
  
switch(msb){
    case '0': result |= 0<<4; break;
    case '1': result |= 1<<4; break;
    case '2': result |= 2<<4; break;
    case '3': result |= 3<<4; break;
    case '4': result |= 4<<4; break;
    case '5': result |= 5<<4; break;
    case '6': result |= 6<<4; break;
    case '7': result |= 7<<4; break;
    case '8': result |= 8<<4; break;
    case '9': result |= 9<<4; break;
    case 'A': result |= 0xA<<4; break;
    case 'B': result |= 0xB<<4; break;
    case 'C': result |= 0xC<<4; break;
    case 'D': result |= 0xD<<4; break;
    case 'E': result |= 0xE<<4; break;
    case 'F': result |= 0xF<<4; break;
  }
  
  return result;
  
}

int checkIf_pic18f2550(){
  
  digitalWrite(PGM,HIGH);
  digitalWrite(MCLR,HIGH);
  delay(1);
  
  if( readFlash(0x3f,0xff,0xff) == 0x12 )
    return 1;
  else
    return 0;
}



