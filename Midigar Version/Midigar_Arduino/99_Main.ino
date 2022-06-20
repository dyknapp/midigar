

void setup() {
	// Serial.begin(9600);
	screenInitialize();
	ledsInitialize();
	nodesInitialize();
  linksInitialize();
  modesInitialize();
}

void loop() {
	// UPDATE INPUTS --------------------------------------------------
	buttonsUpdate();

	// // CHECK FOR CHANGES -------------------------------------------
	modeSelect();

	// // -------------------------------------------------------------
	 if (modeCurrent == Play) {
	 	playMode();
	 }
   else if(modeCurrent == Patch){
     // patchMode() needs to come first because patch() makes additional changes to display states
     patchMode();
     // MAKE PATCHES ---------------------------------------------------
     patch();
   }

	// // UPDATE OUTPUTS ----------------------------------------------
	nodesUpdate();
	ledsUpdate();
}
