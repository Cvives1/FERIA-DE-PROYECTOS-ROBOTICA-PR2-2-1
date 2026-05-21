long now, lastMsg = 0;
long sensorsUpdateInterval = 8000; // tiempo de actualización de los sensores

void on_loop() {
  comprobarImanYAlerta(PINHALL, PINBUZZER);

  now = millis();
  if (now - lastMsg > sensorsUpdateInterval ) {
    lastMsg = now;
    
    


    

  }

}

