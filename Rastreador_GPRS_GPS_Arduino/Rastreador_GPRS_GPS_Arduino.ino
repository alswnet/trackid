#include <AltSoftSerial.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <SFE_MG2639_CellShield.h>

//Tiempo de envio de datos al servidor en milisegundos
const unsigned long tiempoActualizacion = 10 * 1000;

unsigned long tUltimaAct = -tiempoActualizacion;

AltSoftSerial serialGPS;
TinyGPS gps;

bool gpsActualizado = false;

void setup() {
  //Se inicializa el puerto serie para depuracion
  Serial.begin(115200);

  //Se inicializa el puerto serie AltSoftSerial del GPS
  serialGPS.begin(9600);

  //Se intenta inicializar el modulo celular
  Serial.print(F("Iniciando modulo celular..."));
  while (cell.begin() <= 0) {
    //Si no hubo exito, se notifica el error y se otorga
    //una espera antes de reintentar
    Serial.println(F("\nError al inicializar el modulo "
                     "celular... reiniciando"));
    delay(2000);
  }
  //Si todo salio bien, se imprime OK y continua
  Serial.println(F("OK"));

  //Se intenta inicializar el modo GPRS
  Serial.print(F("Iniciando modo GPRS..."));
  while (gprs.open() <= 0) {
    //Si fallo en levantar la conexion, se muestra el mensaje
    //y espera antes de reinitentar
    Serial.println(F("\nError al inicializar el modo GPRS... "
                     "reintentando"));
    delay(2000);
  }
  Serial.println(F("OK"));

  //Se 
}

void loop() {
  //Se actualiza la informacion del modulo GPS
  actualizarGPS();

  //Se toma el tiempo actual del reloj interno
  unsigned long tActual = millis();

  //Si ya elapso el periodo de actualizacion y adema el GPS
  //tiene datos actualizados, entonces se envia la posicion
  if (tActual - tUltimaAct >= tiempoActualizacion &&
      gpsActualizado)
  {
    enviarPosicion();
    tUltimaAct = tActual; //Se registra tiempo de envio
  }
}

void actualizarGPS() {
  char c;

  while (serialGPS.available()) {
    //Toma caracteres del GPS mientras haya disponibles
    c = serialGPS.read();

    //Se procesan los caracteres entrantes
    if (gps.encode(c))
      //Si se encontro una nueva sentencia, significa que
      //se actualuzo el estado del GPS, por lo que se
      //activa la bandera
      gpsActualizado = true;
  }
}

void enviarPosicion() {
  long lat, lon;
  char cadenaNum[16];
  gps.get_position(&lat, &lon, NULL);
  Serial.print(F("Posicion Actual: LAT = "));
  sprintf(cadenaNum, " % li. % 06li", lat / 1000000, abs(lat % 1000000));
  Serial.print(cadenaNum);
  Serial.print(F(", LON = "));
  sprintf(cadenaNum, " % li. % 06li", lon / 1000000, abs(lon % 1000000));
  Serial.println(cadenaNum);
  Serial.println(F("Enviando posicion..."));
}

