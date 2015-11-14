#include <AltSoftSerial.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <SFE_MG2639_CellShield.h>

//Macro usada para hacer el casting necesario para que las funciones que emplean el
//tipo __FlashStringHelper acepten correctamente las cadenas almacenadas en la
//memoria flash declaradas con PROGMEM
#define FSH(cadena_pgm) reinterpret_cast<const __FlashStringHelper *>(cadena_pgm)

//Tiempo de envio de datos al servidor en milisegundos
const unsigned long tiempoActualizacion = 10 * 1000;

unsigned long tUltimaAct = -tiempoActualizacion;

static PROGMEM const char servidor[] = "alsw.net";
static PROGMEM const char pagina[] = "/trackid/posicion2.php";

char bufferTexto[256];

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
}

void loop() {

  while (gprs.available())
  {
    Serial.write(gprs.read());
  }

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
  long lat_i, lat_f, lon_i, lon_f;
  int posBuf;
  
  gps.get_position(&lat, &lon, NULL);
  lat_i = lat / 1000000;
  lat_f = lat % 1000000;
  if (lat_f < 0) lat_f = -lat_f;
  lon_i = lon / 1000000;
  lon_f = lon % 1000000;
  if (lon_f < 0) lon_f= -lon_f;

  sprintf_P(bufferTexto, (PGM_P) F("Posicion Actual: LAT = %li.%06li, LON = %li.%06li"), lat_i, lat_f, lon_i, lon_f);
  Serial.println(bufferTexto);

  Serial.println(F("Enviando posicion..."));

  //Se intenta conectar con el servidor
  strcpy_P(bufferTexto, servidor);
  while (gprs.connect(bufferTexto, 80) <= 0) {
    Serial.println(F("\nNo se puede encontrar servidor... "
                     "reintentando"));
    delay(2000);
  }
  Serial.println(F("Conectado!"));

  strcpy_P(bufferTexto, (PGM_P) F("GET "));
  strcat_P(bufferTexto, pagina);
  posBuf = strlen(bufferTexto);
  sprintf_P(bufferTexto + posBuf, (PGM_P) F("?lat=%li.%06li&lon=%li.%06li HTTP/1.1\nHost: "), lat_i, lat_f,lon_i, lon_f);
  strcat_P(bufferTexto, servidor);
  strcat_P(bufferTexto, (PGM_P) F("\n\n"));

  Serial.print(bufferTexto);
  gprs.print("GET / HTTP/1.0\n\n");
}

