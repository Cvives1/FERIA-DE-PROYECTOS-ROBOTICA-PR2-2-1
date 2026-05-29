# Feria de Proyectos de Robotica

Repositorio organizado para separar la parte del ESP32, la base de datos y la estacion RoboDK.

## Estructura del repositorio

```text
arduino/
  ESP32-S3-IoT-Device/
    codigo del ESP32-S3
  __MACOSX/ESP32-S3-IoT-Device/

base_de_datos/

estacion_RoboDK/
```

## Organizacion

### `arduino/ESP32-S3-IoT-Device`
Aqui se encuentra el codigo principal del proyecto, en arduino.

### `base_de_datos/`
Carpeta reservada para todo lo relacionado con la base de datos del proyecto: scripts, exportaciones, estructura de tablas o ficheros de apoyo.

### `estacion_RoboDK/`
Carpeta reservada para la estacion de trabajo de RoboDK, simulaciones, programas o archivos asociados a la parte robotica.

## Objetivo de esta organizacion

La estructura del repositorio busca que cada parte del proyecto quede separada y sea facil de localizar:

- codigo Arduino en una sola carpeta
- informacion de base de datos en otra
- archivos de RoboDK en su propio bloque

