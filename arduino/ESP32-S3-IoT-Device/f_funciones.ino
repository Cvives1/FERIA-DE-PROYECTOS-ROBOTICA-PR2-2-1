
void ilumina_led(int pinmin, int pinmax, int stock) {
  if (stock < 5) {
    // Tramo 1: Sin stock (Menor que 5) -> Desactiva la salida de ambos pines
    pinMode(pinmax, INPUT);
    pinMode(pinmin, INPUT);
  } 
  else if (stock < 30) {
    // Tramo 2: Stock medio (Entre 5 y 29) -> Activa salida en mínimo, desactiva máximo
    pinMode(pinmax, OUTPUT);
    digitalWrite(pinmax, HIGH); // Asegura que tenga voltaje al activarse
    
    pinMode(pinmin, INPUT);
  } 
  else {
    // Tramo 3: Stock lleno (30 o más) -> Activa la salida en ambos pines
    pinMode(pinmax, OUTPUT);
    digitalWrite(pinmax, HIGH);
    
    pinMode(pinmin, OUTPUT);
    digitalWrite(pinmin, HIGH);
  }
}



void comprobarImanYAlerta(int pinhall, int pinbuzz) {
  static bool pinesConfigurados = false;
  if (!pinesConfigurados) {
    // CAMBIO CLAVE: Activamos la resistencia interna para que el pin no se quede sordo
    pinMode(pinhall, INPUT_PULLUP);   
    pinMode(pinbuzz, OUTPUT);  
    pinesConfigurados = true; 
  }

  static int ultimaLectura = -1; 
  int lecturaSensor = digitalRead(pinhall);

  // =======================================================================
  // OPCIÓN 1: Lógica Directa (Si tu sensor de verdad manda HIGH con imán)
  // =======================================================================
  if (lecturaSensor == LOW) { 
    digitalWrite(pinbuzz, HIGH); 
    if (ultimaLectura != lecturaSensor) {
      Serial.println("¡Imán NO DETECTADO!");
      ultimaLectura = lecturaSensor;
    }
  } 
  else {
    digitalWrite(pinbuzz, LOW);  
    if (ultimaLectura != lecturaSensor) {
      Serial.println("Imán NO retirado");
      ultimaLectura = lecturaSensor;
    }
  }}