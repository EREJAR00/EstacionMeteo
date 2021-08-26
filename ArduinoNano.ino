/*
 *  Arduino SD Card Tutorial Example
 *  
 *  by Dejan Nedelkovski, www.HowToMechatronics.com
 */

#include <SD.h>                   //https://howtomechatronics.com/tutorials/arduino/arduino-sd-card-data-logging-excel-tutorial/
#include <SPI.h>

File Estacion;
int pinCS = 10; // Pin 10 on Arduino Uno
String recibido = "";
int errorPin = 7;
int correctoPin = 5;
int esperaPin = 3;

void setup() {
  digitalWrite(errorPin, LOW);
  digitalWrite(correctoPin, LOW);
  digitalWrite(esperaPin, LOW);
   
  Serial.begin(9600);
  pinMode(errorPin, OUTPUT);
  pinMode(correctoPin, OUTPUT);
  pinMode(esperaPin, OUTPUT);
  pinMode(pinCS, OUTPUT);
  while (!Serial)
    delay(1); 
  Serial.println("Arranco");
 // pinMode(pinCS, OUTPUT);
   

  // SD Card Initialization
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
    digitalWrite(correctoPin, HIGH);
    delay(500);
    digitalWrite(correctoPin, LOW);
    delay(500);
    digitalWrite(correctoPin, HIGH);
    delay(500);
    digitalWrite(correctoPin, LOW);
    delay(500);
  } else
  {
    Serial.println("SD card initialization failed");
    digitalWrite(errorPin, HIGH);
    return;
  }
  
  // Create/Open file 
  Estacion = SD.open("/Estacion.txt", FILE_WRITE);
  Estacion.close();
  if (SD.exists("/Estacion.txt")) 
    Serial.println("Existe el archivo de texto");
  else{
    Serial.println("Error con el archivo de texto");
    digitalWrite(errorPin, HIGH);
  }
}
void loop() {
  
  while(Serial.available() > 0) { // Primero limpiamos el buffer de entrada
    char t = Serial.read();
  }
    delay(1000);
    Serial.println("Preparado para recibir datos");
    digitalWrite(esperaPin, HIGH);
    while(Serial.available() <= 0) // Y esperamos a por los datos para enviar
      delay(500);

  if (Serial.available () >0) //Esperamos un segundo para evitar cortar la comunicación
    delay(1000);
    
    digitalWrite(esperaPin, LOW);
    
  recibido = "";
  while (Serial.available()>0){
    char letra = Serial.read();
     if(letra == '.'){
      letra = ',';
     }
      recibido += letra;
   }

//  Serial.println("RECIBIDO TODO");

  Estacion = SD.open("/Estacion.txt", FILE_WRITE);
    if (Estacion) {
    Estacion.println(recibido);
    Estacion.close(); // close the file
    digitalWrite(correctoPin, HIGH);
    delay(500);
    digitalWrite(correctoPin, LOW);
    delay(500);
    digitalWrite(correctoPin, HIGH);
    delay(500);
    digitalWrite(correctoPin, LOW);
    delay(500);
    }
    // if the file didn't open, print an error:
    else {
      Serial.println("error opening .txt");
      digitalWrite(errorPin, HIGH);
    }
}
