//Cat Tower, Arduino Microcontroller

//quick options
boolean ps = false;    //print sensors?
boolean lightsout = false; //keep the light off for testing purposes?
boolean ignoremain = false; //ignore the main door switch?

//pins
//sensorpins
int pinInDoor = A0;     // cat door going in
int pinOutDoor = A1;    // cat door going out
int pinMainDoor = A2;   // maintainance door
int pinWalle = A3;      // Wall-E sensor (cat litterbox sensor)
int pinFoodDoor = A5;   // food and supplies door
//digitalpins
int pinKittyLight = 7;  //main LED light for the cats in the entryway
int pinXbeePwr = 8;     //XBEE power switch
int pinLed = 13;

//vars
boolean ON = 1;
boolean OFF = 0;
boolean OPEN = 1;
boolean CLOSED = 0;
boolean lightis = OFF;
unsigned long lightondelay = 120000;   //minimum duration for which the light is kept on.
unsigned long lightwentonat = millis();   //what time did the light go on at?
unsigned long checkindelay = 60000;   //check-in interval
unsigned long checkintime = millis() + checkindelay;
int dbd_catdoor = 100;   //amount of time it takes to consider the cat door "opened"
unsigned long dbd_cat_in = millis();  //cat in-door debounce time
unsigned long dbd_cat_out = millis();  //cat out-door debounce time
unsigned long dbd_maindoor = millis();  //maindoor debounce time
unsigned long dbd_fooddoor = millis();  //fooddoor debounce time
unsigned long dbd_walle = millis();  //walle debounce time
boolean dbd_cat_in_d = false; //cat in door debouncing state.
boolean dbd_cat_out_d = false; //cat out door debouncing state.
boolean dbd_maindoor_d = false; //maindoor debouncing state.
boolean dbd_fooddoor_d = false; //fooddoor debouncing state.
boolean dbd_walle_d = false; //walle debouncing state.
boolean catindoor = CLOSED;
boolean catoutdoor = CLOSED;
boolean maindoor = CLOSED;
boolean fooddoor = CLOSED;
boolean walle = false;
int catsinside = 0;   //how many cats are inside?

void setup() {
  // set pins
  pinMode(pinInDoor, INPUT);
  pinMode(pinOutDoor, INPUT); 
  pinMode(pinMainDoor, INPUT); 
  pinMode(pinWalle, INPUT); 
  pinMode(pinFoodDoor, INPUT); 
  pinMode(pinKittyLight, OUTPUT); 
  pinMode(pinXbeePwr, OUTPUT); 
  pinMode(pinLed, OUTPUT);
  // set initial digital pin states
  digitalWrite(pinKittyLight, OFF);
  digitalWrite(pinXbeePwr, ON);
  digitalWrite(pinLed, OFF);
  //start serial
  Serial.begin(57600);
  delay(500);
  Serial.println("Ready");
  delay(500);
  digitalWrite(pinXbeePwr, OFF);
}

void pfs(String pthis) {
  Serial.print(pthis);
}
void pfi(int pthis) {
  Serial.print(pthis);
}
void pfl() {
  Serial.println(" ");
}
void printsensors() {
  if(ps) {
    pfs("pinInDoor: ");
    pfi(analogRead(pinInDoor));
    pfs(", pinOutDoor");
    pfi(analogRead(pinOutDoor));
    pfs(", pinMainDoor");
    pfi(analogRead(pinMainDoor));
    pfs(", pinWalle");
    pfi(analogRead(pinWalle));
    pfs(", pinFoodDoor");
    pfi(analogRead(pinFoodDoor));
    pfl();
    delay(111);
  }
}

void checkin() {
  // pfs("catsinside = ");
  // pfi(catsinside);
  // pfl();
}

void kittylight(boolean state) {
  if(state && !lightsout && !digitalRead(pinKittyLight)) {
    digitalWrite(pinKittyLight, ON);
    lightis = ON;
    delay(77); //delay in order to avoid bad sensor readings due to power fluctuation.
    lightwentonat = millis();
  } else if (!state && digitalRead(pinKittyLight)) {
    digitalWrite(pinKittyLight, OFF);
    lightis = OFF;
  }
}

void checklight() {
  if(lightis) {
    if(((millis() - lightwentonat) > lightondelay) && !maindoor) {
      kittylight(OFF);
    }
  }
}

