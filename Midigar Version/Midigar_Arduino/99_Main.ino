

void setup() {
	// Serial.begin(9600);
	screenInitialize();
	ledsInitialize();
	nodesInitialize();
  
}

void loop() {
	// UPDATE INPUTS -----------------------------------------------
	buttonsUpdate();

	// // CHECK FOR CHANGES -------------------------------------------
	modeSelect();

	// // -------------------------------------------------------------
	 if (modeCurrent == 2) {
	 	playMode();
	 }

	// // UPDATE OUTPUTS ----------------------------------------------
	nodesUpdate();
	ledsUpdate();
}
