/* Code Written by Rishi Tiwari
 *  Website:- https://tricksumo.com
*/



#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Update HOST URL here

#define HOST "https://tcawema.000webhostapp.com/"          // Enter HOST URL without "http:// "  and "/" at the end of URL

#define WIFI_SSID "Redmi Wendy"            // WIFI SSID here                                   
#define WIFI_PASSWORD "wendy123"        // WIFI password here

// Variables globales del sistema

String Marcador = "196311221230";
String Temp = "404";
String Hum = "404";
String Pres = "404";
String Viento = "404";
String Rafaga = "404";
String eCO2 = "404";
String TVOC = "404";
String UV = "404";
String Codigo = "00000000000000";

String recibido = "";

String env_1, env_2, env_3, env_4, env_5, env_6, env_7, env_8, env_9, env_10, postData;


void setup() {
  
       
  Serial.begin(9600);
  while (!Serial)
    delay(1);  
  Serial.println("Comunicación iniciada \n\n");  
  
  reinicio:  
  
  pinMode(LED_BUILTIN, OUTPUT);     // Led interno inicializado para información luminosa
   
  
  int contadorWifi = 0;
  WiFi.mode(WIFI_STA);           
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);         //Proceso de conexión WiFi
  Serial.print("Conectando a: ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    contadorWifi += 1;
    if (contadorWifi >= 120){
      WiFi.disconnect();
      goto reinicio;
    }
    delay(500); 
    }
  
  Serial.println();                            //Información de conexión correcta via Serial para testeo 
  Serial.print("Conectado a: ");
  Serial.println(WIFI_SSID);
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());    
  
  delay(30);

  HTTPClient http;          // Creación de un objeto tipo http
  WiFiClient cliente;
  
  while(Serial.available() > 0) { // Primero limpiamos el buffer de entrada
    char t = Serial.read();
  }
  Serial.println("Preparado para recibir datos");
  while(Serial.available() == 0){ //El equipo espera a recibir datos. No dispone de un modo hibernación a si que se realiza mediante delays
    delay(500);
  }
  if (Serial.available () >0)               //Se espera un segundo para evitar cortar la comunicación
    delay(1000);
  recibido = "";
  int variable = 0;
  while (Serial.available()>0){            //Se lee la información que ha llegado letra a letra y se separa con ;. Solo se esperan 9 datos, por lo que lo demás se ignora 
    char letra = Serial.read();             //Se ha de ignorar debido a que va información de fecha y hora para el datalogger físico. La base de datos online aporta su propia fecha y hora con mejor calibrado
     digitalWrite(LED_BUILTIN, HIGH);
    if(letra == ';'){
      char skip = letra;
      variable = variable + 1;
      //SWITCH CASE ------------------------------------------
      switch (variable){
        case 1:
          Marcador = recibido;
          break;
        case 2:
          Temp = recibido;
          break;
        case 3:
          Hum = recibido;
          break;
        case 4:
          Pres = recibido;
          break;
        case 5:
          Viento = recibido;
          break;
        case 6:
          Rafaga = recibido;
          break;
        case 7:
          eCO2 = recibido;
          break;
        case 8:
          TVOC = recibido;
          break;
        case 9:
          UV = recibido;
          break;
        case 10:
          Codigo = recibido;
          break;
      }
      //SWITCH CASE ------------------------------------------
      recibido = "";
      Serial.print("Saltado :");
      Serial.println(skip);
    }
    else{
      recibido += letra;
    }
  }
  Serial.println("RECIBIDO TODO");
  digitalWrite(LED_BUILTIN, HIGH);
  // Guardamos las variables de forma que sea claramente legible cual es cual y se puedan añadir o cambiar más
  env_1 = Marcador; 
  env_2 = Temp;  
  env_3 = Hum;   
  env_4 = Pres;  
  env_5 = Viento;
  env_6 = Rafaga;  
  env_7 = eCO2;
  env_8 = TVOC;  
  env_9 = UV;
  env_10 = Codigo;
  postData = "env_1=" + env_1 + "&env_2=" + env_2 + "&env_3=" + env_3 + "&env_4=" + env_4 + "&env_5=" + env_5 + "&env_6=" + env_6 + "&env_7=" + env_7 + "&env_8=" + env_8 + "&env_9=" + env_9 + "&env_10=" + env_10;
  
   if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        int contadorWifi = 0;
        WiFi.mode(WIFI_STA);           
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);         //Proceso de conexión WiFi
        Serial.print("Conectando a: ");
        Serial.print(WIFI_SSID);
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
          contadorWifi += 1;
          if (contadorWifi >= 120){
            WiFi.disconnect();
            break;
          }
          delay(500); 
          }
   }
    
  http.begin(cliente, "http://tcawema.000webhostapp.com/dbwrite.php");              // Conexión al host de la base de datos
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");              //Especificación de las cabeceras enviadas
  
    
   
  int httpCode = http.POST(postData);   //  Envío de una petición tipo POST y se guarda la respuesta
  
  // En caso de conexión correcta se ejecuta este código
  if (httpCode == 200) { Serial.println("Valores enviados de forma correcta."); Serial.println(httpCode); 
  String webpage = http.getString();    
  Serial.println(webpage + "\n"); 
  }
  
  // En caso de fallo se ejecuta este otro, y se imprime la información de fallo recibida (el código http)
  
  else { 
    Serial.println(httpCode); 
    Serial.println("Valores no enviados. \n"); 
    }
  WiFi.disconnect();
  
  delay(500); 
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500); 
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500); 
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500); 
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop (){

}
