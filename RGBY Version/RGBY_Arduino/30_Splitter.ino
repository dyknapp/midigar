//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

// yellow input buttons - 276 			253 		271 
//						  254 			273 		266
// white output buttons - 272			240 275		263 258
//						  277 256 248	243			267	
// black midi button 	- 260
//
// black heart beat button not used
// other white buttons used as pushbutton outs when not in MIDI mode

int midiInNodes[] = {245, 244, 257};


//---------------------------------------------------------------
//   SPLITTER FUNCTIONS
//---------------------------------------------------------------

void splitterButtons() {
	detectPress(&nodes[276], 276);
	detectPress(&nodes[253], 253);
	detectPress(&nodes[271], 271);
	detectPress(&nodes[254], 254);
	detectPress(&nodes[273], 273);
	detectPress(&nodes[266], 266);
}


void splitterUpdate() {
	if (nodes[276].extHasChanged()) {
		nodes[272].intState = nodes[276].extState;
	}
	if (nodes[253].extHasChanged()) {
		nodes[240].intState = nodes[253].extState;
		nodes[275].intState = nodes[253].extState;
	}
	if (nodes[271].extHasChanged()) {
		nodes[263].intState = nodes[271].extState;
		nodes[258].intState = nodes[271].extState;
	}	
	if (nodes[254].extHasChanged()) {
		nodes[277].intState = nodes[254].extState;
		nodes[256].intState = nodes[254].extState;
		nodes[248].intState = nodes[254].extState;
	}
	if (nodes[273].extHasChanged()) {
		nodes[243].intState = nodes[273].extState;
	}	
	if (nodes[266].extHasChanged()) {
		nodes[267].intState = nodes[266].extState;
	}
}


//---------------------------------------------------------------
//   MIDI FUNCTIONS
//---------------------------------------------------------------

void midiUpdate() {
	if (buttons.risingEdge(260)) {
		nodes[260].intState = !nodes[260].intState; 
		clearMIDIinputs();
	}
	if (nodes[260].intState == 1) {
		usbMIDI.read();
	}	
}


void clearMIDIinputs() {
	if (nodes[260].intState == 0) {
		for (int i = 0; i < 3; i++) {
			nodes[midiInNodes[i]].intState = 0;
		}
	}
}


void OnNoteOn(byte channel, byte note, byte velocity) {
	for (int i = 0; i < 3; i++) {
  		if (note == 48+i && channel == 1) {
  			nodes[midiInNodes[i]].intState = 1;
  		}
  	}
}


void OnNoteOff(byte channel, byte note, byte velocity) {

  for (int i = 0; i < 3; i++) 
  	if (note == 48+i && channel == 1) 
  		nodes[midiInNodes[i]].intState = 0;
  
}