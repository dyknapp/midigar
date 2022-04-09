//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

struct commandStrings {
    int keys[14];
    int tuners[14];

    int octave;
    int octaveVal;

    int leftSelNode;
    int leftAdjNodes[2];
    int leftAdjVel[2];
    int leftAdjPitch[2];

    int rightSelNode;
    int rightAdjNodes[3];
    int rightAdjVel[3];
    int rightAdjPitch[3];

    int hold;
    int pause;
    int clear;

    byte keyNote[14];
    byte sigNote[14];
    unsigned long noteStartTime[14];
    bool sustain[14];
    bool ledState[14];
    byte flicker[14];
    byte waitOnAdj[14];

    byte tunerVal[14];
    byte lastTunerVal[14];


    /* SET ADJUSTERS FUNCTIONS --------------------------------------------------------------- */

    void adjusters() {
    	octaveUpdate();
    	adjUpdate(leftAdjNodes, leftAdjPitch, leftAdjVel, leftSelNode, 2);
    	adjUpdate(rightAdjNodes, rightAdjPitch, rightAdjVel, rightSelNode, 3);
    	tunerReadings();		
    }

    void octaveUpdate() {
    	if (buttons.vals[octave] == 1) {
    		if (buttons.lastVals[octave] == 0) 
    			commandScreenPrint(octaveVal);
    		if (pots.hasChanged(23)) 
    			octaveVal = commandSetValue(-3, 3, 12);
    	}
    }

