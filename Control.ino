/*
  Inicio: 2017/02/28  Daniel Ferrer
  Fin: 2017/08/27

  Versión 2.2 Control a distancia
  Realizado en el IDE de Arduino 1.6.9 con el plugging de la tarjeta Wemos D1 Mini 

  Componentes:
  - 1 WEMOS D1 mini https://www.wemos.cc/product/d1-mini-pro.html
  - 2 Sensor de temperatura y humedad DHT22 http://www.ebay.es/itm/DHT22-Digital-Temperature-Humidity-Sensor-Module-for-Arduino-DHT-22-raspberry-pi-/322222315164?hash=item4b05f2569c&_uhb=1
  - 2 Resistencia de 10kohm (en caso de tener los DHT22 sin la placa shield de wemos)
  - 2 Condensador de 10nF   (en caso de tener los DHT22 sin la placa shield de wemos)
  - 1 mt de cable rigido de un hilo
  - 17 fichas de empalme de 2,5mm
  - Caja diseño http://www.thingiverse.com/thing:2039876
  
  Circuito:
    D0 - Entrada sensor de temperatura DHT22 exterior vivienda
    D1 - Salida Relé C1
    D2 - Salida Relé C2
    D3 - Entrada I1
    D4 - Salida led placa WEMOS 
    D5 - Entrada I2
    D6 - Entrada I3
    D7 - Entrada sensor de temperatura DHT22 interior vivienda
    D8 - Salida piloto verde
  
  Descripción del proyecto:
    Permite el control a través de Telegram de 2 salidas digitales (calefacción, luz, etc...).
    Mantiene el estado de las salidas digitales después de realizar un corte de luz o reset.
    Permite conocer el estado e histórico a través de Telegram de 3 señales digitales (sensores, puertas, etc...).
    Permite la lectura y guardado (ThingSpeak) de dos temperaturas y humedades, una exterior y otra interior de la vivienda.
    Permite la conexión en diferentes redes Wifi, en caso de no tener señal de alguna de ellas se cambia automáticamente a otra
    que tenga señal dentro de las 5 que puede almacenar, además permite el cambio de red desde Telegram e identifica si tiene conexión real con internet.
    Permite configurar el periodo de consulta de mensajes entrantes al chat para reducir el consumo de megas (para caso de tarjetas prepago).
   
   Links de librerias:
   https://github.com/adafruit/DHT-sensor-library
   https://github.com/dancol90/ESP8266Ping
   https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
   https://github.com/mathworks/thingspeak-arduino
   https://github.com/bblanchon/ArduinoJson
 */

/*----------------- LIBRERIAS ----------------*/
#include <dht.h>                  /*    DHT22 temperatura y humedad         */
#include <ESP8266WiFi.h>          /*    Acceso a internet a través de WIFI  */
#include <ESP8266Ping.h>          /*    Permite realizar una comprobación si existe internet */
#include <WiFiClient.h>           /*    Modo cliente ThingSpeak             */
#include <WiFiClientSecure.h>     /*    Libreria Telegram                   */
#include <UniversalTelegramBot.h> /*    Libreria Telegram                   */
#include <ThingSpeak.h>           /*    Datos en la nube                    */
#include <EEPROM.h>               /*    Para guardar datos remanentes       */
#include "variables.h"            /*    variables                           */
#include "funciones.h"            /*    Funciones                           */

/*----------------- INSTANCIAS ----------------*/
WiFiClient  client;                           /* Wifi cliente para Thingspeak          */
dht DHT_INT;                                  /* Sensor temperatura y humedad interior */
dht DHT_EXT;                                  /* Sensor temperatura y humedad exterior */

