//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

struct commandPiano {
    int keys[25];

    int octave;
    int octaveVal;

    int leftSelNode;
    int leftAdjNodes[3];
    int leftAdjVel[3];
    int leftAdjPitch[3];

    int rightSelNode;
    int rightAdjNodes[3];
    int rightAdjVel[3];
    int rightAdjPitch[3];

    int hold;
    int pause;
    int clear;

    byte keyNote[25];
    byte sigNote[25];
    unsigned long noteStartTime[25];
    bool sustain[25];
    bool ledState[25];
    byte flicker[25];
    byte waitOnAdj[25];


    /* SET ADJUSTERS FUNCTIONS --------------------------------------------------------------- */

    void adjusters() {
    	octaveUpdate();
    	adjUpdate(leftAdjNodes, leftAdjPitch, leftAdjVel, leftSelNode);
    	adjUpdate(rightAdjNodes, rightAdjPitch, rightAdjVel, rightSelNode);			
    }

    void octaveUpdate() {
    	if (buttons.vals[octave] == 1) {
    		if (buttons.lastVals[octave] == 0) 
    			commandScreenPrint(octaveVal);
    		if (pots.hasChanged(23)) 
    			octaveVal = commandSetValue(-3, 3, 12);
    	}
    }

    void adjUpdate(int adjBank[], int pitchSet[], int velSet[], int selNode) {
    	if (buttons.risingEdge(selNode)) 
    		nodes[selNode].intState = !nodes[selNode].intState; 

    	for (int i = 0; i < 3; i++) {
    		int index = adjBank[i];
    		if (buttons.vals[index] == 1) {
    			if (buttons.lastVals[index] == 0) {
    				if (isPitchMode(selNode))
    					commandScreenPrint(pitchSet[i]);
    				else
    					commandScreenPrint(velSet[i]);
    			}	
    			if (pots.hasChanged(23)) {		
    				if (isPitchMode(selNode)) 
    					pitchSet[i] = commandSetValue(-12, 12, 1);
    				else 
    					velSet[i] = commandSetValue(-127, 127, 1); 
    			}
    		}
    	}
    }

    bool isPitchMode(int selNode) {
    	if (nodes[selNode].intState == 0) return true;
    	else return false;
    }    

    int commandSetValue(int low, int high, int scale) {
     	int output = pots.vals[23];
        output = map(output, 0, 150, low, high);
        output = constrain(output, low, high);
        output = output * scale;
     	commandScreenPrint(output);
     	return output;
    } 

    void commandScreenPrint(int input) {
    	screenPrint(abs(input));
    	if (input > 0)
    		screen.writeDigitAscii(0, '+');
    	else if (input < 0)
    		screen.writeDigitAscii(0, '-');
    	screen.writeDisplay();
    }

    /* MAIN READING BUTTONS FUNCTION --------------------------------------------------------- */

    void detectNotes() {
    	checkClear();
    	if (!isPaused() && !isCleared()) {
    		if (userMode == 2) 
        		detectKeyNotes();     
    		detectSignalNotes();
    	}
    	checkHoldRelease();
    	checkPauseRelease();
	}

	/* PHYSICAL KEY PRESS FUNCTION ----------------------------------------------------------- */

	void detectKeyNotes() {  
	    for (int i = 0; i < 25; i++) {
			if (buttons.risingEdge(keys[i])) {	
				sendNote(i, velAdjust[0], pitchAdjust[0], 0);
			}
		    else if (buttons.fallingEdge(keys[i]) && !isHeld()) {
				if (keyNote[i] != 0) {
					usbMIDI.sendNoteOff(keyNote[i], 0, 1);	 
					keyNote[i] = 0;
				}   
		    }
	    }
	}


	/* SIGNAL TRIGGERED NOTE FUNCTIONS ------------------------------------------------------- */

	void detectSignalNotes() {
		for (int i = 0; i < 25; i++) {
			updateFlicker(i);

			if (updateWaitOnAdj(i)) {
				checkSustain(i, lengthAdjust[0]);
				sendNote(i, velAdjust[0], pitchAdjust[0], 1);
				checkFlicker(i);
			}

			if (nodes[keys[i]].extRisingEdge()) 
				checkWaitOnAdj(i);
			
			else if (nodes[keys[i]].extFallingEdge() && sustain[i] == 0 && !isHeld()) {
				if (sigNote[i] != 0) {
					usbMIDI.sendNoteOff(sigNote[i], 0, 1);	
					sigNote[i] = 0;
					resetNote(i);
				}
			}
			updateSustain(i, lengthAdjust[0]);
		}
	}


	/* MAIN NOTE GENERATION FUNCTION --------------------------------------------------------- */
	
	void sendNote(int index, int vel, int pitch, bool type) { 
		for (int i = 0; i < 3; i++) {														
			if (nodes[leftAdjNodes[i]].extState == 1) { 		
				if (isPitchMode(leftSelNode)) pitch += leftAdjPitch[i]; 
				else vel += leftAdjVel[i]; 
			}
			if (nodes[rightAdjNodes[i]].extState == 1) {
				if (isPitchMode(rightSelNode)) pitch += rightAdjPitch[i]; 
				else vel += rightAdjVel[i]; 
			} 
		}

		int outNote = 48 + index + pitch + octaveVal;
		outNote = constrain(outNote, 21, 108);
		int outVel = vel;	
		outVel = constrain(vel, 1, 127);

		if (type == 0) {
			if (keyNote[index] != outNote) {
				keyNote[index] = outNote;
				usbMIDI.sendNoteOn(outNote, outVel, 1);
			}
		}	
		else {
			if (sigNote[index] != outNote) {
				sigNote[index] = outNote;   
				usbMIDI.sendNoteOn(outNote, outVel, 1);
			}
		}   
	}	


