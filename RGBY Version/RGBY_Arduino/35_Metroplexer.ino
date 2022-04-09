//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

	String metroModeTxt[4] = {"PLUS", "BLUS", "BLOK", "SNEK"};
	String metroAuxTxt[4] = {"INVG", "INVT", "REPG", "REPT"};


//---------------------------------------------------------------
//   METROPLEX STRUCT
//---------------------------------------------------------------
	
struct metroplex {
	int inputs[4][4];
	int outputs[4][4];
	int rst;
	int hold;
	int pause;
	int clear; 
	int activeX;
	int activeY;
	byte buffer[4][4];
	byte offsetBuffer[4][4];
	byte transX;
	byte transY;
	byte modeSel;
	byte auxSel;

	/* MAIN UPDATE ------------------------------------------- */

	void update() {	
		loadBuffer();
		checkReset();
		if (!isPaused()) 
			inputChanged();
		checkHoldRelease();
		checkPauseRelease();
		checkClear();
		exportBuffer();
	}

	void inputChanged() {
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {

				if (nodes[inputs[x][y]].extRisingEdge()) {
					if (modeSel == 2)
						if (!isHeld()) 
							if (x == 0) 
								offsetToggle(y, 0);
					if (isReplaceMode()) {
						if (!isHeld()) {

							if (!(activeX == x && activeY == y)) 
								if (!isBlokReplaceX(x))
									clearBuffer();		
								
							if (!isBlokReplaceX(x)) {
								activeX = x;
								activeY = y;	
							}
						}
					}
					if (modeSel == 0) plusToggle(x, y);
					else if (modeSel == 1) bigPlusToggle(x, y);
					else if (modeSel == 2) blokToggle(x, y);
				}

				if (nodes[inputs[x][y]].extFallingEdge()) {
					if (isGateMode()) {
						if (isReplaceMode()) {
							if (!isHeld()) {
								if (activeX == x && activeY == y) {
									if (!isBlokReplaceX(x)) {
										clearBuffer();
									}
								}
							}			
						}
						else {
							if (modeSel == 0) plusToggle(x, y);
							else if (modeSel == 1) bigPlusToggle(x, y);
							else if (modeSel == 2) blokToggle(x, y);
						}
					}
				}

				if (modeSel == 3) {
					if (nodes[inputs[x][y]].extRisingEdge()) {
						if (!isHeld())
							buffer[transX][transY] = 0;
						if (x == 0) {
							if (y == 0) 	 transY = (transY+4 + 1) % 4;
							else if (y == 1) transY = (transY+4 - 1) % 4;
							else if (y == 2) transX = (transX+4 + 1) % 4;
							else if (y == 3) transX = (transX+4 - 1) % 4;	
						}
						buffer[transX][transY] = 1;
					}
				}

			}
		}		
	}


	/* PLUS AND BIG PLUS FUNCTIONS --------------------------- */

	void plusToggle(int x, int y) {
		plusPart(x, y,  0,  0); 	// start node
		plusPart(x, y, -1,  0); 	// left and right
		plusPart(x, y,  1,  0);
		plusPart(x, y,  0, -1); 	// above and below
		plusPart(x, y,  0,  1);
	}

	void bigPlusToggle(int x, int y) {
		plusPart(x, y,  0,  0);		// start node
		plusPart(x, y, -2,  0);		// left and right
		plusPart(x, y, -1,  0);
		plusPart(x, y, 	1,  0);
		plusPart(x, y, 	2,  0);
		plusPart(x, y,  0, -2);		// above and below
		plusPart(x, y,  0, -1);
		plusPart(x, y,  0,  1);
		plusPart(x, y,  0,  2);
		plusPart(x, y, -1, -1);		// diagonals
		plusPart(x, y,  1, -1);
		plusPart(x, y, -1,  1);
		plusPart(x, y,  1,  1);
	}

	void plusPart(int x, int y, int addX, int addY) {
		x = x + addX;
		y = y + addY;

		if ((x >= 0 && x < 4) && (y >= 0 && y < 4)) 
			outputToggle(x, y);		
	}


	/* BLOCK TOGGLE AND OFFSET FUNCTIONS --------------------- */

	void offsetToggle(int direction, bool release) {
		fillOffsetBuffer();
		updateDirection(direction);

		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				int offset;
				if (direction == 0 || direction == 2) offset = (x+3) % 4;
				else offset = (x+1) % 4;

				if (direction == 0) buffer[y][x] = offsetBuffer[y][offset];
				else if (direction == 1) buffer[y][x] = offsetBuffer[y][offset];
				else if (direction == 2) buffer[x][y] = offsetBuffer[offset][y];
				else if (direction == 3) buffer[x][y] = offsetBuffer[offset][y];
			}
		}	
	}

	void updateDirection(int direction) {
		if (direction == 0) transX = (transX + 1) % 4;
		else if (direction == 1) transX = (transX - 1) % 4;
		else if (direction == 2) transY = (transY + 1) % 4;
		else if (direction == 3) transY = (transY - 1) % 4;		
	}	

	void blokToggle(int x, int y) {
		for (int i = 0; i < 4; i++) 
			if (x == 1 && y == i) 
				for (int k = 0; k < 4; k++) 
					outputToggle((k + transY) % 4, (i + transX) % 4);
				
		for (int i = 0; i < 4; i++) 
			if (x == 2 && y == i) 
				for (int k = 0; k < 4; k++)  
					outputToggle((i + transY) % 4, (k + transX) % 4);		

		if (x == 3 && y == 0) {
			outputToggle((3 + transY) % 4, (0 + transX) % 4);		
			outputToggle((2 + transY) % 4, (1 + transX) % 4);		
			outputToggle((1 + transY) % 4, (2 + transX) % 4);		
			outputToggle((0 + transY) % 4, (3 + transX) % 4);		
		}
		if (x == 3 && y == 1) {
			outputToggle((0 + transY) % 4, (0 + transX) % 4);		
			outputToggle((1 + transY) % 4, (1 + transX) % 4);		
			outputToggle((2 + transY) % 4, (2 + transX) % 4);		
			outputToggle((3 + transY) % 4, (3 + transX) % 4);		
		}
		if (x == 3 && y == 2) {
			outputToggle((1 + transY) % 4, (0 + transX) % 4);		
			outputToggle((2 + transY) % 4, (0 + transX) % 4);		
			outputToggle((0 + transY) % 4, (1 + transX) % 4);		
			outputToggle((3 + transY) % 4, (1 + transX) % 4);		
			outputToggle((0 + transY) % 4, (2 + transX) % 4);		
			outputToggle((3 + transY) % 4, (2 + transX) % 4);		
			outputToggle((1 + transY) % 4, (3 + transX) % 4);		
			outputToggle((2 + transY) % 4, (3 + transX) % 4);	
		}	
		if (x == 3 && y == 3) {
			outputToggle((2 + transY) % 4, (1 + transX) % 4);		
			outputToggle((1 + transY) % 4, (1 + transX) % 4);		
			outputToggle((2 + transY) % 4, (2 + transX) % 4);		
			outputToggle((1 + transY) % 4, (2 + transX) % 4);				
		}
	}

	void fillOffsetBuffer() {
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++) 
				offsetBuffer[x][y] = buffer[x][y];			
	}

	void outputToggle(int x, int y) {
		if (!isHeld())
			buffer[x][y] = !buffer[x][y];
		else if (isHeld() && buffer[x][y] == 0)
			buffer[x][y] = !buffer[x][y];		
	}


	/* DETERMINE CURRENT AUX SETTING ------------------------- */

	bool isReplaceMode() {
		if (auxSel == 2 || auxSel == 3)
			return true;
		else
			return false;
	}

	bool isGateMode() {
		if (auxSel == 0 || auxSel == 2) 
			return true;
		else
			return false;
	}

	bool isBlokReplaceX(int x) {
		if (modeSel == 2 && (auxSel == 2 || auxSel == 3) && x == 0)	
			return true;
		else
			return false;	
	}

	/* HOLD, PAUSE, AND CLEAR -------------------------------- */

	void checkReset() {
		if (nodes[rst].extRisingEdge()) {

			if (modeSel == 3) {
				buffer[transX][transY] = 0;		
				buffer[0][0] = 1;
			}

			transX = 0;
			transY = 0;			

			if (modeSel == 0 || modeSel == 1 ||modeSel == 2) {
				clearBuffer();
				resetOutputs();
			}
		}
	}

	void resetOffset() {
		transX = 0;
		transY = 0;
		if (modeSel == 3) 
			nodes[outputs[0][0]].extState = 1;
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
		if (nodes[hold].extFallingEdge() && !isPaused()) {
			clearBuffer();	
			resetOutputs();	
		}
	}

	void checkPauseRelease() {
		if (nodes[pause].extFallingEdge() && !isHeld()) {
			clearBuffer();
			resetOutputs();		
		}
	}

	void checkClear() {
		if (nodes[clear].extState)
			clearBuffer();
		if (nodes[clear].extFallingEdge())
			resetOutputs();
	}	


	/* MANAGEMENT -------------------------------------------- */

	void loadBuffer() {
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++) 
				buffer[x][y] = nodes[outputs[x][y]].extState;		
	}

	void exportBuffer() {
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++) 
				nodes[outputs[x][y]].intState = buffer[x][y]; 			
	}

	void clearBuffer() {
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++) 
				buffer[x][y] = 0;
	}	

	void clearOutputs() {
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				nodes[outputs[x][y]].intState = 0;
				nodes[outputs[x][y]].extState = 0;	
			}		
		}
	}

	void resetOutputs() {
		activeX = 999;
		activeY = 999;

		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				if (buttons.vals[inputs[x][y]] == 1 || nodes[inputs[x][y]].extState == 1) {
					if (isReplaceMode()) {
						if (!isBlokReplaceX(x)) {
							activeX = x;
							activeY = y;
						}
						if (modeSel == 0) plusToggle(x, y);
						else if (modeSel == 1) bigPlusToggle(x, y);
						else if (modeSel == 2) blokToggle(x, y);
						return;
					} else {
						if (modeSel == 0) plusToggle(x, y);
						else if (modeSel == 1) bigPlusToggle(x, y);
						else if (modeSel == 2) blokToggle(x, y);
					}			
				}
			}
		}	
	}	

	void updateButtons() {
		for (int x = 0; x < 4; x++) 
			for (int y = 0; y < 4; y++) 
				detectPress(&nodes[inputs[x][y]], inputs[x][y]);
		detectPress(&nodes[rst], rst);
		detectPress(&nodes[hold], hold);	
		detectPress(&nodes[pause], pause);
		detectPress(&nodes[clear], clear);			
	}		


	/* PERFORMANCE SELECTION --------------------------------- */
	void changeModeAux() {			
		for (int i = 0; i < 4; i++) {
			if (buttons.risingEdge(outputs[0][i])) {
				modeSel = i;
				screenPrint(metroModeTxt[modeSel]);
				clearOutputs();
				resetOutputs();
				resetOffset();
			}
			if (buttons.risingEdge(outputs[2][i])) {
				auxSel = i;
				screenPrint(metroAuxTxt[auxSel]);
				clearOutputs();
				resetOutputs();
				resetOffset();
			}
		}
	}		

	void displayModeAux() {
		leds.drawPixel(nodes[outputs[0][modeSel]].ledx, nodes[outputs[0][modeSel]].ledy, 1);
		leds.drawPixel(nodes[outputs[2][auxSel]].ledx, nodes[outputs[2][auxSel]].ledy, 1);
	}	

};


//---------------------------------------------------------------
//   INITIALIZE STRUCT
//---------------------------------------------------------------

metroplex metroplexer = {
	{{264, 269, 278, 270}, {261, 268, 252, 246}, {  0,  29,  37,  24}, { 38,  36,  30,  21}},
	{{249, 251, 242, 274}, {  8,  13,  28,  12}, {  4,  14,  20,  22}, { 32,   6,  16,   5}},
	250, 255, 241, 247, 999, 999
};