WiFiClientSecure client2;                     /* Cliente de Telegram */
UniversalTelegramBot bot(BOTtoken, client2);  /* Creación del objeto Telegram */
long Bot_mtbs = 30000;                        /* Tiempo entre dos colsultas de mensaje entrante de Telegram*/
long ultimo_tiempo_sin_mensajes = 0;          /* 0 Tiempo por defecto */
bool ultimo_mensaje = LOW;                    /* Memoria para saber que ha habido una consulta*/
long Bot_lasttime;                            /* Auxiliar de tiempo entre dos mensajes Telegram */
byte periodo_consulta = 0;
byte retardo_volver_a_reposo = 0;

/*---------------- VARIABLES GLOBALES ------------------*/
bool inicio = LOW;
int red_wifi = 0;
int red_wifi_ant = 0;
int redwifialmacenada = 6;
int repeticiones = 0;
bool piloto = LOW;
int address = 0;
byte value;
const char* remote_host = "www.google.com";

/*--------------- FUNCIONES -----------------------*/

/* Inicialización de las variables  */
void inicializacion_variables() {
  inicio = HIGH;
  red_wifi = 0;
  redwifialmacenada = 6;
  repeticiones = 0;
  piloto = LOW;
  inestable = HIGH;
}

/* Cuando se llama a esta función, se cambia de red Wifi.
 Se llama cuando no hay red Wifi disponible. */
void cambiar_red_wifi() {
  redwifialmacenada = 6;
  if(red_wifi >= 5) {
    red_wifi = 0;
  }
  WiFi.begin(ssid[red_wifi], password[red_wifi]);
  Serial.println("");
  Serial.print("Intentando conectar con: ");
  Serial.println(ssid[red_wifi]);
  while (WiFi.status() != WL_CONNECTED && repeticiones<=15) {
    Serial.print(".");
    piloto_cambia();
    delay(500);
    repeticiones++;
  }
  repeticiones = 0;
  red_wifi++;
}

/* Funcion piloto cambia, cuando es llamado, cambia el estado del piloto
   verde de encendido a apagado y viceversa.  */
void piloto_cambia() {
  if(piloto==LOW) {
    piloto=HIGH;
  } else {
    piloto=LOW;
  }
  digitalWrite(LED_VERDE,piloto);
}

/* Se llama al arrancar el Wemos y una sola vez. */
void setup() {
/* Configuración de entradas y salidas */
  pinMode(RELE_1_PIN, OUTPUT);
  pinMode(RELE_2_PIN, OUTPUT);
  pinMode(LED_VERDE,  OUTPUT);
  pinMode(IN_1_PIN , INPUT_PULLUP);
  pinMode(IN_2_PIN , INPUT_PULLUP);
  pinMode(IN_3_PIN , INPUT_PULLUP);

  inicializacion_variables();
  
  // Inicializamos el puerto serie para depurar
  Serial.begin(9600);      
  // Configuramos el WIFI como estación
  WiFi.mode(WIFI_STA);
  // Arrancamos la EEPROM
  EEPROM.begin(512);
  // Leemos el último estado de la Eprom de como están los relés
  address = 0;
  value = EEPROM.read(address);
  if (value == 1) {
    digitalWrite(RELE_1_PIN,HIGH); 
  } else {
    digitalWrite(RELE_1_PIN,LOW);  
  }
  address = 1;
  value = EEPROM.read(address);
  if (value == 1) {
    digitalWrite(RELE_2_PIN,HIGH); 
  } else {
    digitalWrite(RELE_2_PIN,LOW);  
  }
  address = 2;                 // Valor del periodo de scan
  value = EEPROM.read(address);
  periodo_consulta = value;

  address = 3;                 // retardo volver al valor de reposo
  value = EEPROM.read(address);
  retardo_volver_a_reposo = value;

  delay(2000);  /*Estabiliza tensiones en las entradas*/
}

void OB35() { /*100ms*/

}

void OB36() { /*200ms*/
 
}

void OB37() { /*500ms*/

}

void OB38() { /*1s*/

}

void OB39() { /*2s*/

}

void OB40() { /*5s*/

}

