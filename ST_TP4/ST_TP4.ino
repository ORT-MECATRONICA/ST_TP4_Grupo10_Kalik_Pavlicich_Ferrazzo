//Grupo10_Kalik_Pavlicich_Ferrazzo

//Librerias
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <Preferences.h>

//Eeprom
Preferences preferences;

//Ajustes de pantalla
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Ajustes de DHT
#define DHTPIN 23      // Pin
#define DHTTYPE DHT11  // tipo de dht
DHT dht(DHTPIN, DHTTYPE);

//Maquina de estados
#define PANTALLA1 1
#define ESTADO_CONFIRMACION1 2
#define ESTADO_CONFIRMACION2 3
#define PANTALLA2 4
#define SUBIR 5
#define BAJAR 6

#define PULSADO 0
#define NO_PULSADO 1

#define BOTON1 34
#define BOTON2 35
#define LED1 25
#define LED2 26

//Variables globales
int lectura1;
int lectura2;
int estado = 1;
int contador1;

unsigned long TiempoAhora;
unsigned long TiempoUltimoCambio1 = 0;
const long Intervalo = 2000;
const long Intervalo5Seg = 5000;
unsigned long TiempoUltimoCambio2= 0;

unsigned int VALOR_UMBRAL;
float t;

void MAQUINA_DE_ESTADOS();

void setup() {
  Serial.begin(115200);

  //DHT inicio
  dht.begin();

  //Eeprom
  preferences.begin("Eeprom", false); //False significa modo leer y escribir, en True seria solo leer //El "Eeprom" es el nombre del esapcio que se crea
  VALOR_UMBRAL = preferences.getUInt("VALOR_UMBRAL", 28); //Obtiene el valor //get Uint porque es Usigned int, y el 28 es un valor predeterminado si no hay un "Valor UMBRAL" devuelve 28

  //Pinmodes
  pinMode(BOTON1, INPUT);
  pinMode(BOTON2, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  //Inicio de pantalla
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.display();
}

void loop() {
  //Lectura de botones
  lectura1 = digitalRead(BOTON1);
  lectura2 = digitalRead(BOTON2);

  //Serial.println(estado);
  MAQUINA_DE_ESTADOS();
}

void MAQUINA_DE_ESTADOS() {
  t = dht.readTemperature();
  TiempoAhora = millis();
  switch (estado) {
    case PANTALLA1:
      if (TiempoAhora - TiempoUltimoCambio1 >= Intervalo)  ///delay sin bloqueo
      {
        TiempoUltimoCambio1 = TiempoAhora;  // importante actualizar el tiempo

        display.clearDisplay();
        display.setCursor(0, 20);
        display.print("Temp: ");
        display.print(t);
        display.print("°C ");

        display.setCursor(0, 40);
        display.print("Valor Umbral: ");
        display.print(VALOR_UMBRAL);
        display.display();

      }
      if (t >= VALOR_UMBRAL) {
        digitalWrite(LED1, HIGH);
      } else {
        digitalWrite(LED1, LOW);
      }

      //Para cambiar de estado
      if (lectura1 == PULSADO) {
        TiempoUltimoCambio2 = TiempoAhora;  //Se actualiza el tiempo
        estado = ESTADO_CONFIRMACION1;
      }
      break;

    case ESTADO_CONFIRMACION1:
      if (lectura1 == NO_PULSADO && (TiempoAhora - TiempoUltimoCambio2 >= Intervalo5Seg)) { //Si pasan 5 segundos
        estado = PANTALLA2;
      }
      else if (lectura1 == NO_PULSADO){ //Si no pasa 5 segundos vuelve
        estado = PANTALLA1;
      }
      break;

    case PANTALLA2:
      digitalWrite(LED1, LOW);

      display.clearDisplay();
      display.setCursor(0, 20);
      display.print("Valor Umbral: ");
      display.print(VALOR_UMBRAL);
      display.display();

      if (lectura1 == PULSADO) {
        contador1 = 1;
        estado = SUBIR;
      }
      if (lectura2 == PULSADO) {
        TiempoUltimoCambio2 = TiempoAhora;
        contador1 = 1;
        estado = BAJAR;
      }
      break;

    case ESTADO_CONFIRMACION2:
      if (lectura2 == NO_PULSADO) {
        preferences.putUInt("VALOR_UMBRAL", VALOR_UMBRAL);  //Guarda el valor Umbral en una Key llamada "VALOR UMBRAL"
        estado = PANTALLA1;
      }
      break;

    case SUBIR:
      
      if (lectura1 == NO_PULSADO) {
        digitalWrite(LED1, HIGH);
        if (contador1 == 1) {
          VALOR_UMBRAL = VALOR_UMBRAL + 1;
          contador1 = 0;
        }
        estado = PANTALLA2;
      }
      break;

    case BAJAR:
    if (TiempoAhora - TiempoUltimoCambio2 >= Intervalo5Seg) { //Si pasan los 5 segundos va a confirmacion2
        estado = ESTADO_CONFIRMACION2;
      }
      if (lectura2 == NO_PULSADO) {
        digitalWrite(LED1, HIGH);
      if (contador1 == 1) {
        VALOR_UMBRAL = VALOR_UMBRAL - 1;
        contador1 = 0;
      }
        estado = PANTALLA2;
      }
      break;
  }
}