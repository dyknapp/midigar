//---------------------------------------------------------------
//   MODULE TEST STRUCTURES
//---------------------------------------------------------------

int seq[4] = {127, 123, 137, 143};

struct toggler {

	bool tog;
	unsigned int interval;
	unsigned long previousMillis;
	int step;

	void autoTog() {

 		if (millis() - previousMillis >= interval) {

   			previousMillis = millis();

   			nodes[seq[step]].intState = 0;
   			
   			step++;
   			step = step % 4;

 		}
    nodes[seq[step]].intState = 1;
	}
};

toggler tog = {0, 400};


//---------------------------------------------------------------
//   CONTROLLER TESTS
//---------------------------------------------------------------

void testButtons() {
  
  for (int i = 0; i < 304; i++) {
    
    if (buttons.risingEdge(i)) {
      
        Serial.print("Key");
        Serial.print("\t");
        Serial.print("index:   ");
        Serial.print(i);
        Serial.print("\t");

        String type;
        if (nodes[i].type == 0)
          type = "non-patch";
        else if (nodes[i].type == 1)
          type = "input";
        else if (nodes[i].type == 2)
          type = "special input";
        else if (nodes[i].type == 3)
          type = "output";

        Serial.print(type);
        Serial.println();
      
    }
  }
}


void testPots() {
  for (int i = 0; i < 26; i++) {
    if (pots.hasChanged(i)) {
      Serial.print("Pot");
      Serial.print("\t");
      Serial.print("index:");
      Serial.print(i);
      Serial.print("\t");
      Serial.println(pots.vals[i]);
      screenPrint(pots.vals[i]);
    }
  }
}


void countIdentify() {

  int counters = 0;

  for (int i = 0; i<buttonsMax; i++) {
      if (nodes[i].type == 1 || nodes[i].type == 2) {
          counters++;
          Serial.print(i);
          Serial.print(", ");
      }
  }
  Serial.println();
  Serial.println(counters);

}


