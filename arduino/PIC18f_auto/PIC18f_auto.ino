// I wrote it according to the following datasheet:
// http://ww1.microchip.com/downloads/en/DeviceDoc/39622L.pdf

// Modified for PIC18F2xK22
// http://ww1.microchip.com/downloads/en/devicedoc/41398b.pdf


//pin out config
#define PGC 6
#define PGD 5
#define PGM 8 /* unused on PIC18F2xK22 */
#define MCLR 7

// delay constants from page 42 & 43 of 39622L.pdf
#define P10us 100
#define P5us 1
#define P6us 1
#define P11ms 5

// page 38, 39
#define P9 (1+1)
#define P9A (5+1)
#define P10 1
#define P15 1
#define P18 1
#define P20 1

String inputString = "";
boolean stringComplete = false;

unsigned long temp;

byte buffer[32];
byte address[3];

void setup() {
    Serial.begin(2000000);
    pinMode(PGC, OUTPUT);
    pinMode(PGD, OUTPUT);
    pinMode(PGM, OUTPUT);
    pinMode(MCLR, OUTPUT);

    digitalWrite(MCLR, HIGH);
    Serial.print("ICD-A ready\n");

    inputString.reserve(200);
    initDevice();
}

void loop() {

    //turn on the chip (disable programming mode & disable reset)
    if (Serial.available())
        mainFunction();
}

