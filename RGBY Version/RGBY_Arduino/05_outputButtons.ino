//-----------------------------------------------------------------------------
//   GENERAL FUNCTIONS
//-----------------------------------------------------------------------------

// used for detecting button press on module's input nodes in play mode 
// when making an array and for looping through them all doesn't make sense
void detectPress(struct node *location, int index) {
	if (buttons.risingEdge(index)) 
		location->intState = 1;
	if (buttons.fallingEdge(index)) 
		location->intState = 0;
}

// press any white key in play mode to generate an output
void allOutputButtons() { 
	for (int i = 0; i < buttonsMax; i++) {
		if (nodes[i].type == 3) {
			if (buttons.vals[i]) {
				nodes[i].intState = 1;
			}
			else if (buttons.fallingEdge(i)) {
				nodes[i].intState = 0;
			}		
		}
	}	
}


//---------------------------------------------------------------
//   PERFORMANCE MODE SELECTION
//---------------------------------------------------------------

String screenUserMode[3] = {"EDIT", "LINK", "PLAY"};

void userModeSelect() {
	int userModeInputs[3] = {298, 299, 287};
	
	for (int i = 0; i < 3; i++) {
		if (buttons.risingEdge(userModeInputs[i])) {
			nodes[userModeInputs[userMode]].intState = 0;
			userMode = i;
			nodes[userModeInputs[i]].intState = 1;
			userModeNode = userModeInputs[i];
			screenPrint(screenUserMode[i]);
			leds.fillScreen(0);
		}
	}
}


//-----------------------------------------------------------------------------
//   COM CONTROLLER SELECTION 
//-----------------------------------------------------------------------------

String screenComCtrl[5] = {"VEL", "LEN", "PICH", "CC", " "};

void comCtrlSelect() {
	comCtrlUpdate();
	int comCtrlInputs[4] = {285, 301, 302, 296};

  	for (int i = 0; i < 4; i++) {
     	if (buttons.risingEdge(comCtrlInputs[i])) {
      		nodes[comCtrlInputs[comCtrlMode]].intState = 0;

        	if (comCtrlMode == i) {
	          comCtrlMode = 4;  
        	}
	        else {
	          nodes[comCtrlInputs[i]].intState = 1;
	          comCtrlMode = i;
	        }

	        comCtrlModeNode = comCtrlInputs[i];
	        screenPrint(screenComCtrl[i]);
	    }
  	}
}


void comCtrlUpdate() {
	int comCtrlpots[4] = {21, 15, 22, 20};
  	
  	for (int i = 0; i < 4; i++) {
    	if (pots.hasChanged(comCtrlpots[i])) {
			int potReading = pots.vals[comCtrlpots[i]];    

	      	switch (comCtrlMode) {
		        case 0: {
		            int vel = potReading;
		            vel = map(vel, 0, 150, 0, 127);
		            vel = constrain(vel, 0, 127);

		            if (vel != velAdjust[i]) {
			            velAdjust[i] = vel;
			            screenPrint(vel);
		            }
		            break;
		        }
		        case 1: {
		            unsigned int noteLength = potReading;
		            noteLength = fscale(noteLength, 0, 150, 5, 3000, -3);
		            noteLength = constrain(noteLength, 5, 3000);

		            if (noteLength != lengthAdjust[i]) {
			            if (noteLength < 3000)
			            	screenPrint(noteLength);
			            else 
			            	screenPrint(" OFF");
			            lengthAdjust[i] = noteLength;
		            }
		            break;
		       	}
		        case 2: {
		            int adj = potReading;
		            adj = map(adj, 0, 150, -24, 24);
		            adj = constrain(adj, -24, 24);

		            if (adj != pitchAdjust[i]) {
			            pitchAdjust[i] = adj;
			            screenPrint(adj);
		            }
		            break;
		        }
		        case 3: {
		            int ccVal = potReading;
		            ccVal = map(ccVal, 0, 150, 0, 127);
		            ccVal = constrain(ccVal, 0, 127);

		            if (ccVal != lastccVal[i]) {
			        	usbMIDI.sendControlChange(20 + i, ccVal, 1);
			            lastccVal[i] = ccVal;
			            screenPrint(ccVal);
		            }
		        	break;
				}
	      	}
    	}
  	}
}


float fscale(float inputValue, float originalMin, float originalMax, float newBegin, float newEnd, float curve) {

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; 
  curve = pow(10, curve); 

  if (inputValue < originalMin) 
    inputValue = originalMin;
  
  if (inputValue > originalMax) 
    inputValue = originalMax;

  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin)
    NewRange = newEnd - newBegin;
  else {
    NewRange = newBegin - newEnd; 
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;  

  if (originalMin > originalMax ) 
    return 0;

  if (invFlag == 0)
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;
  else  
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange); 

  return rangedValue;

}
