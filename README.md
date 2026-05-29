# Feria de Proyectos de Robotica

Este README solo explica la organizacion general del repositorio. Para la explicacion tecnica del codigo, la referencia es la carpeta del proyecto Arduino.

## Estructura del repositorio

```text
arduino/
  ESP32-S3-IoT-Device/
    codigo principal del ESP32-S3  

  __MACOSX/ESP32-S3-IoT-Device/  (carpeta auxiliar del paquete)

base_de_datos/

estacion_RoboDK/
```

## Organizacion

### `arduino/ESP32-S3-IoT-Device`
Aqui se encuentra el codigo principal del proyecto, en arduino.

### `base_de_datos/`
Carpeta reservada para todo lo relacionado con la base de datos del proyecto.

### `estacion_RoboDK/`
Carpeta reservada para la estacion de trabajo de RoboDK.

## Objetivo de esta organizacion

La estructura del repositorio busca que cada parte del proyecto quede separada y sea facil de localizar:

- codigo Arduino en una sola carpeta
- informacion de base de datos en otra
- archivos de RoboDK en su propio bloque