    void adjUpdate(int adjBank[], int pitchSet[], int velSet[], int selNode, int size) {
    	if (buttons.risingEdge(selNode)) 
    		nodes[selNode].intState = !nodes[selNode].intState; 

    	for (int i = 0; i < size; i++) {
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

    void tunerReadings() {
    	for (int i = 0; i < 14; i++) {
    		if (pots.vals[tuners[i]] != pots.lastVals[tuners[i]]) {
		     	tunerVal[i] = pots.vals[tuners[i]];
		        tunerVal[i] = map(tunerVal[i], 0, 150, 21, 108);
		        tunerVal[i] = constrain(tunerVal[i], 21, 108); 

	    		if (tunerVal[i] != lastTunerVal[i]) {
	    			screenPrint(tunerVal[i]);	
	    			lastTunerVal[i] = tunerVal[i];
	    		}
	    	}
    	}
    }

    bool isPitchMode(int selNode) {
    	if (nodes[selNode].intState == 0) 
    		return true;
    	else 	
    		return false;
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
	    for (int i = 0; i < 14; i++) {
			if (buttons.risingEdge(keys[i])) {	
				sendNote(i, velAdjust[2], pitchAdjust[2], 0);
			}
		    else if (buttons.fallingEdge(keys[i]) && !isHeld()) {
				if (keyNote[i] != 0) {
					usbMIDI.sendNoteOff(keyNote[i], 0, 3);	 
					keyNote[i] = 0;
				}   
		    }
	    }
	}


	/* SIGNAL TRIGGERED NOTE FUNCTIONS ------------------------------------------------------- */

	void detectSignalNotes() {
		for (int i = 0; i < 14; i++) {
			updateFlicker(i);

			if (updateWaitOnAdj(i)) {
				checkSustain(i, lengthAdjust[2]);
				sendNote(i, velAdjust[2], pitchAdjust[2], 1);
				checkFlicker(i);
			}

			if (nodes[keys[i]].extRisingEdge()) 
				checkWaitOnAdj(i);
			
			else if (nodes[keys[i]].extFallingEdge() && sustain[i] == 0 && !isHeld()) {
				if (sigNote[i] != 0) {
					usbMIDI.sendNoteOff(sigNote[i], 0, 3);	
					sigNote[i] = 0;
					resetNote(i);
				}
			}
			updateSustain(i, lengthAdjust[2]);
		}
	}


	/* MAIN NOTE GENERATION FUNCTION --------------------------------------------------------- */
	
	void sendNote(int index, int vel, int pitch, bool type) { 
		for (int i = 0; i < 2; i++) {														
			if (nodes[leftAdjNodes[i]].extState == 1) { 		
				if (isPitchMode(leftSelNode)) pitch += leftAdjPitch[i]; 
				else vel += leftAdjVel[i]; 
			}
		}
		for (int i = 0; i < 3; i++) {														
			if (nodes[rightAdjNodes[i]].extState == 1) {
				if (isPitchMode(rightSelNode)) pitch += rightAdjPitch[i]; 
				else vel += rightAdjVel[i]; 
			} 
		}

		int outNote = tunerVal[index] + pitch + octaveVal;
		outNote = constrain(outNote, 21, 108);
		int outVel = vel;	
		outVel = constrain(vel, 1, 127);

		if (type == 0) {
			if (keyNote[index] != outNote) {
				keyNote[index] = outNote;
				usbMIDI.sendNoteOn(outNote, outVel, 3);
			}
		}	
		else {
			if (sigNote[index] != outNote) {
				sigNote[index] = outNote;   
				usbMIDI.sendNoteOn(outNote, outVel, 3);
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
			for (int i = 0; i < 14; i++) {
				if (keyNote[i] != 0) {
					usbMIDI.sendNoteOff(keyNote[i], 0, 3);	 
					keyNote[i] = 0;
				}
				if (sigNote[i] != 0) {
					usbMIDI.sendNoteOff(sigNote[i], 0, 3);	 
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
	    for (int i = 0; i < 14; i++) {
			if (buttons.vals[i] == 1) {
				sendNote(i, velAdjust[2], pitchAdjust[2], 0);
			}
			else {
				if (keyNote[i] != 0) {
					usbMIDI.sendNoteOff(keyNote[i], 0, 3);	 
					keyNote[i] = 0;
				}   
			}

			if (nodes[keys[i]].extState == 1) {
				if (nodes[keys[i]].extState == 1) {
					if (isNoteDifferent(i, pitchAdjust[2], sigNote[i])) {
						usbMIDI.sendNoteOff(sigNote[i], 0, 3);	
						resetNote(i);
						checkWaitOnAdj(i);
					}
				}
			}
			else {
				if (sigNote[i] != 0) {
					usbMIDI.sendNoteOff(sigNote[i], 0, 3);	
					sigNote[i] = 0;
					resetNote(i);
				}
			}		
		}
	}

	bool isNoteDifferent (int index, int pitch, int existingNote) {
		for (int i = 0; i < 2; i++) {
			if (nodes[leftAdjNodes[i]].extState == 1) { 		
				if (isPitchMode(leftSelNode)) {
					pitch += leftAdjPitch[i]; 
				}
			}
		}
		for (int i = 0; i < 3; i++) {
			if (nodes[rightAdjNodes[i]].extState == 1) {
				if (isPitchMode(rightSelNode)) {
					pitch += rightAdjPitch[i]; 
				}
			} 	
		}
		
		int newNote = tunerVal[index] + pitch + octaveVal;
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
				usbMIDI.sendNoteOff(sigNote[index], 0, 3);	
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
	   			usbMIDI.sendNoteOff(sigNote[index], 0, 3);
   				sigNote[index] = 0;
				resetNote(index);
	 		}
		}
	}

	void updateButtons() {	
		for (int i = 0; i < 2; i++)
			detectPress(&nodes[leftAdjNodes[i]], leftAdjNodes[i]);	
		for (int i = 0; i < 3; i++)
			detectPress(&nodes[rightAdjNodes[i]], rightAdjNodes[i]);
		detectPress(&nodes[hold], hold);	
		detectPress(&nodes[pause], pause);
		detectPress(&nodes[clear], clear);			
	}		

};


//---------------------------------------------------------------
//   INITIALIZE STRUCT
//---------------------------------------------------------------

commandStrings strings  {
	{69, 56, 76, 40, 53, 45, 54, 52, 44,  3,  7, 15,  2, 23},
	{ 0,  9, 17, 10,  2,  3,  8, 11,  1, 18, 16, 19, 25, 24},
	27, 0,
    18, {26, 17}, {32, 64}, { 2, 4},
	35, {19, 31, 9}, {32, 64, 96}, { 2, 3, 7},
	1, 10, 11
};
