# EL CODIGO SE ENCUENTRA EN LA CARPETA `ESP32-S3-IoT-Device`  
# Explicacion detallada del codigo 

## 1. Visión general del proyecto

Este proyecto implementa un dispositivo IoT sobre un ESP32-S3 que se comunica por WiFi y MQTT con un broker externo. El código está dividido en varios archivos para separar responsabilidades y facilitar su mantenimiento. La idea principal es que el sistema reciba datos desde sensores y mensajes MQTT, los procese de forma concurrente mediante tareas de FreeRTOS y publique respuestas o estados de control hacia otros sistemas.

La estructura general del programa es la siguiente:

- `ESP32-S3-IoT-Device.ino`: punto de entrada principal, donde se ejecuta `setup()` y `loop()`.
- `Config.h`: archivo de configuración global, con credenciales, topics MQTT, constantes y pines.
- `f_funciones.ino`: funciones auxiliares reutilizables.
- `g_comunicaciones.ino`: comunicaciones MQTT, colas, tareas FreeRTOS e interrupciones.
- `s_setup.ino`: inicialización específica del sistema.
- `w_loop.ino`: lógica periódica del bucle principal.

Este diseño permite que el código esté ordenado, modular y fácil de extender.

## 2. Archivo Config.h

El archivo `Config.h` reúne toda la configuración estática del sistema. Su objetivo es centralizar valores que no deberían estar repartidos por el resto del programa.

### 2.1 Comunicación serie y depuración

Se define la velocidad del puerto serie con:

- `BAUDS 115200`

También se activa el logger por consola con:

- `LOGGER_ENABLED`

y se fija el nivel de detalle del log con:

- `LOG_LEVEL TRACE`

Esto permite ver mensajes de diagnóstico durante la ejecución del programa.

### 2.2 Identidad del dispositivo

Se define un identificador del dispositivo:

- `DEVICE_GIIROB_PR2_ID "Miesp32"`

Ese identificador se usa para construir el `deviceID` MQTT en el archivo principal. Gracias a ello, cada placa puede distinguirse del resto cuando se conecta al broker.

### 2.3 Configuración WiFi

Se almacenan el nombre de la red y la contraseña:

- `NET_SSID`
- `NET_PASSWD`

El programa utiliza estos valores para conectar automáticamente el ESP32 a la red local al arrancar.

### 2.4 Configuración MQTT

Se definen los parámetros de conexión al broker:

- `MQTT_SERVER_IP`
- `MQTT_SERVER_PORT`
- `MQTT_USERNAME`
- `MQTT_PASSWORD`

En este caso el broker es `broker.emqx.io` y la conexión se realiza por el puerto estándar `1883`.

### 2.5 Topics MQTT del sistema de calidad

Se declaran los topics asociados al control de calidad:

- `CALIDAD_MANDAR_TOPIC`
- `CALIDAD_LEER_TOPIC`

Además, se definen las cadenas de estado que se usan dentro de los mensajes JSON:

- `MENSAJE_RECIBIDO = "on"`
- `MENSAJE_NEGATIVO_RECIBIDO = "off"`
- `NO_ANOMALIA = "mantener"`
- `SI_ANOMALIA = "tirar"`

Estas cadenas sirven para traducir entre el lenguaje del MQTT y los estados internos del programa.

### 2.6 Topics MQTT del dispensador

Se crean varios topics para el bloque del dispensador:

- `DISPENSADOR_LEER_PRESENCIA_TOPIC`
- `DISPENSADOR_MANDAR_TOPIC`
- `DISPENSADOR_MANDAR_MA_TOPIC`
- `DISPENSADOR_MANDAR_ME_TOPIC`
- `DISPENSADOR_MANDAR_K_TOPIC`
- `DISPENSADOR_MANDAR_S_TOPIC`
- `DISPENSADOR_MANDAR_P_TOPIC`
- `DISPENSADOR_MANDAR_U_TOPIC`
- `DISPENSADOR_LEER_PESO_TOPIC`

También se declaran las cadenas que identifican cada fruta dentro del JSON de pesos:

- `PINA`
- `SANDIA`
- `MANZANA`
- `MELON`
- `UVA`
- `KIWI`

Y los estados que se publican sobre la cinta o válvula:

- `CINTA_ON = "on"`
- `CINTA_OFF = "off"`

### 2.7 Enumeraciones internas

Se usan dos enumeraciones para manejar estados de forma más clara dentro del código:

- `sensor_estado`: `DETECTADO`, `DESCONOCIDO`, `NO_DETECTADO`
- `calidad`: `BUENO`, `MALO`, `NOSESABE`

Estas enumeraciones convierten valores textuales en estados lógicos más manejables.

### 2.8 Pines físicos

Por último, se fijan los pines conectados a la protoboard y periféricos:

- `PINUADILD` para el botón
- `PINMAX1` / `PINMIN1` y el resto de pares para los LEDs de stock
- `PINBUZZER` para el zumbador
- `PINHALL` para el sensor Hall

El archivo `Config.h` es, en resumen, la base de parametrización de todo el sistema.

## 3. Archivo f_funciones.ino

