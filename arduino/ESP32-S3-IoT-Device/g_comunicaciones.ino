#include <ArduinoJson.h>

portMUX_TYPE taskMux = portMUX_INITIALIZER_UNLOCKED;
// =====================================================
// ESTRUCTURAS DE DATOS PARA LOS BUFFERS (FIFO QUERIES)
// =====================================================

// Estructura para almacenar el bloque completo de pesos de frutas
struct PesosFrutas {
  int pina;
  int sandia;
  int manzana;
  int melon;
  int uva;
  int kiwi;
};

// Estructuras que actúan como parámetros de creación de las tareas.
// Contienen las colas (buffers FIFO) que cada tarea necesita gestionar.
struct ParametrosTareaStock {
  QueueHandle_t colaPresencia; // Buffer FIFO 1
  QueueHandle_t colaPesos;     // Buffer FIFO 2 -> ¡Maneja más de un buffer!
};

struct ParametrosTareaCalidad {
  QueueHandle_t colaPresencia; // Buffer FIFO 1
};

// Descriptores globales de las colas para que la recepción MQTT pueda escribir en ellos
QueueHandle_t qPresenciaStock = NULL;
QueueHandle_t qPesosStock = NULL;
QueueHandle_t qPresenciaCalidad = NULL;

// Instancias estáticas de los parámetros para asegurar que persistan en memoria
ParametrosTareaStock paramsStock;
ParametrosTareaCalidad paramsCalidad;


// Descriptores de las tareas para poder enviarles notificaciones
TaskHandle_t hTareaStock = NULL;
TaskHandle_t hTareaCalidad = NULL;

// =====================================================
// FUNCIONES AUXILIARES / PUBLICACIÓN MQTT
// =====================================================

void suscribirseATopics() {
  mqtt_subscribe(DISPENSADOR_LEER_PRESENCIA_TOPIC);
  mqtt_subscribe(DISPENSADOR_LEER_PESO_TOPIC);
  mqtt_subscribe(CALIDAD_LEER_TOPIC);
}

void publicarEstado(const char* topic, const char* estado) {
  StaticJsonDocument<128> doc;
  doc["estado"] = estado;

  char buffer[128];
  serializeJson(doc, buffer);
  mqtt_publish(topic, buffer);
}

// =====================================================
// RECEPCIÓN MQTT (ESCRIBE EN LOS BUFFERS FIFO)
// =====================================================

