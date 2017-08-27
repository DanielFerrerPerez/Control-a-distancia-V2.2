//--------------DEFINE----------------------------------------
#define DHT22_PIN         D7
#define DHT22_PIN1        D0
#define RELE_1_PIN        D1
#define RELE_2_PIN        D2
#define IN_1_PIN          D3
#define IN_2_PIN          D5
#define IN_3_PIN          D6
#define LED_VERDE         D8
// Initializa Telegram BOT
#define BOTtoken "XXXXXXXXX:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"  // tú Bot Token (Se consigue desde Botfather) 

//--------------VARIABLES-------------------------------------
/* Periodicas */
const unsigned long MAXUL = 4294967295UL; 
unsigned long LT[10] = {0,0,0,0,0,0,0,0,0,0};
unsigned long MS[10] = {100,200,500,1000,2000,5000,10000,30000,60000,600000};
                                                                   

/* DHT22 */
bool  inestable;
bool  arranque;

/* DHT22 */
float humedad_filtrada1;
float temperatura_filtrada1;
float humedad_filtrada2;
float temperatura_filtrada2;

/* ThingSpeak */
unsigned long myChannelNumber2 = XXXXXX;
const char * myWriteAPIKey2 = "XXXXXXXXXXXXXXXX";
char   thingSpeakAddress[] = "api.thingspeak.com";
const char * idTelegram = "XXXXXXXXX";  

long    lastConnectionTime = 0; 
boolean      lastConnected = false;
int          failedCounter = 0;

/* Señales de entrada y de salida */
bool RELE1;
bool RELE2;
bool IN1; 
bool IN2;
bool IN3;
bool IN1_last;
bool IN2_last;
bool IN3_last;
bool RELE1_last;
bool RELE2_last;

char buf[12];           /* Auxiliar */

// SSID Y CONTRASEÑAS
const char* ssid[5] =     {  "SSID_1" ,      "SSID_2" ,        "SSID_3" ,       "SSID_4" ,       "SSID_5"          };
const char* password[5] = {  "PASSWORD_1" ,  "PASSWORD_2" ,    "PASSWORD_3" ,   "PASSWORD_4" ,   "PASSWORD_5" ,    };