	/* HOLD, PAUSE, AND CLEAR ---------------------------------------------------------------- */

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

	bool isCleared() {
		if (nodes[clear].extState) 
			return true;
		 else 
			return false;			
	}

	void checkHoldRelease() {
		if (nodes[hold].extFallingEdge() && !isPaused()) 
			recheckNotes();
	}

	void checkPauseRelease() {
		if (nodes[pause].extFallingEdge() && !isHeld()) 
			recheckNotes();
	}

	void checkClear() {
		if (nodes[clear].extRisingEdge())
			for (int i = 0; i < 25; i++) {
				if (keyNote[i] != 0) {
					usbMIDI.sendNoteOff(keyNote[i], 0, 1);	 
					keyNote[i] = 0;
				}
				if (sigNote[i] != 0) {
					usbMIDI.sendNoteOff(sigNote[i], 0, 1);	 
					sigNote[i] = 0;   
					resetNote(i);
				}			
			}
		
		if (nodes[clear].extFallingEdge()) 
			recheckNotes();	
	}

	void resetNote(int index) {
		noteStartTime[index] = 0;
		sustain[index] = 0;
		ledState[index] = 0;
		flicker[index] = 0;
		waitOnAdj[index] = 0;		
	}	

	void recheckNotes() {
	    for (int i = 0; i < 25; i++) {
			if (buttons.vals[i] == 1) {
				sendNote(i, velAdjust[0], pitchAdjust[0], 0);
			}
			else {
				if (keyNote[i] != 0) {
					usbMIDI.sendNoteOff(keyNote[i], 0, 1);	 
					keyNote[i] = 0;
				}   
			}

			if (nodes[keys[i]].extState == 1) {
				if (nodes[keys[i]].extState == 1) {
					if (isNoteDifferent(i, pitchAdjust[0], sigNote[i])) {
						usbMIDI.sendNoteOff(sigNote[i], 0, 1);	
						resetNote(i);
						checkWaitOnAdj(i);
					}
				}
			}
			else {
				if (sigNote[i] != 0) {
					usbMIDI.sendNoteOff(sigNote[i], 0, 1);	
					sigNote[i] = 0;
					resetNote(i);
				}
			}		
		}
	}

	bool isNoteDifferent (int index, int pitch, int existingNote) {
		for (int i = 0; i < 3; i++) {
			if (nodes[leftAdjNodes[i]].extState == 1) { 		
				if (isPitchMode(leftSelNode)) {
					pitch += leftAdjPitch[i]; 
				}
			}

			if (nodes[rightAdjNodes[i]].extState == 1) {
				if (isPitchMode(rightSelNode)) {
					pitch += rightAdjPitch[i]; 
				}
			} 	
		}
			
		int newNote = 48 + index + pitch + octaveVal;
		newNote = constrain(newNote, 21, 108);	

		if (newNote != existingNote) 
			return true;
		else
			return false;	
	}	



	/* SUPPORT AND MANAGEMENT FUNCTIONS ------------------------------------------------------ */

	bool updateWaitOnAdj(int index) {
		if (waitOnAdj[index] > 0) {
			waitOnAdj[index] = waitOnAdj[index] - 1;
			if (waitOnAdj[index] == 0) return true;			
			else return false;
		}
		else return false;
	}

	void checkWaitOnAdj(int index) {
	    if (waitOnAdj[index] == 0) {
			waitOnAdj[index] = commandWaitFrames;
			if (sigNote[index] != 0) {
				usbMIDI.sendNoteOff(sigNote[index], 0, 1);	
				sigNote[index] = 0;
				if (sustain[index] == 1) {
					ledState[index] = 0;
					flicker[index] = 2; 		
				}				
			}   
		}
	}	

	void updateFlicker(int index) {
		if (flicker[index] > 0) {
			flicker[index] = flicker[index] - 1;
			if (flicker[index] == 0) {
				ledState[index] = 1;			
			}
		}
	}	

	void checkFlicker(int index) {
	    if (flicker[index] == 0) 
			ledState[index] = 1;
	}	

	void checkSustain(int index, unsigned int length) {
		if (length < 3000) {
		 	noteStartTime[index] = millis();
		 	sustain[index] = 1;
		}
	}

	void updateSustain(int index, unsigned int length) {
	    if (sustain[index] == 1) {
	 		if (millis() - noteStartTime[index] > length && !isHeld()) {
	   			usbMIDI.sendNoteOff(sigNote[index], 0, 1);
   				sigNote[index] = 0;
				resetNote(index);
	 		}
		}
	}

	void updateButtons() {	
		for (int i = 0; i < 3; i++){
			detectPress(&nodes[leftAdjNodes[i]], leftAdjNodes[i]);	
			detectPress(&nodes[rightAdjNodes[i]], rightAdjNodes[i]);
		} 
		detectPress(&nodes[hold], hold);	
		detectPress(&nodes[pause], pause);
		detectPress(&nodes[clear], clear);			
	}		

};


//---------------------------------------------------------------
//   INITIALIZE STRUCT
//---------------------------------------------------------------

commandPiano piano  {
    {85, 95, 86, 82, 91, 87, 90, 81, 83, 105, 55, 50, 47, 57, 
     74, 65, 41, 51, 63, 59, 68, 72, 70,  77, 78}, 
     97, 0, 
    119, {113, 115,  43}, { 32,  32,  64}, {  2,   4,   5},
	 73, { 75,  66,  71}, { 32,  64,  96}, {  2,   3,   7},
	 61, 48, 62 
};