void dodoors() {
  //read in door
  if((analogRead(pinInDoor) > 350) && !catoutdoor) {
    //cat door appears to be open
    if(!dbd_cat_in_d && !catindoor) {
      dbd_cat_in = millis() + dbd_catdoor;
      dbd_cat_in_d = true;
    } else if (dbd_cat_in_d) {
      if((millis() > dbd_cat_in) && !catindoor) {
        dbd_cat_in_d = false;
        catindoor = OPEN;
        catsinside++;
        kittylight(ON);
      }
    }
  } else {
    if(catindoor) {
      if(!dbd_cat_in_d) {
        dbd_cat_in_d = true;
        dbd_cat_in = millis() + (dbd_catdoor * 6);
      } else if (millis() > dbd_cat_in) {
        dbd_cat_in_d = false;
        catindoor = CLOSED;
      }
    }
  }

  //read out door
  if((analogRead(pinOutDoor) > 450) && !catindoor) {
    //cat door appears to be open
    if(!dbd_cat_out_d && !catoutdoor) {
      dbd_cat_out = millis() + dbd_catdoor;
      dbd_cat_out_d = true;
    } else if (dbd_cat_out_d) {
      if((millis() > dbd_cat_out) && !catoutdoor) {
        dbd_cat_out_d = false;
        catoutdoor = OPEN;
        catsinside--;
      }
    }
  } else {
    if(catoutdoor) {
      if(!dbd_cat_out_d) {
        dbd_cat_out_d = true;
        dbd_cat_out = millis() + (dbd_catdoor * 3);
      } else if (millis() > dbd_cat_out) {
        dbd_cat_out_d = false;
        catoutdoor = CLOSED;
      }
    }
  }

  //read maintenance door
  if(!ignoremain) {
    if(analogRead(pinMainDoor) > 490) {
      //cat door appears to be open
      if(!dbd_maindoor_d) {
        dbd_maindoor = millis() + dbd_catdoor;
        dbd_maindoor_d = true;
      } else if (dbd_maindoor_d) {
        if(millis() > dbd_maindoor) {
          dbd_maindoor_d = false;
          maindoor = OPEN;
          kittylight(ON);
        }
      }
    } else {
      if(maindoor) {
        if(!dbd_maindoor_d) {
          dbd_maindoor_d = true;
          dbd_maindoor = millis() + (dbd_catdoor * 10);
        } else if (millis() > dbd_maindoor) {
          dbd_maindoor_d = false;
          maindoor = CLOSED;
          kittylight(OFF);
        }
      }
    }
  }

  //read food door
  if(analogRead(pinFoodDoor) > 120) {
    //cat door appears to be open
    if(!dbd_fooddoor_d && !fooddoor) {
      dbd_fooddoor = millis() + dbd_catdoor;
      dbd_fooddoor_d = true;
    } else if (dbd_fooddoor_d) {
      if((millis() > dbd_fooddoor) && !fooddoor) {
        dbd_fooddoor_d = false;
        fooddoor = OPEN;
        pfs("FoodDoor OPEN");
        pfl();
      }
    }
  } else {
    if(fooddoor) {
      if(!dbd_fooddoor_d) {
        dbd_fooddoor_d = true;
        dbd_fooddoor = millis() + dbd_catdoor;
      } else if (millis() > dbd_fooddoor) {
        dbd_fooddoor_d = false;
        fooddoor = CLOSED;
        pfs("FoodDoor CLOSED");
        pfl();
      }
    }
  }
}

void checkwalle() {
  //read Wall-E
  if(analogRead(pinWalle) > 333) {
    //it appears there's a cat in front of Wall-E
    if(!dbd_walle_d && !walle) {
      dbd_walle = millis() + 333;
      dbd_walle_d = true;
    } else if (dbd_walle_d) {
      if((millis() > dbd_walle) && !walle) {
        dbd_walle_d = false;
        walle = true;
        kittylight(ON);
        pfs("Wall-E sees a kitty.");
        pfl();
      }
    }
  } else {
    if(walle) {
      if(!dbd_walle_d) {
        dbd_walle_d = true;
        dbd_walle = millis() + 3333;
      } else if (millis() > dbd_walle) {
        dbd_walle_d = false;
        walle = CLOSED;
        //kittylight(OFF);
        pfs("Wall-E can't see a kitty.");
        pfl();
      }
    }
  }
}

void loop() {
  dodoors();
  checklight();
  checkwalle();
  printsensors();
}