Este archivo contiene funciones auxiliares que se reutilizan en distintas partes del programa.

### 3.1 Función ilumina_led

La función `ilumina_led(int pinmin, int pinmax, int stock)` representa el nivel de stock mediante dos pines.

Su comportamiento está dividido en tres niveles:

- Si `stock < 5`, apaga ambos pines configurándolos como entrada.
- Si `stock` está entre 5 y 29, activa solo el pin máximo y deja el mínimo apagado.
- Si `stock >= 30`, activa ambos pines.

Esta estrategia permite mostrar visualmente tres estados de inventario:

- sin stock
- stock medio
- stock alto

La función se aplica a cada fruta para informar rápidamente del estado de los depósitos.

### 3.2 Función comprobarImanYAlerta

La función `comprobarImanYAlerta(int pinhall, int pinbuzz)` lee el sensor Hall y activa el buzzer cuando detecta el estado de alarma.

Su lógica usa dos variables estáticas:

- `pinesConfigurados`, para inicializar los pines solo una vez
- `ultimaLectura`, para evitar repetir mensajes por consola en cada iteración

Si el sensor devuelve `LOW`, el buzzer se activa y se imprime el mensaje de que el imán no se detecta. Si devuelve otro valor, el buzzer se desactiva y se informa de que el imán sí está detectado.

Esta función es importante porque añade una alerta física y una confirmación visual por consola.

## 4. Archivo g_comunicaciones.ino

Este es el archivo más importante del proyecto, porque implementa la lógica de comunicaciones, la gestión de colas, las tareas concurrentes y la interrupción del botón.

### 4.1 Estructuras de datos

Se define una estructura llamada `PesosFrutas` para agrupar los pesos de todas las frutas:

- `pina`
- `sandia`
- `manzana`
- `melon`
- `uva`
- `kiwi`

También se crean estructuras de parámetros para las tareas:

- `ParametrosTareaStock`
- `ParametrosTareaCalidad`

Estas estructuras contienen los identificadores de colas que cada tarea necesita para trabajar. De esta forma, las tareas reciben sus datos por parámetro y no dependen de variables globales innecesarias.

### 4.2 Colas FIFO

Se declaran tres colas globales:

- `qPresenciaStock`
- `qPesosStock`
- `qPresenciaCalidad`

Las colas se crean con tamaño 1 y usan `xQueueOverwrite`, lo que significa que siempre se conserva el último dato recibido. Este enfoque es muy útil en sistemas en tiempo real donde solo interesa el valor más reciente.

### 4.3 Suscripción a topics

La función `suscribirseATopics()` se encarga de suscribir el dispositivo a los topics MQTT de entrada:

- presencia del dispensador
- peso de frutas
- calidad

Con esto el ESP32 puede recibir mensajes del exterior y reaccionar a ellos.

### 4.4 Publicación de estados

La función `publicarEstado(const char* topic, const char* estado)` construye un JSON con un único campo llamado `estado` y lo publica en el topic indicado.

Esto mantiene un formato homogéneo de mensajes y facilita que otros nodos o sistemas interpreten la información.

### 4.5 Recepción de mensajes MQTT

La función `alRecibirMensajePorTopic(char* topic, String incomingMessage)` es el callback lógico que procesa los mensajes entrantes.

Primero convierte el texto recibido desde MQTT en un documento JSON con `ArduinoJson`. Después, según el topic, realiza una acción distinta:

#### a) Presencia del dispensador

Si el mensaje pertenece a `DISPENSADOR_LEER_PRESENCIA_TOPIC`, lee el campo `estado` y lo traduce a un valor interno de tipo `sensor_estado`.

- `on` se interpreta como `DETECTADO`
- `off` se interpreta como `NO_DETECTADO`

Ese estado se escribe en la cola `qPresenciaStock`.

#### b) Pesos de frutas

Si el mensaje pertenece a `DISPENSADOR_LEER_PESO_TOPIC`, se construye una estructura `PesosFrutas` con los valores del JSON.

Después, cada peso se usa para actualizar la iluminación de los LEDs mediante `ilumina_led`. Finalmente, toda la estructura se guarda en `qPesosStock`.

#### c) Calidad

Si el mensaje pertenece a `CALIDAD_LEER_TOPIC`, también se lee el campo `estado` y se traduce a `sensor_estado`.

- `on` pasa a `DETECTADO`
- `off` pasa a `NO_DETECTADO`

Ese dato se escribe en `qPresenciaCalidad`.

La idea clave es que el callback no ejecuta toda la lógica directamente, sino que convierte los mensajes en datos internos y los entrega a las tareas.

### 4.6 Interrupción del botón

El sistema usa una variable global volátil llamada `sistemaParado`.

La rutina de interrupción `boton()` se ejecuta cuando se detecta un flanco de bajada en el botón. Dentro de ella se hace muy poco:

- se aplica antirrebote temporal
- se activa `sistemaParado = true`

Esto es correcto porque una ISR debe ser breve y no debe ejecutar tareas pesadas como publicar MQTT o imprimir demasiado.

### 4.7 Tarea de control de stock

