//---------------------------------------------------------------
//   MAIN
//---------------------------------------------------------------

void updateLEDs() {

    ledsStates();

    ledOctave(piano.octave);
    ledAdjusters(piano.leftAdjNodes, 3);
	ledAdjusters(piano.rightAdjNodes, 3);
    ledsKeys(piano.keys, piano.ledState, 25);

    ledOctave(strings.octave);
    ledAdjusters(strings.leftAdjNodes, 2);
   	ledAdjusters(strings.rightAdjNodes, 3);
    ledsKeys(strings.keys, strings.ledState, 14);

	ledsAllLinks(); 
    ledsActiveNodeLinks();

    ledsModifiedNodes();

    leds.write();

}


//---------------------------------------------------------------
//   GENERAL
//---------------------------------------------------------------

void ledsClear() {
    leds.fillScreen(0);
    leds.drawPixel(nodes[userModeNode].ledx, nodes[userModeNode].ledy, nodes[userModeNode].extState);
    leds.drawPixel(nodes[comCtrlModeNode].ledx, nodes[comCtrlModeNode].ledy, nodes[comCtrlModeNode].extState);  
    leds.drawPixel(nodes[modifierModeNode].ledx, nodes[modifierModeNode].ledy, nodes[modifierModeNode].extState); 
}


void  ledsStates() {
  for (int i = 0; i < buttonsMax; i++) 
        leds.drawPixel(nodes[i].ledx, nodes[i].ledy, nodes[i].extState);
}


void ledsKeys(int nodeBank[], bool ledBank[], int size) {
	for (int i = 0; i < size; i++) {		
		leds.drawPixel(nodes[nodeBank[i]].ledx, nodes[nodeBank[i]].ledy, ledBank[i]);	
	}
}


//---------------------------------------------------------------
//   LINKS
//---------------------------------------------------------------

void ledsAllLinks() {
	if (userMode == 0)  {
	    ledsClear();
	    ledLogic();

	    byte x;
	    byte y;

	    for (int i = 0; i < linkCount; i++) {
	        x = nodes[links[i].receiver].ledx;
	        y = nodes[links[i].receiver].ledy;
	        leds.drawPixel(x, y, 1);

	        x = nodes[links[i].sender].ledx;
	        y = nodes[links[i].sender].ledy;
	        leds.drawPixel(x, y, 1);
	    }
	}
}


void ledsActiveNodeLinks() {
    if (userMode == 1 || userMode == 0) {
    	if (activeNode != 999) {
    		ledsClear(); 
		    byte x = nodes[activeNode].ledx;
		    byte y = nodes[activeNode].ledy;
		    leds.drawPixel(x, y, 1);

		    if (isNodeOutput(activeNode)) {
		        for (int i = 0; i < linkCount; i++) {
		            if (links[i].sender == activeNode) {
		                x = nodes[links[i].receiver].ledx;
		                y = nodes[links[i].receiver].ledy;
		                leds.drawPixel(x, y, 1);
		            }
		        }
		    } 
		    else if (isNodeInput(activeNode)) {
		        for (int i = 0; i < linkCount; i++) {
		            if (links[i].receiver == activeNode) {
		                x = nodes[links[i].sender].ledx;
		                y = nodes[links[i].sender].ledy;
		                leds.drawPixel(x, y, 1);
		            }
		        }
		    }
		}
	}
}


//---------------------------------------------------------------
//   MODIFIERS
//---------------------------------------------------------------

void ledsModifiedNodes() {
	if (modifierMode != 7) {
		ledsClear();

		for (int i = 0; i < 304; i++) {
			if (modifierMode == 0) { 			// mode
				counterBlue.displayMode();
				counterRed.displayMode();
				counterGreen.displayMode();
				metroplexer.displayModeAux();
			}
			else if (modifierMode == 1) {		// aux
				counterBlue.displayAux();
				counterRed.displayAux();
				counterGreen.displayAux();
				outplexDisplayAux();
				inplexDisplayAux();
				metroplexer.displayModeAux();
			}
			else if (modifierMode == 2) {		// perm
				counterBlue.displayPerm();
				counterRed.displayPerm();
				counterGreen.displayPerm();
				outplexDisplayPerm();
				inplexDisplayPerm();
			}
			else if (modifierMode == 3) {		// mute
				if (nodes[i].mute) 
					leds.drawPixel(nodes[i].ledx, nodes[i].ledy, 1);
			}
			else if (modifierMode == 4) {		// tog
				if (nodes[i].toggle) 
					leds.drawPixel(nodes[i].ledx, nodes[i].ledy, 1);
			}
			else if (modifierMode == 5) {		// invert
				if (nodes[i].invert) 
					leds.drawPixel(nodes[i].ledx, nodes[i].ledy, 1);
			}
		}
	}
}


//---------------------------------------------------------------
//   MODIFIERS
//---------------------------------------------------------------

void ledOctave(int octave) {
	if (buttons.vals[octave])
		leds.drawPixel(nodes[octave].ledx, nodes[octave].ledy, 1);
}

void ledAdjusters(int bank[], int size) {
	for (int i = 0; i < size; i++) {
		int index = bank[i];
		if (buttons.vals[index] == 1) {
			leds.drawPixel(nodes[index].ledx, nodes[index].ledy, 1);		
		}
	}
}

void ledLogic() {
	for (int i = 0; i < 4; i++) {
		int index = logicSelects[i];
		if (buttons.vals[index] == 1) {
			leds.drawPixel(nodes[index].ledx, nodes[index].ledy, 1);
		}
	}
}