void OB41() { /*10s*/
  int chk = DHT_INT.read22(DHT22_PIN);  
  int chk1 = DHT_EXT.read22(DHT22_PIN1);  
  filtro1(DHT_INT.temperature,DHT_INT.humidity); 
  filtro2(DHT_EXT.temperature,DHT_EXT.humidity); 
  Serial.print("T1=");
  Serial.println(temperatura_filtrada1);
  Serial.print("H1=");
  Serial.println(humedad_filtrada1);
  Serial.print("T2=");
  Serial.println(temperatura_filtrada2);
  Serial.print("H2=");
  Serial.println(humedad_filtrada2);
  Serial.println("");
}

void OB42() { /*30s*/

}

void OB43() { /*1m*/

}

void OB44() { /*10m*/
  /*Verificamos si existe internet, si no lo hay, busca otra red WiFi*/
  if(Ping.ping(remote_host)) {
    Serial.println("Hay internet");
  } else {
    Serial.println("NO HAY INTERNET");
    cambiar_red_wifi();
  }

  if(inestable == LOW) {  /* Cada 10 minutos, enviamos a thinkspeak los valores filtrados siempre después de que ha pasado 2 minutos del arranque*/
    ThingSpeak.setField(1,temperatura_filtrada2);             /* Dato de la temperatura actual a enviar a thinkspeak */
    ThingSpeak.setField(2,humedad_filtrada2);                 /* Dato de la humedad actual a enviar a thinkspeak */
    ThingSpeak.setField(3,temperatura_filtrada1);             /* Dato de la temperatura actual a enviar a thinkspeak */
    ThingSpeak.setField(4,humedad_filtrada1);                 /* Dato de la humedad actual a enviar a thinkspeak */ 
    ThingSpeak.writeFields(myChannelNumber2, myWriteAPIKey2); /* Enviamos los datos */
    Serial.println("Datos a ThingSpeak subidos. Esto ocurre cada 10 min");
  }
}