La función `tareaControlStockRobot(void * pvParameters)` implementa una tarea FreeRTOS que controla el stock y decide si el robot puede avanzar.

Su comportamiento es el siguiente:

1. Comprueba si el botón de parada ha sido pulsado.
2. Lee los últimos valores disponibles en la cola de presencia.
3. Lee los últimos valores disponibles en la cola de pesos.
4. Publica el estado de cada fruta en su topic correspondiente.
5. Evalúa si el robot puede continuar o si debe detenerse.

La lógica de decisión es sencilla:

- Si hay presencia detectada y todas las frutas tienen al menos 5 unidades, el sistema considera que todo está correcto y publica `valvula_abierta`.
- Si falta presencia o alguna fruta baja de 5, publica `valvula_cerrada`.

Además, si un peso alcanza 30 o más, la cinta asociada se apaga con `off`; si no, permanece en `on`.

La tarea se ejecuta cada 10 segundos, por lo que actúa como un controlador periódico.

### 4.8 Tarea de control de calidad

La función `tareaControlCalidad(void * pvParameters)` se encarga de la lógica de calidad.

Esta tarea espera bloqueada a que llegue un nuevo mensaje en la cola de presencia de calidad. Eso se hace con `portMAX_DELAY`, por lo que no consume CPU mientras no haya eventos.

Cuando llega un mensaje `on`, se simula una evaluación aleatoria del táper:

- en la mayoría de casos se considera `BUENO`
- en una parte menor de casos se considera `MALO`

Según el resultado, se publica:

- `mantener` si la calidad es buena
- `tirar` si se detecta una anomalía

Si llega `off`, la tarea no hace ninguna acción aleatoria.

### 4.9 Inicialización de colas y tareas

La función `inicializarTareasComunicaciones()` realiza la puesta en marcha del sistema concurrente.

Sus pasos son:

1. Configurar el botón como `INPUT_PULLUP`.
2. Crear las colas FIFO.
3. Asociar cada cola con la estructura de parámetros de su tarea.
4. Crear las tareas con `xTaskCreatePinnedToCore`.
5. Asociar la interrupción del botón con `attachInterrupt`.

En la práctica, esta función deja preparado todo el sistema de comunicación y control.

## 5. Archivo s_setup.ino

Este archivo contiene la función `on_setup()`.

Su papel es muy simple: llama a `inicializarTareasComunicaciones()` y muestra un mensaje por serie indicando que el hardware y las tareas de FreeRTOS se han inicializado correctamente.

Aunque parece pequeño, cumple una función importante: centraliza la inicialización específica del proyecto y separa esa lógica del `setup()` principal.

## 6. Archivo w_loop.ino

El archivo `w_loop.ino` contiene la función `on_loop()`.

Actualmente su trabajo es mínimo:

- llama a `comprobarImanYAlerta(PINHALL, PINBUZZER)`
- mantiene una estructura temporal basada en `millis()`
- reserva un intervalo de 8 segundos para posibles acciones futuras

En la versión actual, el bloque periódico está prácticamente vacío, así que este archivo actúa como espacio de supervisión y extensión futura.

## 7. Archivo principal ESP32-S3-IoT-Device.ino

Aunque no lo hayas pedido de forma explícita, este archivo es el que conecta todas las piezas.

En `setup()` se hace lo siguiente:

1. Inicializar el puerto serie si el logger está activo.
2. Conectar a la red WiFi.
3. Conectar al broker MQTT.
4. Suscribirse a los topics necesarios.
5. Llamar a `on_setup()`.

En `loop()` se ejecuta:

1. `wifi_loop()` para mantener la conexión WiFi.
2. `mqtt_loop()` para mantener la conexión MQTT y procesar mensajes.
3. `on_loop()` para ejecutar la lógica periódica del proyecto.

Esto hace que el archivo principal funcione como coordinador general del sistema.

## 8. Flujo general de funcionamiento

El funcionamiento completo del sistema puede resumirse así:

1. El ESP32 arranca.
2. Se conecta a WiFi y a MQTT.
3. Se suscribe a los topics de entrada.
4. Se crean las colas y tareas de FreeRTOS.
5. Llegan mensajes MQTT con presencia, pesos o calidad.
6. El callback traduce esos mensajes a estados internos y los guarda en colas.
7. Las tareas procesan los datos y toman decisiones.
8. El sistema publica respuestas a través de MQTT.
9. El sensor Hall y el buzzer se supervisan desde el bucle principal.
10. El botón de emergencia puede parar las tareas mediante interrupción.

## 9. Conclusión

El proyecto está bien estructurado para un sistema IoT basado en ESP32-S3, porque separa configuración, funciones auxiliares, comunicaciones y lógica de control. Además, el uso de MQTT permite integrar el dispositivo con otros nodos o plataformas externas, mientras que FreeRTOS aporta concurrencia y una organización clara entre tareas.

Desde el punto de vista académico, este código demuestra el uso conjunto de:

- comunicación WiFi
- publicación y suscripción MQTT
- JSON para estructurar datos
- interrupciones por hardware
- colas FIFO
- tareas FreeRTOS
- control de actuadores y sensores

