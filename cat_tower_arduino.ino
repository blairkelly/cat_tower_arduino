//Cat Tower, Arduino Microcontroller

//quick options
boolean ps = false;    //print sensors?
boolean lightsout = false; //keep the light off for testing purposes?

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
unsigned long lightondelay = 120000;   //minimum duration for which the light is kept on.
unsigned long lightwentonat = millis();   //what time did the light go on at?
unsigned long checkindelay = 60000;   //check-in interval
unsigned long checkintime = millis() + checkindelay;
int dbd_catdoor = 100;   //amount of time it takes to consider the cat door "opened"
unsigned long dbd_cat_in = millis();  //cat in-door debounce time
unsigned long dbd_cat_out = millis();  //cat out-door debounce time
boolean dbd_cat_in_d = false; //cat in door debouncing state.
boolean dbd_cat_out_d = false; //cat out door debouncing state.
boolean catindoor = CLOSED;
boolean catoutdoor = CLOSED;
boolean maindoor = CLOSED;
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
    delay(500);
  }
}

void checkin() {
  pfs("catsinside = ");
  pfi(catsinside);
  pfl();
}

void kittylight(boolean state) {
  if(state && !lightsout && !digitalRead(pinKittyLight)) {
    digitalWrite(pinKittyLight, ON);
    delay(110); //delay in order to avoid bad sensor readings due to power fluctuation.
    lightwentonat = millis();
  } else if (!state && digitalRead(pinKittyLight)) {
    digitalWrite(pinKittyLight, OFF);
  }
}

void checklight() {
  if(((millis() - lightwentonat) > lightondelay) && !maindoor) {
    kittylight(OFF);
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
        checkin();
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
        checkin();
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
}

void loop() {
  dodoors();
  checklight();
  printsensors();
}