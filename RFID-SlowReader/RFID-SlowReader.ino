/*
 *  RFID Reader to Slow Reader
 *  
 *  The standard Weigand protocol is too fast for a Raspberry Pi to 
 *  reliably get data. Thus we'll capture a read here on the Arduino
 *  then slowly send the data to the Raspberry Pi.
 * 
 */
#define PIN_RFID_DATA0 2            //From the RFID Reader (Green)
#define PIN_RFID_DATA1 3            //From the RFID Reader (White)

#define PIN_PIE_DATA0 7             //From the RPi (data0)
#define PIN_PIE_DATA1 8             //From the RPi (data1)
#define PIN_PIE_NEXTBIT 9           //From the RPi (ACK PIN)


#define MAX_BITS 50                 // max number of bits (only need 26 or 35 for our cards)
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.  
 
unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char lastcard[MAX_BITS];    // copy of the last card read bits
unsigned char bitCount;              // number of bits currently captured
unsigned char lastcardBitCount;      // total number of bits in the last card read
unsigned char lastcardCurrentBit;    // how many bits of the last card have been read

unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits


 
// interrupt that happens when INTO goes low (0 bit)
void ISR_INT0()
{
  //Serial.print("0");   // uncomment this line to display raw binary
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
 
}
 
// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1()
{
  //Serial.print("1");   // uncomment this line to display raw binary
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}

// interrupt that happens when INT1 goes low (1 bit)
void ISR_READ_REMOTE_BIT()
{
  Serial.print(".");   // uncomment this line to display raw binary
  setNextReadBit();
}

void setup()
{
  pinMode(10, OUTPUT);  // LED
  pinMode(PIN_RFID_DATA0, INPUT);     // DATA0 (INT0)
  pinMode(PIN_RFID_DATA1, INPUT);     // DATA1 (INT1)

  pinMode(PIN_PIE_DATA0, OUTPUT);     // Sending data to the Raspberry Pi
  pinMode(PIN_PIE_DATA1, OUTPUT);     // Sending data to the Raspberry Pi
  digitalWrite(PIN_PIE_DATA0, LOW);
  digitalWrite(PIN_PIE_DATA0, LOW);

  pinMode(PIN_PIE_NEXTBIT, INPUT);
 
  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(0, ISR_INT0, FALLING);  
  attachInterrupt(1, ISR_INT1, FALLING);
  
  Serial.begin(115200);
  Serial.println("Setup Finished. Lets read some RFID cards.");
  
  weigand_counter = WEIGAND_WAIT_TIME;
}
 
void loop()
{
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--weigand_counter == 0){
      flagDone = 1;  
      Serial.println("Done Reading Flag");
    }
  }

  // Check if the Raspberry PI Ack Pin is high
  // If so, wait and then set the next bit of the RFID
  if (digitalRead(PIN_PIE_NEXTBIT)){
    while (digitalRead(PIN_PIE_NEXTBIT)){
      delay(2);
    }
    setNextReadBit();
  }
    
 
  // Reading RFID from tag
  // If we have some bits and hit the time out, then process the bits
  if (bitCount > 0 && flagDone) {
 
    Serial.print("Read ");
    Serial.print(bitCount);
    Serial.print(" bits. ");

    copyBitsToLastCard();
 
     // cleanup and get ready for the next card
     bitCount = 0;
     for (int i=0; i<MAX_BITS; i++) 
     {
       databits[i] = 0;
     }

     //Serial.println("Cleared Bits");
  }
}


// Sets the next bits for the Raspberry Pi to read from us
// When we hit the of the RFID bits, both lines will go low
void setNextReadBit(){
  if (lastcardBitCount == lastcardCurrentBit) {
    digitalWrite(PIN_PIE_DATA0, LOW);
    digitalWrite(PIN_PIE_DATA1, LOW);

    // Might not need to clear bit, but lets keep this consistent for now
    for (int i=0; i<MAX_BITS; i++) {
       lastcard[i] = 0;
    } 
    Serial.println("All Bits Read");
    return;
  }

  if (lastcard[lastcardCurrentBit] == 0){
     digitalWrite(PIN_PIE_DATA0, HIGH);
     digitalWrite(PIN_PIE_DATA1, LOW);
  } else {
     digitalWrite(PIN_PIE_DATA0, LOW);
     digitalWrite(PIN_PIE_DATA1, HIGH);
  }

  Serial.print("+");
  lastcardCurrentBit++;
}


// Utility function to copy read RFID bits to another array for the Raspberry PI
void copyBitsToLastCard()
{
     
     lastcardBitCount = bitCount;
     for (int i=0; i<bitCount; i++) 
     {
       lastcard[i] = databits[i];
     }

     lastcardCurrentBit = 0;
     setNextReadBit();
}
