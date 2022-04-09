//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

byte plexPerms[4][4] {
	{0, 1, 2, 3}, {1, 3, 0, 2}, {2, 0, 3, 1}, {3, 2, 1, 0}
};


//---------------------------------------------------------------
//   OUTPLEX STRUCT
//---------------------------------------------------------------

struct outplex {
	int mode;
	int input;
	int selBit1;
	int selBit2;
	int hold;
	int pause;
	int clear;
	int outputs[4];
	byte outSel;
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
		if (auxSel == 2) 
			nodes[input].extState = 1;
		if (nodes[input].extHasChanged())
			updateOutput();
	}

	void selBitChanged(int bit, int adjust) {
		if (nodes[bit].extHasChanged()) {

			if (!isHeld())
				nodes[outputs[plexPerms[permSel][outSel]]].intState = 0;

			if (isMuxMode()) {
				if (nodes[bit].extState == 1) 
					outSel += adjust;		
				else 
					outSel -= adjust;		
			} else {
				if (nodes[bit].extState == 1) {	
					outSel += adjust;
					seqWrapAround();	
				}
			}
			updateOutput();
		}
	}

	void seqWrapAround() {
		if (outSel > 3) 
			outSel = outSel % 4;	
	}


	/* GENERATING OUTPUT ------------------------------------- */
	void updateOutput() {
		if (!isHeld())
			nodes[outputs[plexPerms[permSel][outSel]]].intState = nodes[input].extState;
		else if (isHeld() && nodes[input].extState == 1)
			nodes[outputs[plexPerms[permSel][outSel]]].intState = 1;	
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
		if (nodes[clear].extRisingEdge())
			for (int i = 0; i < 4; i++) 
				nodes[outputs[i]].extState = 0;
		if (nodes[clear].extState) 
			nodes[outputs[plexPerms[permSel][outSel]]].extState = 0;	
		if (nodes[clear].extFallingEdge()) 
			nodes[outputs[plexPerms[permSel][outSel]]].intState = nodes[input].extState;	
	}	


	/* MANAGEMENT -------------------------------------------- */
	void reset() {
		for (int i = 0; i < 4; i++) {
			nodes[outputs[i]].intState = 0;
		}
		if (isMuxMode()) {
			outSel = 0;
			if (nodes[selBit1].extState == 1) 
				outSel += 1;	
			if (nodes[selBit2].extState == 1) 
				outSel += 2;
		}
		nodes[outputs[plexPerms[permSel][outSel]]].intState = nodes[input].extState;
	}	

	void setMode() {
		if (buttons.risingEdge(mode)) {
			nodes[mode].intState = !nodes[mode].intState;
			reset();
		}
	}	

	bool isMuxMode() {
		if (nodes[mode].intState == 0)
			return true;
		else 
			return false;
	}

	void updateButtons() {
		detectPress(&nodes[input], input);
		detectPress(&nodes[selBit1], selBit1);	
		detectPress(&nodes[selBit2], selBit2);
		detectPress(&nodes[hold], hold);	
		detectPress(&nodes[pause], pause);
		detectPress(&nodes[clear], clear);			
	}	


	/* PERFORMANCE SELECTION --------------------------------- */
	void changeAux() {			
		for (int i = 0; i < 4; i++) {
			if (buttons.risingEdge(outputs[i])) {
				auxSel = i;
				reset();
			}
		}
	}

	void changePerm() {			
		for (int i = 0; i < 4; i++) {
			if (buttons.risingEdge(outputs[i])) {
				permSel = i;
				reset();
			}
		}
	}			

	void displayAux() {
		leds.drawPixel(nodes[outputs[auxSel]].ledx, nodes[outputs[auxSel]].ledy, 1);
	}	

	void displayPerm() {
		leds.drawPixel(nodes[outputs[permSel]].ledx, nodes[outputs[permSel]].ledy, 1);
	}

};


//---------------------------------------------------------------
//   INITIALIZE STRUCTS
//---------------------------------------------------------------

outplex outplexA = {210, 203, 211, 218, 235, 225, 224, {219, 214, 238, 194}};
outplex outplexB = {215, 201, 226, 234, 237, 222, 200, {223, 202, 217, 161}};


//---------------------------------------------------------------
//   FUNCTIONS
//---------------------------------------------------------------

// Run Module in Main.ino ---------------------------------------
void outplexSetMode() {
	outplexA.setMode();
	outplexB.setMode();
}

void outplexButtons() {
	outplexA.updateButtons();
	outplexB.updateButtons();
}

void outplexUpdate() {
	outplexA.update();
	outplexB.update();
}

// Adjust Settings in Modifiers.ino -----------------------------
void outplexChangeAux() {
	outplexA.changeAux();
	outplexB.changeAux();	
}

void outplexChangePerm() {
	outplexA.changePerm();
	outplexB.changePerm();	
}

// Display Current Setting in LEDs.ino ---------------------------
void outplexDisplayAux() {
	outplexA.displayAux();
	outplexB.displayAux();
}

void outplexDisplayPerm() {
	outplexA.displayPerm();
	outplexB.displayPerm();
}

