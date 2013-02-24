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
boolean xbeeis = ON;
unsigned long xbeepwronat = millis();
int xbeepwrondelay = 1000; //length of time to wait for xbee to power up before writing data.
String databuffer = "";      //data queued to be sent through xbee once it's ready.
unsigned long xbeeondelay = 45000;   //minimum length of time to keep xbee on.
unsigned long xbeeusedat = millis();   //when was the last time the xbee was used?
unsigned long lightondelay = 180000;   //minimum duration for which the light is kept on.
unsigned long lightwentonat = millis();   //what time did the light go on at?
int dbd_catdoor = 100;   //amount of time it takes to consider the cat door "opened"
unsigned long dbd_cat_in = millis();  //cat in-door debounce time
unsigned long dbd_cat_out = millis();  //cat out-door debounce time
unsigned long dbd_maindoor = millis();  //maindoor debounce time
unsigned long dbd_fooddoor = millis();  //fooddoor debounce time
unsigned long dbd_walle = millis();  //walle debounce time
unsigned long wallelastsaw = millis();
int wewblatd = 8000;  //Wall-E Will Be Lonely After This Delay
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
  delay(xbeepwrondelay);
  Serial.begin(57600);
  delay(xbeepwrondelay);
  Serial.println("R1");
  delay(xbeepwrondelay);
  digitalWrite(pinXbeePwr, OFF);
  xbeeis = OFF;
}

void checkbee() {
  if(!digitalRead(pinXbeePwr)) {
    //xbee is off. Turn it on.
    digitalWrite(pinXbeePwr, ON);
    xbeeis = ON;
    xbeepwronat = millis();  //record the time at which the xbee was powered on.
    xbeeusedat = millis();
  }
}
void pfwrite() {
  if(!databuffer.equals("")) {
    if(!xbeeis) {
      checkbee();
    } else if(((millis() - xbeepwronat) > xbeepwrondelay) && xbeeis) {
      //it's ok to write to the xbee
      Serial.println(databuffer);
      xbeeusedat = millis();
      databuffer = "";
    }
  }
}
void pfs(String pthis) {
  databuffer = databuffer + pthis;
}
void pfi(int pthis) {
  databuffer = databuffer + String(pthis);
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
    delay(111);
  }
}

void kittylight(boolean state) {
  if(state && !lightsout && !digitalRead(pinKittyLight)) {
    digitalWrite(pinKittyLight, ON);
    lightis = ON;
    delay(77); //delay in order to avoid bad sensor readings due to power fluctuation.
    lightwentonat = millis();
    pfs("l");
    pfi(1);
  } else if (!state && digitalRead(pinKittyLight)) {
    digitalWrite(pinKittyLight, OFF);
    lightis = OFF;
    pfs("l");
    pfi(0);
  }
}

void checkpower() {
  //checks light and xbee power.
  if(lightis) {
    if(((millis() - lightwentonat) > lightondelay) && !maindoor) {
      kittylight(OFF);
    }
  }
  if(xbeeis) {
    if((millis() - xbeeusedat) > xbeeondelay) {
      digitalWrite(pinXbeePwr, OFF);
      xbeeis = OFF;
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
        pfs("c");
        pfi(catsinside);
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
        pfs("c");
        pfi(catsinside);
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
      //maintainance door appears to be open
      if(!dbd_maindoor_d) {
        dbd_maindoor = millis() + dbd_catdoor;
        dbd_maindoor_d = true;
      } else if (dbd_maindoor_d) {
        if((millis() > dbd_maindoor) && !maindoor) {
          closewalle();
          dbd_maindoor_d = false;
          maindoor = OPEN;
          kittylight(ON);
          pfs("m");
          pfi(1);
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
          pfs("m");
          pfi(0);
        }
      }
    }
  }

  //read food door
  if(analogRead(pinFoodDoor) > 50) {
    //food and supplies door appears to be open.
    if(!dbd_fooddoor_d && !fooddoor) {
      dbd_fooddoor = millis() + 500;
      dbd_fooddoor_d = true;
    } else if (dbd_fooddoor_d) {
      if((millis() > dbd_fooddoor) && !fooddoor) {
        dbd_fooddoor_d = false;
        fooddoor = OPEN;
        pfs("f");
        pfi(1);
      }
    }
  } else {
    if(fooddoor) {
      if(!dbd_fooddoor_d) {
        dbd_fooddoor_d = true;
        dbd_fooddoor = millis() + 500;
      } else if (millis() > dbd_fooddoor) {
        dbd_fooddoor_d = false;
        fooddoor = CLOSED;
        pfs("f");
        pfi(0);
      }
    }
  }
}

void closewalle() {
  dbd_walle_d = false;
  walle = CLOSED;
  //kittylight(OFF);
  pfs("W");
  pfi(0);
}
void checkwalle() {
  if(!maindoor || ignoremain) {
    int walleHigh = 311;
    int walleMid = 190;
    //read Wall-E
    if(analogRead(pinWalle) > walleHigh) {
      //it appears there's a cat in front of Wall-E
      wallelastsaw = millis(); //the last time Wall-E saw something.
      if(!dbd_walle_d && !walle) {
        dbd_walle = millis() + 333;
        dbd_walle_d = true;
      } else if (dbd_walle_d) {
        if((millis() > dbd_walle) && !walle) {
          dbd_walle_d = false;
          walle = true;
          kittylight(ON);
          pfs("W");
          pfi(1);
        }
      }
    } else if ((analogRead(pinWalle) > walleMid) && (analogRead(pinWalle) <= walleHigh)) {
      //do nothing. it's a grey zone. The proximity sensor seems to fluctuate quite a bit
      //When something is put in front of it, the value can change even if the item doesn't
      //move - perhaps I have a bad sensor. I'm not sure.
    } else {
      //Wall-E is low.
      //here, we can specify how long it'll take before the Arduino decides
      //a cat is no longer in the litterbox. I am guessing that a cat could
      //be seen to be moving inside a litterbox at least every twelve seconds
      //or so...
      if(walle && ((millis() - wallelastsaw) > wewblatd)) {
        if(!dbd_walle_d) {
          dbd_walle_d = true;
          dbd_walle = millis() + 5000;
        } else if (millis() > dbd_walle) {
          closewalle();
        }
      }
    }
  }
}

void loop() {
  dodoors();
  checkwalle();
  printsensors();
  pfwrite();
  checkpower();
}