//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

String screenModifier[7] = {"MODE", " AUX", "PERM", "MUTE", " TOG", "INVT", "    "};


//---------------------------------------------------------------
//   SELECTION FUNCTIONS
//---------------------------------------------------------------

void modifierSelect() {
	int modifierInputs[6] = {290, 281, 291, 283, 282, 289};

  	for (int i = 0; i < 6; i++) {
      	if (buttons.risingEdge(modifierInputs[i])) {
      		nodes[modifierInputs[modifierMode]].intState = 0;
	        nodes[modifierInputs[i]].intState = 1;
	        modifierMode = i;
	     	modifierModeNode = modifierInputs[i];
	        screenPrint(screenModifier[i]);
	    }
	    else if (buttons.fallingEdge(modifierInputs[i]) && modifierMode == i) {
	    	nodes[modifierInputs[modifierMode]].intState = 0;
	    	modifierMode = 7;
	    	modifierModeNode = 999;
	    	screenPrint(screenModifier[6]);
	    }
  	}
}


void modifierButtons() {
	switch (modifierMode) {
		case 0:{
			counterBlue.changeMode();
			counterRed.changeMode();
			counterGreen.changeMode();
			metroplexer.changeModeAux();
			break;
		}
		case 1: {
			counterBlue.changeAux();
			counterRed.changeAux();
			counterGreen.changeAux();
			outplexChangeAux();
			inplexChangeAux();
			metroplexer.changeModeAux();
			break;
		}
		case 2: {
			counterBlue.changePerm();
			counterRed.changePerm();
			counterGreen.changePerm();
			outplexChangePerm();
			inplexChangePerm();
			break;
		}
		case 3: {
			for (int i = 0; i < buttonsMax; i++) 
				if (buttons.risingEdge(i) && nodes[i].type != 0) 
					nodes[i].mute = !nodes[i].mute;
			break;
		}
		case 4: {
			for (int i = 0; i < buttonsMax; i++) 
				if (buttons.risingEdge(i) && nodes[i].type != 0) 
					nodes[i].toggle = !nodes[i].toggle;
			break;
		}
		case 5: {
			for (int i = 0; i < buttonsMax; i++) 
				if (buttons.risingEdge(i) && nodes[i].type != 0) 
					nodes[i].invert = !nodes[i].invert;
			break;
		}	
	}
}


//---------------------------------------------------------------
//   UPDATE FUNCTIONS
//---------------------------------------------------------------

void applyModifierInput() {
	for (int i = 0; i < buttonsMax; i++) 
		if (nodes[i].type == 1 || nodes[i].type == 2) 
			modify(&nodes[i]);
}


void applyModifierOutput() {
	for (int i = 0; i < buttonsMax; i++) 
		if (nodes[i].type == 0 || nodes[i].type == 3) 
			modify(&nodes[i]);		
}




//---------------------------------------------------------------
//   INTERNAL FUNCTIONS
//---------------------------------------------------------------

void modify(struct node *location) {

	if (location->mute) {
		location->extState = 0;
	}
	else if (location->toggle) {
		
		if (location->intRisingEdge()) {

			location->togLock = !location->togLock;

			if (location->togLock) 
				if (location->invert)
					location->extState = 1;
				else
					location->extState = 0;
			else
				if (location->invert)
					location->extState = 0;
				else
					location->extState = 1;
		}
	}
	else if (location->invert && !location->toggle) {
		location->extState = !location->intState;	
	}
	else {
		location->extState = location->intState;
	}
}







