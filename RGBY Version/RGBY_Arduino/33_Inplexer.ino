//---------------------------------------------------------------
//   INPLEX STRUCT
//---------------------------------------------------------------

struct inplex {
	int mode;
	int inputs[4];
	int selBit1;
	int selBit2;
	int hold;
	int pause;
	int clear;
	int output;
	byte inSel;
	byte auxSel;
	byte permSel;

	/* MAIN UPDATE ------------------------------------------- */
	void update() {	
		if (!isPaused()) {
			inputChanged();
			selBitChanged(selBit1, 1);
			selBitChanged(selBit2, 2);
		}
		checkHoldRelease();
		checkPauseRelease();
		checkClear();
	}

	void inputChanged() {
		if (nodes[inputs[plexPerms[permSel][inSel]]].extHasChanged())
			updateOutput();
	}

	void selBitChanged(int bit, int adjust) {
		if (nodes[bit].extHasChanged()) {
			if (isMuxMode()) {
				if (nodes[bit].extState == 1) 
					inSel += adjust;		
				else 
					inSel -= adjust;		
			} else {
				if (nodes[bit].extState == 1) {	
					inSel += adjust;
					seqWrapAround();	
				}
			}
			updateOutput();
		}
	}

	void seqWrapAround() {
		if (inSel > 3) 
			inSel = inSel % 4;	
	}


	/* GENERATING OUTPUT ------------------------------------- */
	void updateOutput() {
		if (!isHeld())
			nodes[output].intState = nodes[inputs[plexPerms[permSel][inSel]]].extState;
		else if (isHeld() && nodes[inputs[plexPerms[permSel][inSel]]].extState == 1)
			nodes[output].intState = 1;	
	}

	bool isHeld() {
		if (nodes[hold].extState) 
			return true;
		 else 
			return false;	
	}

	bool isPaused() {
		if (nodes[pause].extState) 
			return true;
		 else 
			return false;	
	}	

	void checkHoldRelease() {
		if (nodes[hold].extFallingEdge() && !isPaused()) 
			reset();	
	}

	void checkPauseRelease() {
		if (nodes[pause].extFallingEdge() && !isHeld()) 
			reset();		
	}

	void checkClear() {
		if (nodes[clear].extState) 
			nodes[output].extState = 0;	
		if (nodes[clear].extFallingEdge()) 
			nodes[output].intState = nodes[inputs[plexPerms[permSel][inSel]]].extState;
	}	


	/* MANAGEMENT -------------------------------------------- */
	void reset() {
		nodes[output].intState = 0;

		if (isMuxMode()) {
			inSel = 0;
			if (nodes[selBit1].extState == 1) 
				inSel += 1;	
			if (nodes[selBit2].extState == 1) 
				inSel += 2;
		}
		nodes[output].intState = nodes[inputs[plexPerms[permSel][inSel]]].extState;
	}	

	bool isMuxMode() {
		if (nodes[mode].intState == 0)
			return true;
		else 
			return false;
	}

	void updateButtons() {
		for (int i = 0; i < 4; i++) 
			detectPress(&nodes[inputs[i]], inputs[i]);
		detectPress(&nodes[selBit1], selBit1);	
		detectPress(&nodes[selBit2], selBit2);
		detectPress(&nodes[hold], hold);	
		detectPress(&nodes[pause], pause);
		detectPress(&nodes[clear], clear);			
	}	


	/* PERFORMANCE SELECTION --------------------------------- */
	void setMode() {
		if (buttons.risingEdge(mode)) {
			nodes[mode].intState = !nodes[mode].intState;
			reset();
		}
	}	

	void changeAux() {			
		for (int i = 0; i < 4; i++) {
			if (buttons.risingEdge(inputs[i])) {
				auxSel = i;
				reset();
			}
		}
	}

	void changePerm() {			
		for (int i = 0; i < 4; i++) {
			if (buttons.risingEdge(inputs[i])) {
				Serial.println(i);
				permSel = i;
				reset();
			}
		}
	}			

	void displayAux() {
		leds.drawPixel(nodes[inputs[auxSel]].ledx, nodes[inputs[auxSel]].ledy, 1);
	}	

	void displayPerm() {
		leds.drawPixel(nodes[inputs[permSel]].ledx, nodes[inputs[permSel]].ledy, 1);
	}

};


//---------------------------------------------------------------
//   INITIALIZE STRUCTS
//---------------------------------------------------------------
				  
inplex inplexA = {233, {209, 239, 213, 236}, 221, 216, 232, 208, 229, 171};
inplex inplexB = {231, {227, 207, 228, 205}, 220, 230, 204, 212, 206, 179};


//---------------------------------------------------------------
//   FUNCTIONS
//---------------------------------------------------------------

// Run Module in Main.ino ---------------------------------------
void inplexSetMode() {
	inplexA.setMode();
	inplexB.setMode();
}

void inplexButtons() {
	inplexA.updateButtons();
	inplexB.updateButtons();
}

void inplexUpdate() {
	inplexA.update();
	inplexB.update();
}

// Adjust Settings in Modifiers.ino -----------------------------
void inplexChangeAux() {
	inplexA.changeAux();
	inplexB.changeAux();	
}

void inplexChangePerm() {
	inplexA.changePerm();
	inplexB.changePerm();	
}

// Display Current Setting in LEDs.ino ---------------------------
void inplexDisplayAux() {
	inplexA.displayAux();
	inplexB.displayAux();
}

void inplexDisplayPerm() {
	inplexA.displayPerm();
	inplexB.displayPerm();
}

