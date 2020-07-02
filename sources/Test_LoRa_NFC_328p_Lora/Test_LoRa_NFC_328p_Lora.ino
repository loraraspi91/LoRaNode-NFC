
//  ----------------------------------------------------------------------

#include <Wire.h> /* Indispensable pour la gestion de l'I2C */
#include <SPI.h>
#include <LoRa.h>
     
#define RFM95_CS 8
#define RFM95_RST 7
#define RFM95_INT 2
#define LED 9

#define A_Vref 1126400L  // 1,1 * 1024 *1000 =1126400L (apply coef to reality) display (3645) , measured (3614)= coef = 0,991495

byte msgCount = 0;            // count of outgoing messages
int interval = 2000;          // interval between sends
long lastSendTime = 0;        // time of last packet send

     
// Change to 434.0 or other frequency, must match RX's freq!
#define LORA_FREQ 868E6

void test_voltage(void);
void init_Lora();
void test_Lora();

void setup()
{
  /* Les variables du programme */
  byte adresse;       
  byte resultat;      
  byte nb_existe = 0; 
  byte nb_defaut = 0; 

  Wire.begin();       
  Serial.begin(9600); 

  Serial.println("  ____                             ___   ____     ____ ");
  Serial.println(" / ___|    ___    __ _   _ __     |_ _| |___ \\   / ___|");
  Serial.println(" \\___ \\   / __|  / _` | | '_ \\     | |    __) | | |    ");
  Serial.println("  ___) | | (__  | (_| | | | | |    | |   / __/  | |___ ");
  Serial.println(" |____/   \\___|  \\__,_| |_| |_|   |___| |_____|  \\____|");
  Serial.println();

  Serial.println("\nBegin scan"); 

    Serial.print("\t.... ");

  /* I2C scan begin */
  for (adresse = 1; adresse < 128; adresse++ )
  {
    Wire.beginTransmission(adresse);    /* Commence une transmission a l'adresse indiquee */
    resultat = Wire.endTransmission();
    /* resultat = result of transmission
        0 : success (device OK)
        1 : data too long for the buffer (error)
        2 : NACK received on the address (no device)
        3 : NACK received on data transmission (no device)
        4 : other error (so Error) */

    if (resultat == 0)  
    {
      if (adresse < 16)Serial.print("0x0");
      else Serial.print("0x");
      Serial.print(adresse, HEX);
      Serial.print(" ");
      nb_existe++; 
    }
    else if ((resultat == 4) || (resultat == 1)) {
  
      Serial.print("#### ");
      nb_existe++;  
      nb_defaut++;  
    }
    else  Serial.print(".... ");
    delay(25); 

    if (((adresse + 1) % 8) == 0) 
    {
      Serial.println();   
      if (adresse < 255)Serial.print("\t");
      
    }
  }

  /* end of I2C scan */
  Serial.println("End of I2C Scan\n");

  if (nb_existe == 0)Serial.println("no I2C Device !");
  else
  {
   
    Serial.print(nb_existe); 
    if (nb_existe < 2)Serial.print(" device identified"); 
    else Serial.print(" devices identified");       

    if (nb_defaut != 0)  
    {
      Serial.print(" almong ");  
      Serial.print(nb_defaut); 
      if (nb_defaut < 2)Serial.println(" device in error."); 
      else Serial.println(" devices in errors.");            
    }
  }
  
  Serial.println(); /* Saut de ligne supplémentaire */
  
  // TEST the LED 
  pinMode(LED, OUTPUT);
 /* Test led */
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(2000);
  digitalWrite(LED, LOW);   // turn the LED on (HIGH is the voltage level)
  
   test_voltage();

   init_Lora();
}


void loop()
{

  if (millis() - lastSendTime > interval) {
    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    
    String message = "HeLoRa World ! ";   // send a message
    message += msgCount;
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();            // timestamp the message
    interval = random(3000) + 1000;    // 2-3 seconds
    
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());       
}


void test_voltage() {
   float val = 0;
   long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = A_Vref / result; // Back-calculate AVcc in mV 1126400 = 1.1 * 1024
  val = (float) result;
  Serial.print("Voltage = ");
   Serial.print(val/1000);
   Serial.println(" V");
}

void init_Lora() {
      
      Serial.println("Arduino LoRa TX/RX Test!");
     
       //setup LoRa transceiver module
       LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);
  

       while (!LoRa.begin(LORA_FREQ)) {
        Serial.println("LoRa init failed. Check your connections.");
        delay(500);
       }
      Serial.println("LoRa radio init OK!");
     
      Serial.print("Set Freq to: "); Serial.println(LORA_FREQ);
      // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
     
      LoRa.setSyncWord(0xF3);           // ranges from 0-0xFF, default 0x34, see API docs
      Serial.println("LoRa init succeeded.");
    }


void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  String incoming = "";
 
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
 
  Serial.println("Message received: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}
