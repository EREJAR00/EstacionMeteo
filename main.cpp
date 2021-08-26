#include <Arduino.h>              //Biblioteca que permite que se programe para arduino
#include <Wire.h>                 //Biblioteca para el protocolo de comunicación I2C
#include <LiquidCrystal_I2C.h>    //Biblioteca para la pantalla LCD en comunicación I2C
#include <TMP117.h>               //Biblioteca para el sensor de temperatura BlueDot TMP117
#include <Adafruit_SHT31.h>				//Biblioteca para el sensor de humedad Adafruit SHT31
#include <Adafruit_LPS35HW.h>     //Biblioteca para el sensor de presión Adafruit LPS33HW
#include <Adafruit_SGP30.h>				//Biblioteca para el sensor de gases BlueDot SPG_30
#include <SoftwareSerial.h>       //Biblioteca para la intercomunicación ente ATMega y ESP
#include <RTClib.h>               //Biblioteca para el reloj de tiempo real DS3231

//VARIABLES CALCULO ------------------------------------------------------------
float temperatura = 0;	float temUni 	= 0;	float temMinuto		= 0;  float temPromedio  = 0; //Suma los valores minutales pra hacer el promedio de 10 minitos
float humedad     = 0;	float humUni 	= 0;	float humMinuto		= 0;  float humPromedio  = 0;
float presion 		= 0;	float preUni 	= 0;	float preMinuto		= 0;  float prePromedio  = 0;
float viento   		= 0;	float vieUni 	= 0;	float vieMinuto		= 0;  float viePromedio  = 0;
float rafaga 			= 0;	float rafUni	= 0;	                        float rafPromedio  = 0;    float rafOperador = 0; //Sirve para determinar la media sin borrar valores necesarios
int UV 						= 0;											int UVMinuto			= 0;  float UVPromedio   = 0;
long  eCO2 		    = 0;  long  eCO2Uni = 0;	long  eCO2Minuto  = 0;  long eCO2Promedio  = 0;
long  TVOC 		    = 0;  long  TVOCUni	= 0;	long  TVOCMinuto	= 0;  long TVOCPromedio  = 0;


//VARIABLES TEMPORIZADORES -----------------------------------------------------
const int esperaMedio 			= 500;		//Creamos un valor de espera de medio segundo para las medidas de los sensores
const long int esperaMinuto = 60000;	//Tiempo de un minuto
const int esperaTres	 			= 3000;		//Tiempo de 3 milisegundos
int contadorMin 		= 0;				      //Variables para poder hacer los promedios. Valores minutales
int contadorRafaga 	= 0;              //Valores Rafaga
int contador10min   = 0;              //Valores 10 minutos
unsigned long milisMedio 	= 0;				//Usamos una variable para guardar los milisegundos previos y otra para los de ahora y así obtener los medios segundos sin preocuparnos por resetear milis()
unsigned long milisTres 	= 0;
unsigned long milisMinuto = 0;

//VARIABLES UNE 500540 ---------------------------------------------------------
//Las variables deben incluir un código de control. Estos se presentarán después de todas las medidas de las variables. Todas se inicializan a 7 pues son valores sin definir y los usaremos de control del código.
int temCod 	= 0;    int temCodMin   = 0;    int temCodPro   = 0;    int temCodigo   = 0;
int humCod 	= 0;    int humCodMin   = 0;    int humCodPro   = 0;    int humCodigo   = 0;
int preCod 	= 0;    int preCodMin   = 0;    int preCodPro   = 0;    int preCodigo   = 0;
int vieCod 	= 0;		int vieCodMin   = 0;    int vieCodPro   = 0;    int vieCodigo   = 0;   //la medida de la ráfaga comparte este código
                    int UVCodMin    = 0;    int UVCodPro    = 0;    int UVCodigo    = 0;
