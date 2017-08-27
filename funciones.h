void filtro1(float temperatura, float humedad){
  static float hume[10];
  static float temp[10];
  static int x = 0;
  static int y = 0;
  static int humedad_max_i = 0;
  static int humedad_min_i = 0;
  static float humedad_max = 0.0;
  static float humedad_min = 0.0;
  static float humedad_suma = 0.0;
  static int temperatura_max_i = 0;
  static int temperatura_min_i = 0;
  static float temperatura_max = 0.0;
  static float temperatura_min = 0.0;
  static float temperatura_suma = 0.0;
  
  if ((humedad >= 0.0) && (humedad <=100.0) && (temperatura >= -20.0) && (temperatura <= 60.0)) {  /* Filtro de medida coerente */
  temperatura = temperatura +  (0.0);   /* OFFSET por si la medida no es correcta */
  humedad = humedad + (0.0);            /* OFFSET por si la medida no es correcta */
  /*Calculo de la humedad filtrada*/
  if (x < 10) { hume[x] = humedad; temp[x] = temperatura; x++; } 
  if (x >= 10) { x=0; }

  humedad_max = 0.0;                 /* Inicializamos variables */
  humedad_min = 100.0;               /* Inicializamos variables */
  temperatura_max = -30.0;           /* Inicializamos variables */
  temperatura_min = 100.0;           /* Inicializamos variables */
  for(int q=0;q<10;q++) {            /* Busqueda del mayor y el menor de la humedad y la temperatura */
    if (hume[q] > humedad_max) {     /* Buscamos el mayor de la humedad */
      humedad_max = hume[q];
      humedad_max_i = q;
    }
    if (hume[q] < humedad_min) {     /* Buscamos el menor de la humedad */
      humedad_min = hume[q];
      humedad_min_i = q;
    }
    if (temp[q] > temperatura_max) { /* Buscamos el mayor de la temperatura */
      temperatura_max = temp[q];
      temperatura_max_i = q;
    }
    if (temp[q] < temperatura_min) { /* Buscamos el menor de la temperatura */
      temperatura_min = temp[q];
      temperatura_min_i = q;
    }
  }
  humedad_suma = 0.0;               
  temperatura_suma = 0.0;
  for(int q=0;q<10;q++) { 
    if (humedad_max_i == humedad_min_i) {if (humedad_max_i < 8) {humedad_max_i++;} else {humedad_max_i--;}}   /* Si se produce el caso singular de encontrar todos los valores iguales, se establece un maximo y un mínimo forzado */
    if ((q != humedad_max_i) && (q != humedad_min_i)) { /* Discriminamos el mayor y el menor */
      humedad_suma = humedad_suma + hume[q];            /* Sumamos los 8 valores validos */
    }
    if (temperatura_max_i == temperatura_min_i) {if (temperatura_max_i < 8) {temperatura_max_i++;} else {temperatura_max_i--;} }  /* Si se produce el caso singular de encontrar todos los valores iguales, se establece un maximo y un mínimo forzado */
    if ((q != temperatura_max_i)&&(q != temperatura_min_i)) {  /* Discriminamos el mayor y el menor */
      temperatura_suma = temperatura_suma + temp[q];           /* Sumamos los 8 valores validos */
    }
  }
  humedad_filtrada1 = humedad_suma/8;                        /* Realizamos la media */                                                                 /* Salida de la función */
  temperatura_filtrada1 = temperatura_suma/8;                /* Realizamos la media */                                                                 /* Salida de la función */
  }
}

void filtro2(float temperatura, float humedad){
  static float hume[10];
  static float temp[10];
  static int x = 0;
  static int y = 0;
  static int humedad_max_i = 0;
  static int humedad_min_i = 0;
  static float humedad_max = 0.0;
  static float humedad_min = 0.0;
  static float humedad_suma = 0.0;
  static int temperatura_max_i = 0;
  static int temperatura_min_i = 0;
  static float temperatura_max = 0.0;
  static float temperatura_min = 0.0;
  static float temperatura_suma = 0.0;
  
  if ((humedad >= 0.0) && (humedad <=100.0) && (temperatura >= -20.0) && (temperatura <= 60.0)) {  /* Filtro de medida coerente */
  temperatura = temperatura +  (0.0);   /* OFFSET por si la medida no es correcta */
  humedad = humedad + (0.0);            /* OFFSET por si la medida no es correcta */
  /*Calculo de la humedad filtrada*/
  if (x < 10) { hume[x] = humedad; temp[x] = temperatura; x++; } 
  if (x >= 10) { x=0; }

  humedad_max = 0.0;                 /* Inicializamos variables */
  humedad_min = 100.0;               /* Inicializamos variables */
  temperatura_max = -30.0;           /* Inicializamos variables */
  temperatura_min = 100.0;           /* Inicializamos variables */
  for(int q=0;q<10;q++) {            /* Busqueda del mayor y el menor de la humedad y la temperatura */
    if (hume[q] > humedad_max) {     /* Buscamos el mayor de la humedad */
      humedad_max = hume[q];
      humedad_max_i = q;
    }
    if (hume[q] < humedad_min) {     /* Buscamos el menor de la humedad */
      humedad_min = hume[q];
      humedad_min_i = q;
    }
    if (temp[q] > temperatura_max) { /* Buscamos el mayor de la temperatura */
      temperatura_max = temp[q];
      temperatura_max_i = q;
    }
    if (temp[q] < temperatura_min) { /* Buscamos el menor de la temperatura */
      temperatura_min = temp[q];
      temperatura_min_i = q;
    }
  }
  humedad_suma = 0.0;               
  temperatura_suma = 0.0;
  for(int q=0;q<10;q++) { 
    if (humedad_max_i == humedad_min_i) {if (humedad_max_i < 8) {humedad_max_i++;} else {humedad_max_i--;}}   /* Si se produce el caso singular de encontrar todos los valores iguales, se establece un maximo y un mínimo forzado */
    if ((q != humedad_max_i) && (q != humedad_min_i)) { /* Discriminamos el mayor y el menor */
      humedad_suma = humedad_suma + hume[q];            /* Sumamos los 8 valores validos */
    }
    if (temperatura_max_i == temperatura_min_i) {if (temperatura_max_i < 8) {temperatura_max_i++;} else {temperatura_max_i--;} }  /* Si se produce el caso singular de encontrar todos los valores iguales, se establece un maximo y un mínimo forzado */
    if ((q != temperatura_max_i)&&(q != temperatura_min_i)) {  /* Discriminamos el mayor y el menor */
      temperatura_suma = temperatura_suma + temp[q];           /* Sumamos los 8 valores validos */
    }
  }
  humedad_filtrada2 = humedad_suma/8;                        /* Realizamos la media */                                                                 /* Salida de la función */
  temperatura_filtrada2 = temperatura_suma/8;                /* Realizamos la media */                                                                 /* Salida de la función */
  }
}