//////THE MAIN CODE///////////
//////////////////////////////
//////////////////////////////
void mainFunction() {

    while (Serial.available()) {

        char inChar = (char) Serial.read();

        if (inChar == 'X') { //buffering till X
            stringComplete = true;
            break;
        }

        inputString += inChar;

    }

    if (stringComplete && inputString.charAt(0) == 'W') { //WRITE

        int offset;
        int data;
        if (inputString.length() == 69) { // 1+4+32*2
            // Program
            address[2] = 0;
            address[1] = char2byte(inputString.charAt(2), inputString.charAt(1));
            address[0] = char2byte(inputString.charAt(4), inputString.charAt(3));
            offset = 5; // WAAAA
            data = 32;
        } else if (inputString.length() == 71) { // 1+6+32*2
            // EEPROM
            address[2] = char2byte(inputString.charAt(2), inputString.charAt(1));
            address[1] = char2byte(inputString.charAt(4), inputString.charAt(3));
            address[0] = char2byte(inputString.charAt(6), inputString.charAt(5));
            offset = 7; // WAAAAAA
            data = 32;
        } else if (inputString.length() == 23) { // 1+6+8*2
            // ID memory
            address[2] = char2byte(inputString.charAt(2), inputString.charAt(1));
            address[1] = char2byte(inputString.charAt(4), inputString.charAt(3));
            address[0] = char2byte(inputString.charAt(6), inputString.charAt(5));
            offset = 7; // WAAAAAA
            data = 8;
        } else {
            //Serial.println("Input is wrong, should be: W<address - 4 digit><32bytes>X");
            nullString();
            goto endofthisif;
        }

        nullBuffer(); // set everything FF

        for (int i = 0; i < data; i++) {
            buffer[i] = char2byte(inputString.charAt(2 * i + offset + 1),
                    inputString.charAt(2 * i + offset));
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

        delay(1);

        if ((address[2] == 0x00) || (address[2] == 0x01)) {
            programBuffer(address[2], address[1], address[0]);
        } else if (address[2] == 0xf0) {
            // EEPROM Data
            for(int i=0;i<data;i++) {
                if (buffer[i] != 0xFF) { // Speedup, eeprom is erased, only write if bits change
                    eepromWrite(address[1],address[0]+i, buffer[i]);
                }
            }
        } else if (address[2] == 0x20) {
            // ID
            idBuffer();
        }

        Serial.print("K");
    }

    endofthisif: ;

    if (stringComplete && inputString.charAt(0) == 'E') { //Erase all
        delay(1);

        ////erase
        erase_all();
        delay(1);
        erase_eeprom();

        nullString();
        Serial.print("K");
    }

    if (stringComplete && inputString.charAt(0) == 'R') { //READ

        address[2] = char2byte(inputString.charAt(2), inputString.charAt(1));
        address[1] = char2byte(inputString.charAt(4), inputString.charAt(3));
        address[0] = char2byte(inputString.charAt(6), inputString.charAt(5));

        Serial.print("K");
        //read
        Serial.print("R");
        temp = 0;

        temp = ((long) address[2]) << (16); //doesn't work with out (long)
        temp |= ((long) address[1]) << (8);
        temp |= (long) address[0];
        
        serialPrintHex(address[2]);
        serialPrintHex(address[1]);
        serialPrintHex(address[0]);

        delay(1);

        for (int i = 0; i < 32; i++) {
            address[2] = byte((temp & 0xFF0000) >> 16);
            address[1] = byte((temp & 0xFF00) >> 8);
            address[0] = byte(temp & 0xFF);
            
            byte r;
            if (address[2] == 0xf0) {
                r = readEEPROM(address[1], address[0]);
            } else {
                r = readFlash(address[2], address[1], address[0]);
            }
            serialPrintHex(r);
            
            temp++;
        }

        nullString();
        Serial.println("X");
    }

    // Say hello
    if (stringComplete && inputString.charAt(0) == 'H') {
        delay(1);
        nullString();
        Serial.print("H");
    }

    if (stringComplete && inputString.charAt(0) == 'C') { //config
        delay(1);

        configWrite(char2byte(inputString.charAt(1), '0'),
                char2byte(inputString.charAt(3), inputString.charAt(2)));

        nullString();
        Serial.print("K");
    }

    if (stringComplete && inputString.charAt(0) == 'D') { //read device
        delay(1);
        uint8_t devID1 = readFlash(0x3f, 0xff, 0xfe) & 0b11100000;
        uint8_t devID2 = readFlash(0x3f, 0xff, 0xff);
        Serial.write(devID1);
        Serial.write(devID2);

        nullString();
        Serial.print("K");
    }

    
    if (stringComplete && inputString.charAt(0) == 'I') { //init device
        initDevice();

        nullString();
        Serial.print("K");
    }

    if (stringComplete && inputString.charAt(0) == 'J') { //read from device
        delay(2);

        /* try to read some data */
        uint8_t devID1 = readFlash(0x3f, 0xff, 0xfe) & 0b11100000;
        uint8_t devID2 = readFlash(0x3f, 0xff, 0xff);

        char test[100];
        sprintf(test, "devid: %04x:%04x\n", devID2, devID1);
        Serial.write(test);

        nullString();
        Serial.print("K");
    }

    //clear string incase the first CHAR isn't known
    char firstchar = inputString.charAt(0);
    if (firstchar != 'H' && firstchar != 'E'
            && firstchar != 'R' && firstchar != 'W'
            && firstchar != 'C' && firstchar != 'D'
            && firstchar != 'I' && firstchar != 'J') {
        nullString();
    }
}

void initDevice(void) {
    digitalWrite(MCLR, LOW);
    delay(10);
    digitalWrite(MCLR, HIGH);
    delay(10);
    digitalWrite(MCLR, LOW);
    delay(P18);

    uint32_t seq = 0x4d434850;

    sendbyte(seq >> 24u);
    sendbyte(seq >> 16u);
    sendbyte(seq >>  8u);
    sendbyte(seq >>  0u);

    delay(P20);
    digitalWrite(MCLR, HIGH);
    delay(P15);
}

byte readFlash(byte usb, byte msb, byte lsb) {

    byte value = 0;

    pinMode(PGD, OUTPUT);

    send4bitcommand (B0000);
    send16bit(0x0e00 | usb);  /* MOVLW usb */
    send4bitcommand(B0000);
    send16bit(0x6ef8);        /* MOVWF TBLPTRU */

    send4bitcommand(B0000);
    send16bit(0x0e00 | msb);  /* MOVLW msb */
    send4bitcommand(B0000);
    send16bit(0x6ef7);        /* MOVWF TBLPTRH */

    send4bitcommand(B0000);
    send16bit(0x0e00 | lsb);  /* MOVLW lsb */
    send4bitcommand(B0000);
    send16bit(0x6ef6);        /* MOVWF TBLPTRL */

    send4bitcommand (B1000);  /* Table Read */

    pinMode(PGD, INPUT);
    digitalWrite(PGD, LOW);

    for (byte i = 0; i < 8; i++) { //dummy read 1 byte
        digitalWrite(PGC, HIGH);
        digitalWrite(PGC, LOW);
    }

    for (byte i = 0; i < 8; i++) { //shift out 1 byte
        digitalWrite(PGC, HIGH);
        digitalWrite(PGC, LOW);
        if (digitalRead(PGD) == HIGH)
            value += 1 << i; //sample PGD
    }

    return value;

}

byte readEEPROM(byte addrH, byte addr) {

    byte value = 0;

    // Step 1: Direct access to data EEPROM
    send4bitcommand (B0000);
    send16bit(0x9ea6);
    send4bitcommand(B0000);
    send16bit(0x9ca6);

    // Step 2: Set the data EEPROM Address Pointer
    send4bitcommand(B0000);
    send16bit(0x0e00 | addr);
    send4bitcommand(B0000);
    send16bit(0x6ea9);

    send4bitcommand(B0000);
    send16bit(0x0e00 | addrH);
    send4bitcommand(B0000);
    send16bit(0x6eaa);

    // Step 3: Initiate a memory read
    send4bitcommand (B0000);
    send16bit(0x80a6);
    
    //Step 4: Load data into the Serial Data Holding register
    send4bitcommand(B0000);
    send16bit(0x50a8);
    send4bitcommand(B0000);
    send16bit(0x6ef5);
    send4bitcommand(B0000);
    send16bit(0x0000);

    send4bitcommand (B0010);
    
    pinMode(PGD, INPUT);
    digitalWrite(PGD, LOW);

    for (byte i = 0; i < 8; i++) { //LSB undefined
        digitalWrite(PGC, HIGH);
        digitalWrite(PGC, LOW);
    }

    for (byte i = 0; i < 8; i++) { //shift out MSB
        digitalWrite(PGC, HIGH);
        digitalWrite(PGC, LOW);
        if (digitalRead(PGD) == HIGH)
            value += 1 << i; //sample PGD
    }

    return value;

}

void send4bitcommand(byte data) {
    pinMode(PGD, OUTPUT);
    for (byte i = 0; i < 4; i++) {
        if ((1 << i) & data)
            digitalWrite(PGD, HIGH);
        else
            digitalWrite(PGD, LOW);
        digitalWrite(PGC, HIGH);
        digitalWrite(PGC, LOW);
    }

}

void sendbyte(byte data) {
    pinMode(PGD, OUTPUT);
    for (byte i = 0; i < 8; i++) {
        if ((0x80 >> i) & data)
            digitalWrite(PGD, HIGH);
        else
            digitalWrite(PGD, LOW);
        digitalWrite(PGC, HIGH);
        digitalWrite(PGC, LOW);
    }
}

uint8_t readbyte(void) {
    pinMode(PGD, INPUT);
    digitalWrite(PGD, LOW);
    uint8_t val = 0;
    for (byte i = 0; i < 8; i++) { //shift out
        digitalWrite(PGC, HIGH);
        digitalWrite(PGC, LOW);
        if (digitalRead(PGD) == HIGH)
            val |= (0x80 >> i); //sample PGD
    }
}

void send16bit(unsigned int data) {
    pinMode(PGD, OUTPUT);
    for (byte i = 0; i < 16; i++) {
        if ((1 << i) & data)
            digitalWrite(PGD, HIGH);
        else
            digitalWrite(PGD, LOW);
        digitalWrite(PGC, HIGH);
        digitalWrite(PGC, LOW);
    }

}

void erase_all() { //for some reason the chip stops respone, just reconnect voltage

    send4bitcommand (B0000);
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

    send4bitcommand (B1100);
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

    digitalWrite(PGD, LOW);

    delay(P11ms);

    digitalWrite(PGD, HIGH);

}


void erase_eeprom() { //for some reason the chip stops respone, just reconnect voltage

    send4bitcommand (B0000);
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

    send4bitcommand (B1100);
    send16bit(0x0000); //

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
    send16bit(0x8484);

    send4bitcommand(B0000); //
    send16bit(0x0000); //

    delay(2);

    send4bitcommand(B0000); //
    send16bit(0x0000); //

    delay(2);

    digitalWrite(PGD, LOW);

    delay(P11ms);

    digitalWrite(PGD, HIGH);

}


void configWrite(byte address, byte data) {

    send4bitcommand (B0000);
    send16bit(0x8ea6);
    send4bitcommand(B0000);
    send16bit(0x8ca6);

    send4bitcommand(B0000);
    send16bit(0x0e30); //
    send4bitcommand(B0000);
    send16bit(0x6ef8);

    send4bitcommand(B0000);
    send16bit(0x0e00); //
    send4bitcommand(B0000);
    send16bit(0x6ef7);

    send4bitcommand(B0000);
    send16bit(0x0e00 | address); //
    send4bitcommand(B0000);
    send16bit(0x6ef6);

    //Serial.println("programming");

    send4bitcommand (B1111);
    if (address & 0x01)
        send16bit(0x0000 | data << 8); // if even MSB ignored, LSB read
    else
        send16bit(0x0000 | data); // if odd MSB read, LSB ignored

    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    delay(P9A);
    digitalWrite(PGC, LOW);
    delayMicroseconds(100);

    send16bit(0x0000);
}

void eepromWrite(byte addrH, byte addr, byte data) {
    // Step 1: direct access to data EEPROM
    send4bitcommand (B0000);
    send16bit(0x9ea6);
    send4bitcommand(B0000);
    send16bit(0x9ca6);

    // Step 2: Set the data EEPROM Address Pointer
    send4bitcommand(B0000);
    send16bit(0x0e00 | addr);
    send4bitcommand(B0000);
    send16bit(0x6ea9);

    send4bitcommand(B0000);
    send16bit(0x0e00 | addrH);
    send4bitcommand(B0000);
    send16bit(0x6eaa);

    // Step 3: Load the data to be written
    send4bitcommand(B0000);
    send16bit(0x0e00 | data);
    send4bitcommand(B0000);
    send16bit(0x6ea8);

    // Step 4: Enable memory writes
    send4bitcommand(B0000);
    send16bit(0x84a6);

    // Step 5: Initiate write
    send4bitcommand(B0000);
    send16bit(0x82a6);

    // Step 6: Poll WR bit, repeat until the bit is clear
    byte wr = 0x01;
    while(wr & 0x01) {
        send4bitcommand(B0000);
        send16bit(0x50a6);
        send4bitcommand(B0000);
        send16bit(0x6ef5);
        send4bitcommand(B0000);
        send16bit(0x0000);

        send4bitcommand(B0010);
        pinMode(PGD, INPUT);
        digitalWrite(PGD, LOW);

        delayMicroseconds(P5us);

        for (byte i = 0; i < 8; i++) { //read
            digitalWrite(PGC, HIGH);
            digitalWrite(PGC, LOW);
        }

        delayMicroseconds(P6us);

        byte value = 0;
        for (byte i = 0; i < 8; i++) { //shift out
            digitalWrite(PGC, HIGH);
            digitalWrite(PGC, LOW);
            if (digitalRead(PGD) == HIGH)
                value += 1 << i; //sample PGD
        }
        wr = value;

        pinMode(PGD, OUTPUT);
        digitalWrite(PGD, LOW);

        delayMicroseconds(P5us);
    }

    // Step 7: Hold PGC low for time P10
    delayMicroseconds(P10us);

    // Step 8: Disable writes
    send4bitcommand (B0000);
    send16bit(0x94a6);
    
    //nop
    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    delay(P9A);
    digitalWrite(PGC, LOW);
    delay(P10);
    delayMicroseconds(100);

    send16bit(0x0000);

    //done
}

void idBuffer() {

    //step 1
    send4bitcommand (B0000);
    send16bit(0x8ea6);
    send4bitcommand(B0000);
    send16bit(0x9ca6);

    //step 2
    send4bitcommand(B0000);
    send16bit(0x0e20);
    send4bitcommand(B0000);
    send16bit(0x6ef8);

    send4bitcommand(B0000);
    send16bit(0x0e00);
    send4bitcommand(B0000);
    send16bit(0x6ef7);

    send4bitcommand(B0000);
    send16bit(0x0e00);
    send4bitcommand(B0000);
    send16bit(0x6ef6);

    //step 3
    send4bitcommand (B1101);
    send16bit(buffer[1] << 8 | buffer[0]);
    send4bitcommand (B1101);
    send16bit(buffer[3] << 8 | buffer[2]);
    send4bitcommand (B1101);
    send16bit(buffer[5] << 8 | buffer[4]);
    send4bitcommand (B1111);
    send16bit(buffer[7] << 8 | buffer[6]);

    //nop
    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    delay(1);
    digitalWrite(PGC, LOW);
    delayMicroseconds(100);

    send16bit(0x0000);

    //done

}

void programBuffer(byte usb, byte msb, byte lsb) {

    if ((lsb & 0x0F) | ((lsb & 0xF0) >> 4) % 2) {
        //Serial.println("Error: Low nibble (refere as HEX) has to be 0, and the second nibble has to be even.");
        //Serial.println("valid examples: 0x000000 , 0x000060, 0x006320, 0x0063E0");
        //Serial.println("INvalid examples: 0x000004 , 0x000014, 0x006328, 0x0063EA");
        //Serial.println("0,2,4,6,8,A,C,E are even");
        return;
    }

    //step 1
    send4bitcommand (B0000);
    send16bit(0x8ea6);      /* BSF   EECON1, EEPGD */
    send4bitcommand(B0000);
    send16bit(0x9ca6);      /* BCF   EECON1, CFGS */

    //step 2
    send4bitcommand(B0000);
    send16bit(0x0e00 | usb);/* MOVLW <Addr[21:16] */
    send4bitcommand(B0000);
    send16bit(0x6ef8);      /* MOVWF TBLPTRU */

    send4bitcommand(B0000);
    send16bit(0x0e00 | msb);/* MOVLW <Addr[15:8]>*/
    send4bitcommand(B0000);
    send16bit(0x6ef7);      /* MOVWF TBLPTRH */

    send4bitcommand(B0000);
    send16bit(0x0e00 | lsb);/* MOVLW <Addr[7:0] */
    send4bitcommand(B0000);
    send16bit(0x6ef6);      /* MOVWF TBLPTRL */

    //step 3: Load write buffer. Repeat for all but the last two bytes
    for (byte i = 0; i < 15; i++) {
        send4bitcommand (B1101);
        send16bit(buffer[(2 * i) + 1] << 8 | buffer[(i * 2)]); /* Write 2 bytes and post-increment address by 2 */
    }

    //step 4: oad write buffer for last two bytes and start programming
    send4bitcommand (B1111);
    send16bit(buffer[31] << 8 | buffer[30]);

    //nop
    digitalWrite(PGD, LOW);
    
    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);

    digitalWrite(PGC, HIGH);
    digitalWrite(PGC, LOW);
    delay(1);

    digitalWrite(PGC, HIGH);
    delay(P9);
    digitalWrite(PGC, LOW);
    delay(P10);
    delayMicroseconds(100);

    send16bit(0x0000);

    //done

}