int TVOCCod = 0;    int TVOCCodMin  = 0;    int TVOCCodPro  = 0;    int TVOCCodigo  = 0;
int eCO2Cod = 0;    int eCO2CodMin  = 0;    int eCO2CodPro  = 0;    int eCO2Codigo  = 0;
float Control500540 = 0;
//El segundo valor del código hace referencia a si se ha realizado una inspeción visual. Ninguno de los sensores precisan inspección visual, por tanto siempre será 0

// UNE 500540 NIVEL 1 --------------------------------------------------------
int temMin	=	-23;	int temMax	=	44;
int	humMin	=	0;		int humMax	= 100;
int preMin	=	260;	int preMax	= 1260;
int vieMin	=	-1;		int vieMax	=	15;
int UVMin		=	0;		int UVMax		= 15;
int TVOCMin	=	0;		long int TVOCMax	= 60000;
int eCO2Min	=	400;	long int eCO2Max	= 60000;

//ESP8266 COMUNICACION ---------------------------------------------------------
SoftwareSerial mySerial(2, 3); // RX | TX

int contadorSeparador = 0;  //Contador para separar los envíos de la EMA a la base de datos

//PANTALLA ---------------------------------------------------------------------
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

uint8_t ADDR_GND =  0x48;   // 1001000
uint8_t ADDR_VCC =  0x49;   // 1001001
uint8_t ADDR_SDA =  0x4A;   // 1001010
uint8_t ADDR_SCL =  0x4B;   // 1001011
uint8_t ADDR =  ADDR_VCC;

//TEMPERATURA ------------------------------------------------------------------
bool alert_flag = false;
TMP117 tmp(ADDR);

//HUEMDAD ----------------------------------------------------------------------
bool enableHeater = false;
//uint8_t loopCnt = 0;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

//PRESION ----------------------------------------------------------------------
Adafruit_LPS35HW lps35hw = Adafruit_LPS35HW();

//GAS --------------------------------------------------------------------------
Adafruit_SGP30 sgp;

//Compensado recomendado por el fabricante baasado en la temperatura y la humedad
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

//RELOJ ------------------------------------------------------------------------
RTC_DS3231 rtc;
String fecha = "";
String hora  = "";
//SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP
//SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP
//SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP - SETUP

void setup()
{
  //ARDUINO --------------------------------------------------------------------
	pinMode(A0, INPUT);    //Pin medidor UV
  pinMode(A2, INPUT);    //Pin medidor Velocidad Viento
	Wire.begin();
	Serial.begin(115200);
  Serial.println("Arranco");

	pinMode(LED_BUILTIN, OUTPUT);

	UV = analogRead(A0)/100;		//Se da una medida instantanea durante el primer ciclo de medida (10 minutos) de la variable U

  //PANTALLA -------------------------------------------------------------------
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("    PROYECTO    ");
  lcd.setCursor(0,1);
  lcd.print("  Meteorologia  ");

  //ESP8266 COMUNICACION -------------------------------------------------------
  mySerial.begin(9600);  //Se usa una velocidad menor que con el Serial ya que tras testeo muestra un mejor funcionamiento.
  while (!Serial) {
  delay (1); // wait for serial port to connect. Needed for native USB port only
}
Serial.println("Serial Software activado");

  //TEMPERATURA ----------------------------------------------------------------
  tmp.init ( NULL );

	//HUMEDAD --------------------------------------------------------------------
Serial.println("SHT31 test");
 if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
	 Serial.println("Couldn't find SHT31");
 }

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
   Serial.println("ENABLED");
  else
   Serial.println("DISABLED");

  //PRESION --------------------------------------------------------------------
  Serial.println("Adafruit LPS35HW Relative Pressure Test");

  if (!lps35hw.begin_I2C()) {
    Serial.println("Couldn't find LPS35HW chip");
    while (1);
  }
  Serial.println("Found LPS35HW chip");
  Serial.println();
  //GAS ------------------------------------------------------------------------
  Serial.println("SGP30 test");

  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.print("Found SGP30 serial #");
	Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!

  //RELOJ ----------------------------------------------------------------------
  if (! rtc.begin())
     Serial.println("Couldn't find RTC");

	//	 rtc.adjust(DateTime(2021, 7, 9, 18, 4, 0));
  if (rtc.lostPower()) {
   Serial.println("RTC lost power, let's set the time!");
   rtc.adjust(DateTime(2000, 1, 1, 0, 0, 0));
   // When time needs to be set on a new device, or after a power loss, the
   // following line sets the RTC to the date & time this sketch was compiled
   // This line sets the RTC with an explicit date & time, for example to set
   // January 21, 2014 at 3am you would call:
   // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
   }

}


