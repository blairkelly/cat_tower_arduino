//Cat Tower, Arduino Microcontroller


//pins
//sensorpins
int pinInDoor = A0;     // cat door going in
int pinOutDoor = A1;    // cat door going out
int pinMainDoor = A2;   // maintainance door
int pinWalle = A3;      // Wall-E sensor (cat poo sensor)
int pinFoodDoor = A5;   // food and supplies door
//digitalpins
int pinKittyLight = 7;  //main LED light for the cats in the entryway
int pinXbeePwr = 8;     //XBEE power switch
int pinLed = 13;

//vars
unsigned long lightofftime = millis();
int lightondelay = 5000;



void setup() {
  Serial.begin(57600);
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
  digitalWrite(pinKittyLight, LOW);
  digitalWrite(pinXbeePwr, LOW);
  digitalWrite(pinLed, LOW);
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

void loop() {
  //read in door
  if(analogRead(pinInDoor) > 200) {
    digitalWrite(pinKittyLight, HIGH);
    lightofftime = millis() + lightondelay;
  }
  if(millis() > lightofftime) {
    digitalWrite(pinKittyLight, LOW);
  }

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

  delay(666);
}