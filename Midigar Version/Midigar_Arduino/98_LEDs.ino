//---------------------------------------------------------------
//   MAIN
//---------------------------------------------------------------

void ledsUpdate() {
    ledsClear();
    ledsDisplayStates();
    leds.write();
}


//---------------------------------------------------------------
//   SUPPORT FUNCTIONS
//---------------------------------------------------------------

void ledsClear() {
    leds.fillScreen(0);
}

void ledsDisplayStates() {
  for (int i=0; i<totalNodes; i++) {
        leds.drawPixel(nodes[i].ledx, nodes[i].ledy, nodes[i].displayState);
  }
}