/////////////////////////////
//nothing special underhere:

void nullString() {
    inputString = "";
    stringComplete = false;
}

void nullBuffer() {
    for (byte i = 0; i < 32; i++)
        buffer[i] = 0xFF;
}

byte char2byte(char lsb, char msb) {

    byte result = 0;

    switch (lsb) {
    case '0':
        result = 0;
        break;
    case '1':
        result = 1;
        break;
    case '2':
        result = 2;
        break;
    case '3':
        result = 3;
        break;
    case '4':
        result = 4;
        break;
    case '5':
        result = 5;
        break;
    case '6':
        result = 6;
        break;
    case '7':
        result = 7;
        break;
    case '8':
        result = 8;
        break;
    case '9':
        result = 9;
        break;
    case 'A':
        result = 0xA;
        break;
    case 'B':
        result = 0xB;
        break;
    case 'C':
        result = 0xC;
        break;
    case 'D':
        result = 0xD;
        break;
    case 'E':
        result = 0xE;
        break;
    case 'F':
        result = 0xF;
        break;
    }

    switch (msb) {
    case '0':
        result |= 0 << 4;
        break;
    case '1':
        result |= 1 << 4;
        break;
    case '2':
        result |= 2 << 4;
        break;
    case '3':
        result |= 3 << 4;
        break;
    case '4':
        result |= 4 << 4;
        break;
    case '5':
        result |= 5 << 4;
        break;
    case '6':
        result |= 6 << 4;
        break;
    case '7':
        result |= 7 << 4;
        break;
    case '8':
        result |= 8 << 4;
        break;
    case '9':
        result |= 9 << 4;
        break;
    case 'A':
        result |= 0xA << 4;
        break;
    case 'B':
        result |= 0xB << 4;
        break;
    case 'C':
        result |= 0xC << 4;
        break;
    case 'D':
        result |= 0xD << 4;
        break;
    case 'E':
        result |= 0xE << 4;
        break;
    case 'F':
        result |= 0xF << 4;
        break;
    }

    return result;

}


uint16_t checkPIC()
{
    uint8_t devid1 = readFlash(0x3f, 0xff, 0xfe);
    uint8_t devid2 = readFlash(0x3f, 0xff, 0xff);
    return ((devid2 << 8) || devid1);
    /*
     if( readFlash(0x3f,0xff,0xff) == 0x12 )
     return 1;
     else
     return 0;*/

}

void serialPrintHex(byte b) { 
    if (b<0x10) {
        Serial.print("0");
    }
    Serial.print(b, HEX); 
}
