
/*
 ESP-NOW communication example by Dani No www.esploradores.com

*** THIS SKETCH MATCHES TO THE MAESTRO ***

The sketch allows the sending of two data by means of ESP-NOW communication to a slave pair:

 - In the variable -potentiometer- the data of the input reading is sent
   Analogue of ESP. Read and send -values ​​between 0 and 1023- that are made vary
   By means of a potentiometer and will be used to regulate the intensity of the connected LED
   In the slave circuit.
 - In the variable - time - the data of the millisecond is sent in which the one that is sent
   The data to the slave circuit.

 IMPORTANT!!!

To make ESP-NOW communication work if it is installed in Arduino as a manager
Of cards the: esp8266 by ESP8266 Community version 2.3.0 (You can check it by searching for it:
Tools-> Board: "NodeMCU 1.0 (ESP-12E Module)" -> Board Manager ..., it is necessary to edit
The file: ~ / Library / Arduino15 / packages / esp8266 / hardware / esp8266 / 2.1.0 / platform.txt,
Search for "compiler.c.elf.libs", and add it to the end of the "-lespnow" line.
*/

#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
}

// *** STRUCTURE OF TRANSMITTED DATA MASTER / SLAVE *** //
// Set to EQUAL on the slave pair
    struct STRUCT_DATA {
    int potenciomtr;
    int temp;
    };

    //Joystick code
    int    joystick_xmax = 1023;
    float  joystick_xmult = 1;
    int    joystick_ymax = 1023;
    float  joystick_ymult = 1;


void setup() {

  
  // *** INITIALIZATION OF THE PORT SERIES *** //
  Serial.begin(115200); Serial.println();Serial.println();

   
  
  // *** INITIALIZING THE ESP-NOW PROTOCOL *** //
  if (esp_now_init()!=0) {
    Serial.println("*** ESP_Now init failed");
    ESP.restart();
    delay(1);
  }

  
  // *** MAC DATA (Access Point and Station) of the ESP *** //
  Serial.print("Access Point MAC de este ESP: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("Station MAC de este ESP: "); Serial.println(WiFi.macAddress());


    // *** DECLARATION OF THE ROLE OF THE ESP DEVICE IN THE COMMUNICATION *** //
    // 0 = LEFT, 1 = MASTER, 2 = SLAVE and 3 = MASTER + SLAVE
  esp_now_set_self_role(1);   


    // *** PICK-UP WITH THE SLAVE *** //
    // MAC address of the ESP that is paired (slave)
    // The corresponding AP MAC must be entered   ************************
    uint8_t mac_addr[6] = {0x5E, 0xCF, 0x7F, 0x03, 0x8B, 0x78};    
    uint8_t role=2;  
    uint8_t channel=1;
    uint8_t key[0]={};   //No key
    //uint8_t key[16] = {0,255,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    uint8_t key_len=sizeof(key);
    Serial.print("*Key size: "); 
	Serial.println(key_len);
  esp_now_add_peer(mac_addr,role,channel,key,key_len);
}

void loop() {


  // *** DATA TO SEND *** //
  STRUCT_DATA ED;                     
    ED.potenciomtr = get_joystick_x();
    Serial.print("Data X: "); Serial.print(ED.potenciomtr);
    delay(20);
    ED.temp = get_joystick_y();
    Serial.print(". Data Y: "); Serial.print(ED.temp);


    // *** SENDING THE DATA *** //
    // uint8_t * da = NULL; // NULL sends data to all ESPs with which it is paired
    uint8_t da[6] = {0x5E, 0xCF, 0x7F, 0x03, 0x8B, 0x78};
    uint8_t data[sizeof(ED)]; memcpy(data, &ED, sizeof(ED));
    uint8_t len = sizeof(data);
  esp_now_send(da, data, len);

  delay(3); //If data is lost at reception, this value must be raised


// *** VERIFICATION OF CORRECT RECEIPT OF DATA FOR SLAVE *** //
  esp_now_register_send_cb([](uint8_t* mac, uint8_t status) {
    char MACesclavo[6];
    sprintf(MACesclavo,"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    Serial.print(" Posted to ESP MAC: "); Serial.print(MACesclavo);
    Serial.print(". Recepcion (0=0K - 1=ERROR): "); Serial.println(status);
  });
}


void calibrateonce()
{
  static boolean calib_done=0;
  if(!calib_done)
  {
     Serial.println("*** Calibrating Joystic START .");
     calibrate();
     Serial.println("*** Calibrating Joystic END");
     calib_done = 1;
  }
}


static void calibrate()
{
  int cnt = 0;
  int xmax = 0;
  int xmin = 1023;
  int ymax = 0;
  int ymin = 1023;
  while(cnt < 4)
  {
     int jx = joystick_xmax-get_joystick_x();
     int jy = joystick_ymax-get_joystick_y();
     if((cnt % 2 == 0) && jx < 450) 
       cnt++;
     if((cnt % 2 == 1) && jx > 800) 
       cnt++;
     xmax = std::max(xmax, jx);
     xmin = std::min(xmin, jx);
     ymax = std::max(ymax, jy);
     ymin = std::min(ymin, jy);
  }
  joystick_xmax = xmax;
  joystick_xmult = 1023.0/(xmax - xmin);
  joystick_ymax = ymax;
  joystick_ymult = 1023.0/(ymax - ymin);
}

static unsigned int get_joystick_x()
{
  pinMode(14,INPUT);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  return (unsigned int)std::max(0, std::min(1023,(int)((joystick_xmax-analogRead(A0))*joystick_xmult)));
}
static unsigned int get_joystick_y()
{
  pinMode(13,INPUT);
  pinMode(14,OUTPUT);
  digitalWrite(14,LOW);
  return (unsigned int)std::max(0, std::min(1023,(int)((joystick_ymax-analogRead(A0))*joystick_ymult)));
}