int counter = 0;

//LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - L
//LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - L
//LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - LOOP - L


void loop(){


  //MEDIDAS ÚNICAS -----------------------------------------------------------
	if ((unsigned long)(millis() - milisMedio) >= esperaMedio){
	milisMedio = millis();

          //CALCULO PRESION ESTANDAR ---------------------------------------------------
          float PresionInicial = lps35hw.readPressure();
          float PresionTemperatura = sht31.readTemperature();
          float es = 6.11*pow(10,((7.5*PresionTemperatura)/(237.3+PresionTemperatura))); //Calculo presión de valpor saturada
          float Hp = 725; 	//Altura geopotencial en gpm para 725m y 42.15º
          float a = 0.0065;			//Kg/gpm
          float ch = 0.12;			//K/hPa
          float PresionKelvin = 273.15 + PresionTemperatura;
          float gn = 9.80665;		//m/s^2
          float Rn = 287.05; 		//J/KgK
          float NumeroE = 2.71828;
          float PresionReducida = PresionInicial*pow((NumeroE),(((gn*Hp)/Rn)/(PresionKelvin + (a*Hp)/2 + es*ch)));

          //Calculo presión absoluta para compensado del SGP30 (nombres en inglés para evitar problemas con variables en uso)
          float temperature = sht31.readTemperature(); // [°C]
          float humidity 		= sht31.readHumidity(); // [%RH]
          sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

  	Control500540 = sht31.readTemperature();	//Con esta variable comparamos el valor con los límites físicos del sensor, y si la medida es un número
  	if (Control500540 >= temMin && Control500540 <= temMax){	//Temperatura
  		temUni += Control500540;	temCod += 1;	}
  	else
  		temCod += 0;

  	Control500540 = sht31.readHumidity();
  	if (Control500540 >= humMin && Control500540 <= humMax){	//Humedad
  		humUni += Control500540;	humCod += 1;	}
  	else
  		humCod += 0;

  	Control500540 = PresionReducida;
  	if (Control500540 >= preMin && Control500540 <= preMax){	//PresionTemperatura
  		preUni += Control500540;	preCod += 1;	}
  	else
  		preCod += 0;

  	float calculador = analogRead(A2);
    Control500540 = calculador*15/1024;
    Serial.println(Control500540);
  	if (Control500540 >= vieMin && Control500540 <= vieMax){	//Viento
  		vieUni += Control500540;	rafUni += Control500540;	vieCod += 1; contadorRafaga += 1;	}
  	else
  		vieCod += 0;

    if (! sgp.IAQmeasure()) {                     //Esta comprobación está obligada por el sensor y si no se realiza no mide
          Serial.println("Measurement failed");
          return;
        }

  	Control500540 = sgp.eCO2;
  	if (Control500540 >= eCO2Min && Control500540 <= eCO2Max){	//CO2
  		eCO2Uni += Control500540;	eCO2Cod += 1;	}
  	else
  		eCO2Cod += 0;

  	Control500540 = sgp.TVOC;
  	if (Control500540 >= TVOCMin && Control500540 <= TVOCMax){	//TVOC
  		TVOCUni += Control500540;	TVOCCod += 1;	}
  	else
  		TVOCCod += 0;
    Serial.print("TVOC "); Serial.print(TVOCUni); Serial.print(" ppb\t");
    Serial.print("eCO2 "); Serial.print(eCO2Uni); Serial.println(" ppm");
    contadorMin += 1;

  	if (digitalRead(LED_BUILTIN) == LOW)
  		digitalWrite(LED_BUILTIN, HIGH);
  	else
  		digitalWrite(LED_BUILTIN, LOW);

  }
	else
		delay(1);

  //RAFAGAS --------------------------------------------------------------------
	if ((unsigned long)(millis() - milisTres) >= esperaTres){   //Realizamos el promedio de 3 medidas de viento, y guardamos el valor de la ráfaga si es superior al anterior guardado.
		milisTres = millis();

		float mediaRafaga = rafUni/contadorRafaga;
    if (mediaRafaga >= rafOperador)
      rafOperador = mediaRafaga;
    rafUni = 0;
    contadorRafaga = 0;
    mediaRafaga = 0;
  }
	else
		delay(1);


  //FUNCION MINUTAL ----------------------------------------------------------
  if ((unsigned long)(millis() - milisMinuto) >= esperaMinuto){ //Este If llega hasta el final del código
  			milisMinuto = millis();

        //RELOJ --------------------------------------------------------------------
        DateTime now = rtc.now();                     //Producimos una variable que da AÑO_MES_DIA_HORA_MINUTO sin barras bajas (como un número completo)
                                                      ///Por ejemplo: las 15:43 del 7 de abril de 2017 es 201704071543
        fecha = "";                                   //Se añadirá delante un código de uno o dos dígitos que representará el tipo de medida (minutal instantanea o promedio de 10 minutos)
        fecha = fecha + String(now.year(), DEC);      //Se usa para poder ordenar las medidas
        int i = now.month();
        if (i < 10)
          fecha = fecha + "0";
        fecha = fecha + String(i);
        i = now.day();
        if (i < 10)
          fecha = fecha + "0";
        fecha = fecha + String(i);

        hora = "";
        int horabucle = now.hour();
        if (horabucle < 10)
          hora = hora + "0";
        hora = hora + String(horabucle);
        int minuto = now.minute();
        if (minuto < 10)
          hora = hora + "0";
        hora = hora + String(minuto);

        //VALORES INSTANTANEOS -----------------------------------------------
        temMinuto     = temUni/temCod;    temCodMin = temCod/contadorMin;   temUni   = 0; temCod = 0;
        humMinuto     = humUni/humCod;    humCodMin = humCod/contadorMin;   humUni   = 0; humCod = 0;
        preMinuto     = preUni/preCod;    preCodMin = preCod/contadorMin;   preUni   = 0; preCod = 0;
        vieMinuto     = vieUni/vieCod;    vieCodMin = vieCod/contadorMin;   vieUni   = 0; vieCod = 0;
        eCO2Minuto    = eCO2Uni/eCO2Cod;  eCO2CodMin = eCO2Cod/contadorMin; eCO2Uni  = 0; eCO2Cod = 0;
        TVOCMinuto    = TVOCUni/TVOCCod;  TVOCCodMin = TVOCCod/contadorMin; TVOCUni  = 0; TVOCCod = 0;
  		  contadorMin   = 0;

        Control500540 = round(analogRead(A0)/100);
      	if (Control500540 >= UVMin && Control500540 <= UVMax){	 //La medida de UV se realiza aquí porque se recomienda que se haga cada minuto
      		UVMinuto = Control500540;	UVCodMin = 1;	}
      	else
      		UVCodMin = 0;

        temPromedio      += temMinuto;    temCodPro    += temCodMin;
        humPromedio      += humMinuto;    humCodPro    += humCodMin;
        prePromedio      += preMinuto;    preCodPro    += preCodMin;
        viePromedio      += vieMinuto;    vieCodPro    += vieCodMin;
        UVPromedio       += UVMinuto;     UVCodPro    += UVCodMin;
        eCO2Promedio     += eCO2Minuto;   eCO2CodPro   += eCO2CodMin;
        TVOCPromedio     += TVOCMinuto;   TVOCCodPro   += TVOCCodMin;
        contador10min    += 1;

        if (rafOperador >= rafPromedio)
          rafPromedio = rafOperador;

        //Control Pantalla LCD -----------------------------------------------
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(temMinuto);
        lcd.print("C ");
        lcd.print(preMinuto);
        lcd.print("hPa");
        lcd.setCursor(0,1);
        lcd.print(humMinuto);
        lcd.print("% ");
        lcd.print(UVMinuto);
        lcd.print("U ");
        lcd.print(eCO2Minuto);
        lcd.print("ppm");

        temMinuto   = 0;   temCodMin     = 0;
        humMinuto   = 0;   humCodMin     = 0;
        preMinuto   = 0;   preCodMin     = 0;
        vieMinuto   = 0;   vieCodMin     = 0;
        UVMinuto    = 0;   UVCodMin      = 0;
        eCO2Minuto  = 0;   eCO2CodMin    = 0;
        TVOCMinuto  = 0;   TVOCCodMin    = 0;

        //PROMEDIO 10 MINUTOS
        int control = 10;
        if (contador10min >= control){

          temperatura   = temPromedio/contador10min;     temCodigo  = temCodPro/contador10min;     temPromedio   = 0;   temCodPro     = 0;
          humedad       = humPromedio/contador10min;     humCodigo  = humCodPro/contador10min;     humPromedio   = 0;   humCodPro     = 0;
          presion       = prePromedio/contador10min;     preCodigo  = preCodPro/contador10min;     prePromedio   = 0;   preCodPro     = 0;
          viento        = viePromedio/contador10min;     vieCodigo  = vieCodPro/contador10min;     viePromedio   = 0;   vieCodPro     = 0;
          rafaga        = rafPromedio;                                                             rafPromedio   = 0;   rafOperador   = 0;
          UV            = UVPromedio/contador10min;      UVCodigo   = UVCodPro/contador10min;      UVPromedio    = 0;   UVCodPro      = 0;
          eCO2          = eCO2Promedio/contador10min;    eCO2Codigo = eCO2CodPro/contador10min;    eCO2Promedio  = 0;   eCO2CodPro    = 0;
          TVOC          = TVOCPromedio/contador10min;    TVOCCodigo = TVOCCodPro/contador10min;    TVOCPromedio  = 0;   TVOCCodPro    = 0;
          contador10min = 0;

      	//ESP8266 COMUNICACION -----------------------------------------------
        String env_1 = String(temperatura);           //Temperatura
        String cod_1 = String(temCodigo);
        if (env_1 == "0")
          env_1 = "0.00";
        String env_2 = String(humedad);               //Humedad
        String cod_2 = String(humCodigo);
        if (env_2 == "0")
          env_2 = "0.00";
        String env_3 = String(presion);               //Presión
        String cod_3 = String(preCodigo);
        if (env_1 == "0")
          env_1 = "0.00";
        String env_4 = String(viento);                //Viento
        String cod_4 = String(vieCodigo);
        if (env_4 == "0")
          env_4 = "0.00";
        String env_5 = String(rafaga);                //Ráfaga
        if (env_5 == "0")
          env_5 = "0.00";
        String env_6 = String(eCO2);                  //eCO2
        String cod_6 = String(eCO2Codigo);
        if (env_6 == "0")
          env_6 = "0.00";
        String env_7 = String(TVOC);           	      //TVOC
        String cod_7 = String(TVOCCodigo);
        if (env_7 == "0")
          env_7 = "0.00";
        String env_8 = String(UV);										//UV
        String cod_8 = String(UVCodigo);
        if (env_8 == "0")
          env_8 = "0.00";
        String marcador = fecha + hora;
        String coma = ";";
        Serial.println("Preparado para enviar datos");
        String enviar = "";
        enviar = enviar + marcador + coma + env_1 + coma + env_2 + coma + env_3 + coma + env_4 + coma + env_5 + coma + env_6 + coma + env_7 + coma + env_8 + coma + cod_1 + cod_2 + cod_3 + cod_4 + cod_6 + cod_7 + cod_8 + coma;
        Serial.println(enviar);
        mySerial.print(enviar);
        }
  }
}
