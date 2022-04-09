//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

int logicInputs[] = {182, 185, 160, 191, 198, 193, 180, 195};
int logicOutputs[] = {177, 187, 186, 170};
int logicSelects[] = {178, 169, 183, 199};
int logicModeSel[] = {0, 1, 2, 3};

String screenLogic[6] = {"AND", "NAND", "OR", "NOR", "XOR", "XNOR"};


//---------------------------------------------------------------
//   FUNCTIONS
//---------------------------------------------------------------

void logicButtons() {
	for (int i = 0; i < 8; i++) {
		int index = logicInputs[i];
		if (buttons.risingEdge(index)) 
			nodes[index].intState = 1;
		else if (buttons.fallingEdge(index)) 
			nodes[index].intState = 0;
	}
}

void logicGateSel() {
	for (int i = 0; i < 4; i++) {
		int index = logicSelects[i];

		if (buttons.vals[index] == 1) {
			nodes[index].intState = 1;	

			if (buttons.lastVals[index] == 0) {
				screenPrint(screenLogic[logicModeSel[i]]);
				screen.writeDisplay();
			}
			if (pots.hasChanged(23)) {		
				int tempVal = pots.vals[23];
				tempVal = map(tempVal, 0, 150, 0, 5);
				tempVal = constrain(tempVal, 0, 5);
				logicModeSel[i] = tempVal;
				screenPrint(screenLogic[logicModeSel[i]]);
				screen.writeDisplay();
			}
		}
		else if (buttons.vals[index] == 0) {
			nodes[index].intState = 0;		
		}
	}
}


void logicUpdate() {
	for (int i = 0; i < 4; i++) {
		int inA = logicInputs[i*2];
		int inB = logicInputs[(i*2)+1];
		int out = logicOutputs[i];

		if (nodes[inA].extHasChanged() || nodes[inB].extHasChanged()) {
			switch(logicModeSel[i]) {
				case 0: {	// and	
					if (nodes[inA].extState == 1 && nodes[inB].extState == 1)
						nodes[out].intState = 1;
					else 
						nodes[out].intState = 0;
					break;
				}
				case 1: {	// nand
					if (nodes[inA].extState == 1 && nodes[inB].extState == 1)
						nodes[out].intState = 0;
					else 
						nodes[out].intState = 1;
					break;
				}
				case 2: {	// or
					if (nodes[inA].extState == 1 || nodes[inB].extState == 1)
						nodes[out].intState = 1;
					else 
						nodes[out].intState = 0;
					break;
				}
				case 3: {	// nor
					if (nodes[inA].extState == 1 || nodes[inB].extState == 1)
						nodes[out].intState = 0;
					else 
						nodes[out].intState = 1;
					break;
				}
				case 4: {	// xor
					if (nodes[inA].extState != nodes[inB].extState)
						nodes[out].intState = 1;
					else 
						nodes[out].intState = 0;
					break;
				}
				case 5: {	// xnor
					if (nodes[inA].extState == nodes[inB].extState)
						nodes[out].intState = 1;
					else 
						nodes[out].intState = 0;
					break;
				}
			}
		}
	}
}


