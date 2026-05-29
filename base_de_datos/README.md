# Explicación de la base de datos

## 1. Objetivo general

Este script SQL define la estructura inicial de una base de datos orientada al control de calidad y al seguimiento de stock dentro de un sistema de robótica alimentaria o dispensación de frutas. Su función principal es crear dos tablas: una para registrar el estado de los táperes o envases y otra para almacenar los datos de pesaje de las frutas.

Antes de crear las tablas, el script elimina versiones anteriores si ya existen. Esto permite reiniciar la base de datos de forma limpia durante las pruebas o al volver a desplegar el proyecto.

## 2. Análisis línea por línea

### 2.1 Selección del esquema

```sql
SET search_path TO robofruit;
```

Esta instrucción indica a PostgreSQL que, a partir de ese momento, todas las operaciones se realizarán por defecto dentro del esquema `robofruit`.

Un esquema es un contenedor lógico dentro de la base de datos. Usarlo permite organizar mejor las tablas y evitar conflictos de nombres con otros objetos de la base de datos.

### 2.2 Eliminación de tablas previas

```sql
DROP TABLE IF EXISTS Tupper CASCADE;
DROP TABLE IF EXISTS Stock CASCADE;
```

Estas sentencias eliminan las tablas `Tupper` y `Stock` si ya existen.

- `IF EXISTS` evita que se produzca un error si la tabla todavía no está creada.
- `CASCADE` hace que también se eliminen automáticamente los objetos que dependan de esas tablas, si los hubiera.

Este paso se utiliza normalmente durante el desarrollo para poder recrear la estructura desde cero.

## 3. Tabla Tupper

```sql
CREATE TABLE Tupper (
    id SERIAL PRIMARY KEY,

    estado VARCHAR(20) NOT NULL CHECK (
        estado IN ('Contaminado', 'Valido')
    ),

    fecha TIMESTAMP NOT NULL
);
```

Esta tabla almacena el estado de cada táper o recipiente que entra en el sistema.

### 3.1 Campo id

```sql
id SERIAL PRIMARY KEY,
```

- `SERIAL` genera automáticamente un identificador numérico incremental.
- `PRIMARY KEY` indica que este campo identifica de forma única cada registro.

Gracias a esto, cada táper queda registrado con un identificador irrepetible.

### 3.2 Campo estado

```sql
estado VARCHAR(20) NOT NULL CHECK (
    estado IN ('Contaminado', 'Valido')
),
```

Este campo guarda el resultado del control de calidad.

- `VARCHAR(20)` permite almacenar texto de hasta 20 caracteres.
- `NOT NULL` obliga a que siempre exista un valor.
- `CHECK` restringe los posibles valores a dos opciones concretas:
  - `Contaminado`
  - `Valido`

Esta restricción mejora la integridad de los datos, ya que impide insertar estados no definidos.

### 3.3 Campo fecha

```sql
fecha TIMESTAMP NOT NULL
```

Este campo guarda la fecha y la hora en la que se registró el estado del táper.

El tipo `TIMESTAMP` es adecuado para almacenar información temporal precisa, algo importante en trazabilidad y control de eventos.

## 4. Tabla Stock

```sql
CREATE TABLE Stock (
    id SERIAL PRIMARY KEY,
    fruta VARCHAR(20) NOT NULL CHECK (
        fruta IN ('Piña', 'Manzana', 'Uva', 'Kiwi', 'Sandia', 'Melon')
    ),
    pesaje INT NOT NULL,
    fecha TIMESTAMP NOT NULL
);
```

Esta tabla registra el stock o peso asociado a cada fruta en un momento determinado.

### 4.1 Campo id

```sql
id SERIAL PRIMARY KEY,
```

Al igual que en la tabla anterior, este campo identifica de forma única cada fila.

### 4.2 Campo fruta

```sql
fruta VARCHAR(20) NOT NULL CHECK (
    fruta IN ('Piña', 'Manzana', 'Uva', 'Kiwi', 'Sandia', 'Melon')
),
```

Este campo indica qué fruta se está registrando.

- `VARCHAR(20)` permite guardar el nombre de la fruta.
- `NOT NULL` obliga a que siempre haya una fruta asignada.
- `CHECK` limita los valores a una lista cerrada de frutas válidas.

De este modo, la base de datos solo acepta frutas que realmente forman parte del sistema.

### 4.3 Campo pesaje

```sql
pesaje INT NOT NULL
```

Este campo almacena el valor numérico del peso o cantidad registrada para esa fruta.

El tipo `INT` es suficiente si el sistema trabaja con valores enteros de stock o peso simplificado.

### 4.4 Campo fecha

```sql
fecha TIMESTAMP NOT NULL
```

Este campo registra el momento exacto en el que se produjo la medición.

Esto permite analizar la evolución del stock a lo largo del tiempo.

## 5. Relación entre las tablas

Las tablas `Tupper` y `Stock` no están relacionadas directamente mediante claves foráneas en este script. Cada una cumple una función distinta:

- `Tupper` almacena el resultado del control de calidad.
- `Stock` almacena el estado de inventario o pesaje de frutas.

Aunque no exista relación física entre ellas, forman parte del mismo sistema lógico de monitorización. Ambas ayudan a tomar decisiones sobre si un táper es válido y si hay stock suficiente de fruta.

## 6. Ventajas del diseño

Este diseño presenta varias ventajas:

- Es simple y fácil de entender.
- Utiliza restricciones `CHECK` para evitar datos incorrectos.
- Guarda la información con fecha, lo que facilita la trazabilidad.
- Permite reinicializar la base de datos fácilmente durante pruebas.
- Se adapta bien a un proyecto académico o prototipo funcional.

## 7. Posibles mejoras

Aunque la estructura es correcta para un prototipo, se podrían añadir mejoras en una versión más avanzada:

- Incluir claves foráneas si se quiere relacionar táperes y registros de stock.
- Añadir campos como `usuario`, `observaciones` o `origen`.
- Usar `TIMESTAMP WITH TIME ZONE` si se requiere gestión horaria precisa.
- Añadir índices si el volumen de registros crece mucho.

## 8. Conclusión

El script define una base de datos mínima pero funcional para un sistema de control de calidad y stock. La tabla `Tupper` permite registrar si un envase es válido o contaminado, mientras que la tabla `Stock` guarda el peso de cada fruta junto con su fecha de registro. El uso de restricciones y tipos adecuados garantiza una estructura clara, controlada y útil para una memoria técnica del proyecto.