void alRecibirMensajePorTopic(char* topic, String incomingMessage) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, incomingMessage);

  if (error) {
    Serial.print("Error parseando JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // --- SENSOR PRESENCIA DISPENSADOR (Código 1) ---
  if (strcmp(topic, DISPENSADOR_LEER_PRESENCIA_TOPIC) == 0) {
    const char* estado = doc["estado"];
    sensor_estado est = DESCONOCIDO;

    if (strcmp(estado, PRESENCIA_ON) == 0)        est = DETECTADO;
    else if (strcmp(estado, PRESENCIA_OFF) == 0)   est = NO_DETECTADO;

    // Enviar al buffer FIFO de la tarea de Stock (No bloqueante desde interrupción/callback)
    if (qPresenciaStock != NULL) {
      xQueueOverwrite(qPresenciaStock, &est); 
    }
  }

  // --- PESOS FRUTAS (Código 1) ---
  if (strcmp(topic, DISPENSADOR_LEER_PESO_TOPIC) == 0) {
    PesosFrutas nuevosPesos;
    nuevosPesos.pina    = doc[PINA];
    nuevosPesos.sandia  = doc[SANDIA];
    nuevosPesos.manzana = doc[MANZANA];
    nuevosPesos.melon   = doc[MELON];
    nuevosPesos.uva     = doc[UVA];
    nuevosPesos.kiwi    = doc[KIWI];

    ilumina_led(PINMIN1, PINMAX1, nuevosPesos.pina);
    ilumina_led(PINMIN2, PINMAX2, nuevosPesos.sandia);
    ilumina_led(PINMIN3, PINMAX3, nuevosPesos.manzana);
    ilumina_led(PINMIN4, PINMAX4, nuevosPesos.melon);
    ilumina_led(PINMIN5, PINMAX5, nuevosPesos.uva);
    ilumina_led(PINMIN6, PINMAX6, nuevosPesos.kiwi);


    // Enviar al buffer FIFO de pesos
    if (qPesosStock != NULL) {
      xQueueOverwrite(qPesosStock, &nuevosPesos);
    }
  }

  // --- TOPIC CALIDAD (Código 2) ---
  if (strcmp(topic, CALIDAD_LEER_TOPIC) == 0) {
    const char* estado = doc["estado"];
    sensor_estado est = DESCONOCIDO;

    if (strcmp(estado, MENSAJE_RECIBIDO) == 0)           est = DETECTADO;
    else if (strcmp(estado, MENSAJE_NEGATIVO_RECIBIDO) == 0) est = NO_DETECTADO;

    // Enviar al buffer FIFO de la tarea de Calidad
    if (qPresenciaCalidad != NULL) {
      xQueueOverwrite(qPresenciaCalidad, &est);
    }
  }
}


//INTERRUPCIÓN BOTÓN

// Variable global volátil (visible desde ISR y tareas)
volatile bool sistemaParado = false;

void IRAM_ATTR boton() {
  static unsigned long long tiempoUltimoClic = 0;
  unsigned long long tiempoActual = esp_timer_get_time();

  if (tiempoActual - tiempoUltimoClic > 250000ULL) {
    tiempoUltimoClic = tiempoActual;
    sistemaParado = true;  // Solo escribir el flag, nada más
  }
}
// =====================================================
// TAREAS ASÍNCRONAS (ACCEDEN SÓLO POR PARÁMETRO)
// =====================================================

// Tarea 1: Maneja DOS buffers FIFO (Presencia y Pesos) recibidos por parámetro
void tareaControlStockRobot(void * pvParameters) {
  ParametrosTareaStock* misBuffers = (ParametrosTareaStock*) pvParameters;
  
  // Variables que MANTIENEN el estado (no se borran en cada vuelta)
  sensor_estado estado_presencia = DESCONOCIDO;
  PesosFrutas pesos = {0, 0, 0, 0, 0, 0}; 
  uint32_t valorNotificacion;

  for(;;) {
    // 1. Comprobar botón de parada de emergencia
    if (sistemaParado) {
        Serial.println("[PARADA] Sistema detenido por botón.");
        vTaskSuspend(NULL);  // Se suspende a sí misma (NULL = tarea actual)
    }

    // 2. Leer presencia (Solo actualiza si hay algo nuevo en la cola)
    if (misBuffers->colaPresencia != NULL) {
      sensor_estado presenciaTemporal;
      if (xQueueReceive(misBuffers->colaPresencia, &presenciaTemporal, 0) == pdTRUE) {
        estado_presencia = presenciaTemporal;
      }
    }

    // 3. Leer pesos (Solo actualiza si hay algo nuevo en la cola)
    if (misBuffers->colaPesos != NULL) {
      PesosFrutas pesosTemporales;
      if (xQueueReceive(misBuffers->colaPesos, &pesosTemporales, 0) == pdTRUE) {
        pesos = pesosTemporales; // Copiamos los nuevos pesos
      }
    }

    // --- CONTROL STOCK ---
    // Si el peso es >= 30 mandará CINTA_OFF, si es menor mandará CINTA_ON
    publicarEstado(DISPENSADOR_MANDAR_P_TOPIC,  (pesos.pina >= 30)  ? CINTA_OFF : CINTA_ON);
    publicarEstado(DISPENSADOR_MANDAR_MA_TOPIC, (pesos.manzana >= 30) ? CINTA_OFF : CINTA_ON);
    publicarEstado(DISPENSADOR_MANDAR_ME_TOPIC, (pesos.melon >= 30) ? CINTA_OFF : CINTA_ON);
    publicarEstado(DISPENSADOR_MANDAR_U_TOPIC,  (pesos.uva >= 30)  ? CINTA_OFF : CINTA_ON);
    publicarEstado(DISPENSADOR_MANDAR_K_TOPIC,  (pesos.kiwi >= 30)  ? CINTA_OFF : CINTA_ON);
    publicarEstado(DISPENSADOR_MANDAR_S_TOPIC,  (pesos.sandia >= 30)  ? CINTA_OFF : CINTA_ON);

    // --- CONTROL ROBODK ---
    StaticJsonDocument<128> doc;
    char buffer[128];

    if (estado_presencia == DETECTADO && 
        pesos.manzana >= 5 && pesos.pina >= 5  && pesos.sandia >= 5 &&
        pesos.melon >= 5   && pesos.kiwi >= 5  && pesos.uva >= 5) {
      
      doc["estado"] = COND_BUENAS;
      serializeJson(doc, buffer);
      mqtt_publish(DISPENSADOR_MANDAR_TOPIC, buffer);
      Serial.println("[Tarea Robot] ROBODK: Todo correcto, avanzando.");
    } 
    else if (estado_presencia == NO_DETECTADO ||
             pesos.manzana < 5  || pesos.pina < 5    || pesos.sandia < 5 || 
             pesos.melon < 5   || pesos.kiwi < 5   || pesos.uva < 5) {
      
      doc["estado"] = COND_NO_BUENAS;
      serializeJson(doc, buffer);
      mqtt_publish(DISPENSADOR_MANDAR_TOPIC, buffer);
      Serial.println("[Tarea Robot] PARAR ROBODK: Faltan ingredientes o no hay táper.");
    }
    else {
      Serial.println("[Tarea Robot] NO HAGO NADA");

      doc["estado"] = COND_NO_BUENAS;
      serializeJson(doc, buffer);
      mqtt_publish(DISPENSADOR_MANDAR_TOPIC, buffer);
    }

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

// Tarea 2: Control de Calidad Aleatorio (REPARADA)
void tareaControlCalidad(void * pvParameters) {
  ParametrosTareaCalidad* misBuffers = (ParametrosTareaCalidad*) pvParameters;
  sensor_estado estado_presencia;
  calidad taper = NOSESABE;
  uint32_t valorNotificacion;

  for(;;) {
    // 1. Comprobar si el botón de parada fue pulsado
    if (sistemaParado) {
        Serial.println("[PARADA] Sistema detenido por botón.");
        vTaskSuspend(NULL);  // Se suspende a sí misma (NULL = tarea actual)
    }

    // 2. ARREGLADO: Bloqueo absoluto (portMAX_DELAY). La tarea se queda totalmente 
    // congelada sin consumir CPU hasta que entre un nuevo mensaje por el topic de calidad.
    if (misBuffers->colaPresencia != NULL) {
      if (xQueueReceive(misBuffers->colaPresencia, &estado_presencia, portMAX_DELAY) == pdTRUE) {
        
        // 3. --- LÓGICA DE RESPUESTA ÚNICA ---
        if (estado_presencia == DETECTADO) {
          Serial.println("[CALIDAD] Evento ON recibido. Evaluando táper...");
          
          int p_anomalia = random(1, 5);
          taper = (p_anomalia == 1) ? MALO : BUENO;

          if (taper == BUENO) {
            Serial.println("[CALIDAD] Resultado: TAPER BUENO. Respondiendo...");
            publicarEstado(CALIDAD_MANDAR_TOPIC, NO_ANOMALIA);
          }
          else if (taper == MALO) {
            Serial.println("[CALIDAD] Resultado: TAPER MALO. Respondiendo...");
            publicarEstado(CALIDAD_MANDAR_TOPIC, SI_ANOMALIA);
          }
        } 
        else if (estado_presencia == NO_DETECTADO) {
          Serial.println("[CALIDAD] Evento OFF recibido. No se realiza ninguna acción aleatoria.");
        }
      }
    }
  }
}


// =====================================================
// INICIALIZACIÓN DE BUFFERS Y TAREAS (Llamar en el Setup)
// =====================================================
void inicializarTareasComunicaciones() {
  
  pinMode(PINUADILD, INPUT_PULLUP);
  // 1. Creación de los Buffers FIFO (Queues de longitud 1 con sobrescritura para datos en tiempo real)
  qPresenciaStock   = xQueueCreate(1, sizeof(sensor_estado));
  qPesosStock       = xQueueCreate(1, sizeof(PesosFrutas));
  qPresenciaCalidad = xQueueCreate(1, sizeof(sensor_estado));

  // 2. Empaquetar los buffers correspondientes para cada Tarea
  paramsStock.colaPresencia = qPresenciaStock;
  paramsStock.colaPesos     = qPesosStock;      // Aquí pasamos el segundo buffer de esta tarea

  paramsCalidad.colaPresencia = qPresenciaCalidad;

  // 3. Creación de las tareas enviando los buffers ÚNICAMENTE mediante el parámetro (último argumento)
  xTaskCreatePinnedToCore(
    tareaControlStockRobot,
    "ControlStockRobot",
    4096,
    &paramsStock,        // <--- Pasado estrictamente por parámetro de creación
    1,
    &hTareaStock,
    1
  );

  xTaskCreatePinnedToCore(
    tareaControlCalidad,
    "ControlCalidad",
    4096,
    &paramsCalidad,      // <--- Pasado estrictamente por parámetro de creación
    1,
    &hTareaCalidad,
    1
  );

  attachInterrupt(digitalPinToInterrupt(PINUADILD),boton, FALLING);
}