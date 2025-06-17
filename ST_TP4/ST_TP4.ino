//Grupo10_Kalik_Pavlicich_Ferrazzo

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <Preferences.h>

Preferences preferences;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 23      // Pin
#define DHTTYPE DHT11  // tipo de dht

DHT dht(DHTPIN, DHTTYPE);

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

int lectura1;
int lectura2;
int estado = 1;
int contador1;
unsigned long TiempoAhora;
float t;
unsigned long TiempoUltimoCambio1 = 0;
const long Intervalo = 2000;
const long Intervalo5Seg = 5000;
unsigned long TiempoUltimoCambio2= 0;
unsigned int VALOR_UMBRAL;

void MAQUINA_DE_ESTADOS();

void setup() {
  Serial.begin(115200);

  dht.begin();

  preferences.begin("Eeprom", false);

  VALOR_UMBRAL = preferences.getUInt("VALOR_UMBRAL", 28);

  pinMode(BOTON1, INPUT);
  pinMode(BOTON2, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

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
        TiempoUltimoCambio1 = TiempoAhora;  /// importante actualizar el tiempo

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
      if (lectura1 == PULSADO) {
        TiempoUltimoCambio2 = TiempoAhora;
        estado = ESTADO_CONFIRMACION1;
      }
      break;

    case ESTADO_CONFIRMACION1:
      if (lectura1 == NO_PULSADO && (TiempoAhora - TiempoUltimoCambio2 >= Intervalo5Seg)) {
        estado = PANTALLA2;
      }
      else if (lectura1 == NO_PULSADO){
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


      if (lectura1 == PULSADO && lectura2 == PULSADO) {
        estado = ESTADO_CONFIRMACION2;
      }
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
        preferences.putUInt("VALOR_UMBRAL", VALOR_UMBRAL);
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
    if (TiempoAhora - TiempoUltimoCambio2 >= Intervalo5Seg) {
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