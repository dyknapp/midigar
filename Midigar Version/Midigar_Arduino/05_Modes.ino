//-----------------------------------------------------------------------------
//   INITIALIZE
//-----------------------------------------------------------------------------

void modesInitialize(){
  nodes[playNode].displayState = 1;
}

//-----------------------------------------------------------------------------
//   MODE SELECTION
//-----------------------------------------------------------------------------

int modeNodes[3] = {patchNode, reviewNode, playNode};
String screenMode[3] = {" Patch Mode", " Review Mode", " Play Mode"};

void modeSelect() {
	for (int i=0; i<3; i++) {
		if (nodes[modeNodes[i]].physicalRisingEdge()) {
		    nodes[modeNodes[modeCurrent]].displayState = 0;
			modeCurrent = i;
			screenPrint(screenMode[i]);
      nodes[modeNodes[i]].displayState = 1;
		}
	}
}


//-----------------------------------------------------------------------------
//   PLAY MODE
//-----------------------------------------------------------------------------

void playMode() {
	for (int i=0; i<totalNodes; i++) {
		if (nodes[i].type == 1 || nodes[i].type == 3) {
			nodes[i].displayState = nodes[i].physicalState;
		}
	}
}

//-----------------------------------------------------------------------------
//   PATCH MODE
//-----------------------------------------------------------------------------

void patchMode() {
  for (int i=0; i<totalNodes; i++) {
    if (nodes[i].type == 1 || nodes[i].type == 3) {
      nodes[i].displayState = nodes[i].physicalState;
    }
  }
}