void handleNewMessages(int numNewMessages) {
 /*Nuevos mensajes están llegando por Telegram*/
  Serial.println("Nuevos mensajes han llegado");
  Serial.println(String(numNewMessages));
  String red1 = String(ssid[0]);
  String red2 = String(ssid[1]);
  String red3 = String(ssid[2]);
  String red4 = String(ssid[3]);
  String red5 = String(ssid[4]);
  red1.replace("-","_");
  red2.replace("-","_");
  red3.replace("-","_");
  red4.replace("-","_");
  red5.replace("-","_");
  /*Vamos leyendo mensaje a mensaje*/
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);    /*Codigo de identificación de la persona que manda mensaje al chat del bot*/
    String text = bot.messages[i].text;                  /*Mensaje que envia la persona al Bot*/

    String from_name = bot.messages[i].from_name;        /*Nombre de la persona que envia el mensaje al Bot*/
    if (from_name == "") from_name = "Usuario";          /*En caso de estar en blanco el nombre del mensaje, se refiere a el como Usuario*/

    if (chat_id != idTelegram) {                     /*Si el identificador de la persona no coincide con el que tenemos guardado, le da un mensaje de error*/
      bot.sendMessage(chat_id, "Hola " + from_name + ". No esta usted registrado. Debe registrarse primero. Su ID es: " + chat_id); 
    }

    if (chat_id == idTelegram && text == "/MENU") {  /*Si el identificador de la persona y el texto que escribe es /MENU, muestra las posibilidades en pantalla*/
      String mensaje = "Buenos días ";
      mensaje += from_name;
      mensaje += ". Este es el menu de selección:\n\n";
      mensaje += "/RELE1_ON Encender Rele 1\n\n";
      mensaje += "/RELE1_OFF Apagar Rele 1\n\n";
      mensaje += "/RELE2_ON Encender Rele 2\n\n";
      mensaje += "/RELE2_OFF Apagar Rele 2\n\n";
      mensaje += "/ESTADO Estado general\n\n";
      mensaje += "/TECLADO Activa teclado rápido\n\n";
      mensaje += "/WIFI Cambiar red WIFI\n\n";
      mensaje += "/DATOS Cambiar parametros de consumo de datos";
      bot.sendMessage(chat_id,mensaje);                 /*Envia el mensaje de respuesta*/
    }

    if (chat_id == idTelegram && text == "/DATOS") { /*En el caso de que escriba el comando DATOS*/
      String mensaje = "Actualmente tiene los siguientes tiempos configurados:\n\n";
      mensaje += "Espacio de tiempo entre consulta y consulta cuando esta en modo de espera (T1): " + String(periodo_consulta) + " segundos." + "\n\n";
      mensaje += "Tiempo de retardo para volver al modo de espera (T2): " + String(retardo_volver_a_reposo) + " segundos." + "\n\n";
      mensaje += "Cambiar tiempo entre consultas:\n/TIEMPO_T1 \n\n";
      mensaje += "Cambiar retardo modo espera:\n/TIEMPO_T2 \n\n";
      mensaje += "Volver al menu principal: /MENU";
      bot.sendMessage(chat_id,mensaje);
    }

    if (chat_id == idTelegram && text == "/TIEMPO_T1") { /*En el caso de que escriba el comando TIEMPO_T1*/
      String mensaje = "Elija la opción deseada:\n";
      mensaje += "/T1_1 = 10 segundos \n\n";
      mensaje += "/T1_2 = 20 segundos \n\n";
      mensaje += "/T1_3 = 30 segundos \n\n";
      mensaje += "/T1_4 = 40 segundos \n\n";
      mensaje += "/T1_5 = 50 segundos \n\n";
      mensaje += "/T1_6 = 60 segundos \n\n";
      mensaje += "/T1_7 = 70 segundos \n\n";
      mensaje += "/T1_8 = 80 segundos \n\n";
      mensaje += "/T1_9 = 90 segundos \n\n";
      bot.sendMessage(chat_id,mensaje);
    }

    if (chat_id == idTelegram && text == "/TIEMPO_T2") { /*En el caso de que escriba el comando TIEMPO_T2*/
      String mensaje = "Elija la opción deseada:\n";
      mensaje += "/T2_1 = 10 segundos \n\n";
      mensaje += "/T2_2 = 20 segundos \n\n";
      mensaje += "/T2_3 = 30 segundos \n\n";
      mensaje += "/T2_4 = 40 segundos \n\n";
      mensaje += "/T2_5 = 50 segundos \n\n";
      mensaje += "/T2_6 = 60 segundos \n\n";
      mensaje += "/T2_7 = 70 segundos \n\n";
      mensaje += "/T2_8 = 80 segundos \n\n";
      mensaje += "/T2_9 = 90 segundos \n\n";
      bot.sendMessage(chat_id,mensaje);
    }

    if (chat_id == idTelegram && text == "/T1_1") {
      address = 2;
      value = 10;
      EEPROM.write(address, value);
      EEPROM.commit();
      periodo_consulta = 10;
      bot.sendMessage(chat_id, "El nuevo valor de T1 es de 10 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T1_2") {
      address = 2;
      value = 20;
      EEPROM.write(address, value);
      EEPROM.commit();
      periodo_consulta = 20;
      bot.sendMessage(chat_id, "El nuevo valor de T1 es de 20 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T1_3") {
      address = 2;
      value = 30;
      EEPROM.write(address, value);
      EEPROM.commit();
      periodo_consulta = 30;
      bot.sendMessage(chat_id, "El nuevo valor de T1 es de 30 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T1_4") {
      address = 2;
      value = 40;
      EEPROM.write(address, value);
      EEPROM.commit();
      periodo_consulta = 40;
      bot.sendMessage(chat_id, "El nuevo valor de T1 es de 40 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T1_5") {
      address = 2;
      value = 50;
      EEPROM.write(address, value);
      EEPROM.commit();
      periodo_consulta = 50;
      bot.sendMessage(chat_id, "El nuevo valor de T1 es de 50 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T1_6") {
      address = 2;
      value = 60;
      EEPROM.write(address, value);
      EEPROM.commit();
      periodo_consulta = 60;
      bot.sendMessage(chat_id, "El nuevo valor de T1 es de 60 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T1_7") {
      address = 2;
      value = 70;
      EEPROM.write(address, value);
      EEPROM.commit();
      periodo_consulta = 70;
      bot.sendMessage(chat_id, "El nuevo valor de T1 es de 70 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T1_8") {
      address = 2;
      value = 80;
      EEPROM.write(address, value);
      EEPROM.commit();
      periodo_consulta = 80;
      bot.sendMessage(chat_id, "El nuevo valor de T1 es de 80 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T1_9") {
      address = 2;
      value = 90;
      EEPROM.write(address, value);
      EEPROM.commit();
      periodo_consulta = 90;
      bot.sendMessage(chat_id, "El nuevo valor de T1 es de 90 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T2_1") {
      address = 3;
      value = 10;
      EEPROM.write(address, value);
      EEPROM.commit();
      retardo_volver_a_reposo = 10;
      bot.sendMessage(chat_id, "El nuevo valor de T2 es de 10 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T2_2") {
      address = 3;
      value = 20;
      EEPROM.write(address, value);
      EEPROM.commit();
      retardo_volver_a_reposo = 20;
      bot.sendMessage(chat_id, "El nuevo valor de T2 es de 20 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T2_3") {
      address = 3;
      value = 30;
      EEPROM.write(address, value);
      EEPROM.commit();
      retardo_volver_a_reposo = 30;
      bot.sendMessage(chat_id, "El nuevo valor de T2 es de 30 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T2_4") {
      address = 3;
      value = 40;
      EEPROM.write(address, value);
      EEPROM.commit();
      retardo_volver_a_reposo = 40;
      bot.sendMessage(chat_id, "El nuevo valor de T2 es de 40 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T2_5") {
      address = 3;
      value = 50;
      EEPROM.write(address, value);
      EEPROM.commit();
      retardo_volver_a_reposo = 50;
      bot.sendMessage(chat_id, "El nuevo valor de T2 es de 50 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T2_6") {
      address = 3;
      value = 60;
      EEPROM.write(address, value);
      EEPROM.commit();
      retardo_volver_a_reposo = 60;
      bot.sendMessage(chat_id, "El nuevo valor de T2 es de 60 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T2_7") {
      address = 3;
      value = 70;
      EEPROM.write(address, value);
      EEPROM.commit();
      retardo_volver_a_reposo = 70;
      bot.sendMessage(chat_id, "El nuevo valor de T2 es de 70 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T2_8") {
      address = 3;
      value = 80;
      EEPROM.write(address, value);
      EEPROM.commit();
      retardo_volver_a_reposo = 80;
      bot.sendMessage(chat_id, "El nuevo valor de T2 es de 80 segundos"); 
    }

    if (chat_id == idTelegram && text == "/T2_9") {
      address = 3;
      value = 90;
      EEPROM.write(address, value);
      EEPROM.commit();
      retardo_volver_a_reposo = 90;
      bot.sendMessage(chat_id, "El nuevo valor de T2 es de 90 segundos"); 
    }

    if (chat_id == idTelegram && text == "/WIFI") { /*En el caso de que escriba el comando WIFI*/
      String mensaje = "El equipo está conectado a:\n";
      mensaje += WiFi.SSID();
      mensaje +=" con una potencia de señal de: " + String(WiFi.RSSI()+130) + "%\n\n";
      mensaje +="Para cambiar de red pulse una de las siguientes opciones:\n\n";
      mensaje +="/" + red1 + "\n\n";
      mensaje +="/" + red2 + "\n\n";
      mensaje +="/" + red3 + "\n\n";
      mensaje +="/" + red4 + "\n\n";
      mensaje +="/" + red5 + "\n\n";
      mensaje +="Nota: Durante unos segundos se perderá la conexión.\n\n";
      mensaje +="Volver al menu principal: /MENU";
      bot.sendMessage(chat_id,mensaje);
    }  

    if (chat_id == idTelegram && text == "/" + red1) {  /*En el caso de que escriba la 1ª red wifi*/
      red_wifi = 0;
      cambiar_red_wifi();
    }

    if (chat_id == idTelegram && text == "/" + red2) {  /*En el caso de que escriba la 2ª red wifi*/
      red_wifi = 1;
      cambiar_red_wifi();
    }

    if (chat_id == idTelegram && text == "/" + red3) {  /*En el caso de que escriba la 3ª red wifi*/
      red_wifi = 2;
      cambiar_red_wifi();
    }

    if (chat_id == idTelegram && text == "/" + red4) {  /*En el caso de que escriba la 4ª red wifi*/
      red_wifi = 3;
      cambiar_red_wifi();
    }

    if (chat_id == idTelegram && text == "/" + red5) {  /*En el caso de que escriba la 5ª red wifi*/
      red_wifi = 4;
      cambiar_red_wifi();
    }
 
    if (chat_id == idTelegram && text == "/RELE1_ON") { /*En el caso de que escriba la RELE1_ON*/
      address = 0;
      value = 1;
      EEPROM.write(address, value);
      EEPROM.commit();
      digitalWrite(RELE_1_PIN,HIGH);
      bot.sendMessage(chat_id, "Se acaba de activar el RELE 1"); 
    }   

    if (chat_id == idTelegram && text == "/RELE1_OFF") {  /*En el caso de que escriba la RELE1_OFF*/
      address = 0;
      value = 0;
      EEPROM.write(address, value);
      EEPROM.commit();
      digitalWrite(RELE_1_PIN,LOW);
      bot.sendMessage(chat_id, "Se acaba de desactivar el RELE 1"); 
    }  

    if (chat_id == idTelegram && text == "/RELE2_ON") {  /*En el caso de que escriba la RELE2_ON*/
      address = 1;
      value = 1;
      EEPROM.write(address, value);
      EEPROM.commit();
      digitalWrite(RELE_2_PIN,HIGH);
      bot.sendMessage(chat_id, "Se acaba de activar el RELE 2"); 
    }   

    if (chat_id == idTelegram && text == "/RELE2_OFF") { /*En el caso de que escriba la RELE2_OFF*/
      address = 1;
      value = 0;
      EEPROM.write(address, value);
      EEPROM.commit();
      digitalWrite(RELE_2_PIN,LOW);
      bot.sendMessage(chat_id, "Se acaba de desactivar el RELE 2"); 
    } 

     if (chat_id == idTelegram && text == "/TECLADO") {  /*Esto permite modificar los botones del teclado especial*/
       String keyboardJson = "[[\"/RELE1_ON\", \"/RELE1_OFF\"],[\"/RELE2_ON\", \"/RELE2_OFF\"],[\"/ESTADO\"],[\"/WIFI\", \"/DATOS\"]]";
       bot.sendMessageWithReplyKeyboard(chat_id, "Elegir una de las siguientes opciones", "", keyboardJson, true);
     }

    if (chat_id == idTelegram && text == "/ESTADO") {   /*En el caso de que escriba ESTADO*/
      bool rele1 = LOW;
      bool rele2 = LOW;
      if(digitalRead(IN_1_PIN) == LOW)  { IN1 = HIGH; }
      if(digitalRead(IN_1_PIN) == HIGH) { IN1 = LOW; }
      if(digitalRead(IN_2_PIN) == LOW)  { IN2 = HIGH; }
      if(digitalRead(IN_2_PIN) == HIGH) { IN2 = LOW; }
      if(digitalRead(IN_3_PIN) == LOW)  { IN3 = HIGH; }
      if(digitalRead(IN_3_PIN) == HIGH) { IN3 = LOW; }
      address = 0;
      value = EEPROM.read(address);
      if (value == 1) {
        rele1 = HIGH; 
      } else {
        rele1 = LOW; 
      }
      address = 1;
      value = EEPROM.read(address);
      if (value == 1) {
        rele2 = HIGH; 
      } else {
        rele2 = LOW;    
      }
      String estado = "Resumen de los estados:\n\n";
      estado += "Red WIFI: " + WiFi.SSID();
      estado += " Señal: " + String(WiFi.RSSI()+130) + "%\n\n";
      estado += "Temperatura en el interior: " + String(temperatura_filtrada1) + "ºC\n";
      estado += "Humedad en el interior: " + String(humedad_filtrada1) + "%\n\n";
      estado += "Temperatura en el exterior: " + String(temperatura_filtrada2) + "ºC\n";
      estado += "Humedad en el exterior: " + String(humedad_filtrada2) + "%\n\n";
      estado += "Entrada digital 1: ";
      estado += IN1?"Activada":"Desactivada";
      estado += "\n";
      estado += "Entrada digital 2: ";
      estado += IN2?"Activada":"Desactivada";
      estado += "\n";
      estado += "Entrada digital 3: ";
      estado += IN3?"Activada":"Desactivada";
      estado += "\n";
      estado += "Relé 1: ";
      estado += rele1?"Activado":"Desactivado";
      estado += "\n";
      estado += "Relé 2: ";
      estado += rele2?"Activado":"Desactivado";
      estado += "\n";
      estado += "Volver al menú principal: /MENU \n";
      bot.sendMessage(chat_id, estado);            
    }
  }
}

void periodicas() {
  unsigned long T = millis();
  if (LT[0]+MS[0] <= T)   {if (LT[0] < MAXUL - MS[0])   {LT[0] = T;} else {LT[0] = MAXUL - T;} OB35();} /*100ms*/
  if (LT[1]+MS[1] <= T)   {if (LT[1] < MAXUL - MS[1])   {LT[1] = T;} else {LT[1] = MAXUL - T;} OB36();} /*200ms*/
  if (LT[2]+MS[2] <= T)   {if (LT[2] < MAXUL - MS[2])   {LT[2] = T;} else {LT[2] = MAXUL - T;} OB37();} /*500ms*/
  if (LT[3]+MS[3] <= T)   {if (LT[3] < MAXUL - MS[3])   {LT[3] = T;} else {LT[3] = MAXUL - T;} OB38();} /*1s   */
  if (LT[4]+MS[4] <= T)   {if (LT[4] < MAXUL - MS[4])   {LT[4] = T;} else {LT[4] = MAXUL - T;} OB39();} /*2s   */
  if (LT[5]+MS[5] <= T)   {if (LT[5] < MAXUL - MS[5])   {LT[5] = T;} else {LT[5] = MAXUL - T;} OB40();} /*5s   */
  if (LT[6]+MS[6] <= T)   {if (LT[6] < MAXUL - MS[6])   {LT[6] = T;} else {LT[6] = MAXUL - T;} OB41();} /*10s  */
  if (LT[7]+MS[7] <= T)   {if (LT[7] < MAXUL - MS[7])   {LT[7] = T;} else {LT[7] = MAXUL - T;} OB42();} /*30s  */
  if (LT[8]+MS[8] <= T)   {if (LT[8] < MAXUL - MS[8])   {LT[8] = T;} else {LT[8] = MAXUL - T;} OB43();} /*1m   */
  if (LT[9]+MS[9] <= T)   {if (LT[9] < MAXUL - MS[9])   {LT[9] = T;} else {LT[9] = MAXUL - T;} OB44();} /*10m   */
}

void estabilidad() {
  if (millis() >= 120000 && inestable == HIGH) { /* Esperamos 2 min después del arranque a que se cargue el buffer del filtro y tengamos buenos valores medios */
    inestable = LOW;
  }
}

void loop() {
  /* Si no hay Wifi disponible se conecta a otra red */
  while (WiFi.status() != WL_CONNECTED) {
    cambiar_red_wifi(); /* Salimos de la función cuando tenemos Wifi */
    if (WiFi.status() == WL_CONNECTED){
      Serial.println("");
      Serial.println("WiFi conectado OK");
      Serial.print("RED WIFI = ");
      Serial.print(red_wifi);
      Serial.println ( "" );
      Serial.print ( "Conectado a: " );
      Serial.println (WiFi.SSID());
      Serial.println(WiFi.localIP());
    }
  }
  
  
  if (WiFi.status() == WL_CONNECTED  && red_wifi!= redwifialmacenada){
    if(Ping.ping(remote_host)){
      bot.sendMessage(idTelegram,"Una nueva conexión WIFI se ha establecido con: " + WiFi.SSID() + " con una señal de: " + String(WiFi.RSSI()+130) + "%\n\nPulse /MENU\n", "");
      ThingSpeak.begin(client); /* Arrancamos el cliente para Thinkspeak */
      redwifialmacenada = red_wifi;
    }
  }

  /* En el caso de tener Wifi, conectamos el piloto verde en señal de que funciona correctamente */
  if (WiFi.status() == WL_CONNECTED) {digitalWrite(LED_VERDE,HIGH);} else {digitalWrite(LED_VERDE,LOW);}

  /* Leemos el estado de las entradas (negadas) y refrescamos las variables (sin negar)*/
  if(digitalRead(IN_1_PIN) == LOW)  { IN1 = HIGH; }
  if(digitalRead(IN_1_PIN) == HIGH) { IN1 = LOW; }
  if(digitalRead(IN_2_PIN) == LOW)  { IN2 = HIGH; }
  if(digitalRead(IN_2_PIN) == HIGH) { IN2 = LOW; }
  if(digitalRead(IN_3_PIN) == LOW)  { IN3 = HIGH; }
  if(digitalRead(IN_3_PIN) == HIGH) { IN3 = LOW; }

  /* Envio de eventos */
  if (IN1_last!=IN1) {
    if(IN1) { bot.sendMessage(idTelegram, "La primera entrada digital ha cambiado de estado. Su estado actual es ACTIVA", ""); }
    else    { bot.sendMessage(idTelegram, "La primera entrada digital ha cambiado de estado. Su estado actual es DESACTIVADA", ""); }
    IN1_last = IN1;
    delay(1000);
  }

  if (IN2_last!=IN2) {
    if(IN2) { bot.sendMessage(idTelegram, "La segunda entrada digital ha cambiado de estado. Su estado actual es ACTIVA", ""); }
    else    { bot.sendMessage(idTelegram, "La segunda entrada digital ha cambiado de estado. Su estado actual es DESACTIVADA", ""); }
    IN2_last = IN2;
    delay(1000);
  }

  if (IN3_last!=IN3) {
    if(IN3) { bot.sendMessage(idTelegram, "La tercera entrada digital ha cambiado de estado. Su estado actual es ACTIVA", ""); }
    else    { bot.sendMessage(idTelegram, "La tercera entrada digital ha cambiado de estado. Su estado actual es DESACTIVADA", ""); }
    IN3_last = IN3;
    delay(1000);
  }


  /* Recepción de eventos */
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      ultimo_tiempo_sin_mensajes = millis();
      Bot_mtbs = 1000;
    }
    Bot_lasttime = millis();
  }
  
  if (millis() > (ultimo_tiempo_sin_mensajes + (retardo_volver_a_reposo*1000)) && Bot_mtbs!=(periodo_consulta*1000)) {
    Bot_mtbs = periodo_consulta*1000;   
  } 
  periodicas(); 
  estabilidad();
}

