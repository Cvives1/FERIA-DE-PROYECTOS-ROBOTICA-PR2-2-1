// COMM BAUDS
#define BAUDS 115200

#define LOGGER_ENABLED            // Comentar para deshabilitar el logger por consola serie

#define LOG_LEVEL TRACE           // nivells en c_logger: TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NONE

// DEVICE
//#define DEVICE_ESP_ID             "54CE0361421"   // ESP32 ID
#define DEVICE_GIIROB_PR2_ID      "Miesp32" //"giirobpr2_00"

// WIFI
#define NET_SSID                  "Redmi Note 9 Pro"
#define NET_PASSWD                "987654321"

// MQTT
#define MQTT_SERVER_IP            "broker.emqx.io"
#define MQTT_SERVER_PORT          1883
#define MQTT_USERNAME             "giirob"    // Descomentar esta línea (y la siguiente) para que se conecte al broker MQTT usando usuario y contraseña
#define MQTT_PASSWORD             "UPV2024"

#define HELLO_TOPIC "giirob/pr2/estacion/hola"
#define CALIDAD_MANDAR_TOPIC          "giirob/pr2/estacion/calidad/mandar"

#define CALIDAD_LEER_TOPIC        "giirob/pr2/estacion/calidad/leer"

#define MENSAJE_RECIBIDO "on"
#define MENSAJE_NEGATIVO_RECIBIDO "off"

#define NO_ANOMALIA "mantener"
#define SI_ANOMALIA "tirar"



enum sensor_estado
{
  DETECTADO,
  DESCONOCIDO,
  NO_DETECTADO
};

enum calidad
{
  BUENO,
  MALO,
  NOSESABE
};

#define DISPENSADOR_LEER_PRESENCIA_TOPIC        "giirob/pr2/estacion/dispensador/leer/presencia"
#define DISPENSADOR_MANDAR_TOPIC      "giirob/pr2/estacion/dispensador/mandar/valvula"
#define DISPENSADOR_MANDAR_MA_TOPIC      "giirob/pr2/estacion/dispensador/mandar/ma"
#define DISPENSADOR_MANDAR_ME_TOPIC      "giirob/pr2/estacion/dispensador/mandar/me"
#define DISPENSADOR_MANDAR_K_TOPIC      "giirob/pr2/estacion/dispensador/mandar/k"
#define DISPENSADOR_MANDAR_S_TOPIC      "giirob/pr2/estacion/dispensador/mandar/s"
#define DISPENSADOR_MANDAR_P_TOPIC      "giirob/pr2/estacion/dispensador/mandar/p"
#define DISPENSADOR_MANDAR_U_TOPIC      "giirob/pr2/estacion/dispensador/mandar/u"

#define DISPENSADOR_LEER_PESO_TOPIC        "giirob/pr2/estacion/dispensador/leer/peso"

#define PINA        "pina"
#define SANDIA      "sandia"
#define MANZANA     "manzana"
#define MELON       "melon"
#define UVA         "uva"
#define KIWI        "kiwi"

#define PRESENCIA_ON "on"
#define PRESENCIA_OFF "off"

#define COND_BUENAS "valvula_abierta"
#define COND_NO_BUENAS "valvula_cerrada"

#define CINTA_ON "on"
#define CINTA_OFF "off"



#define PINUADILD 15

#define PINMAX1 16 
#define PINMIN1 17

#define PINMAX2 18
#define PINMIN2 8

#define PINMAX3 3
#define PINMIN3 46

#define PINMAX4 9
#define PINMIN4 10

#define PINMAX5 11
#define PINMIN5 12

#define PINMAX6 13
#define PINMIN6 14

#define PINBUZZER 7
#define PINHALL 6



