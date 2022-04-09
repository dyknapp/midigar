

void setup() {
    potsInit();
    ledsInit();
    screenInit();
    linksInit();
    usbMIDI.setHandleNoteOn(OnNoteOn);
    usbMIDI.setHandleNoteOff(OnNoteOff);    
    nodes[299].intState = 1;
    screenPrint(screenUserMode[userMode]);

    counterBlue.init(0, 0);
    counterRed.init(0, 0);
    counterGreen.init(3, 3);
}

void loop() {

    // UPDATE STATES -----------------------------------------------------------------

    /* Controller Messages */ 
    buttons.update();
    pots.update();
    

    // CHECK FOR INPUT MESSAGES ------------------------------------------------------

    /* Always Be Running */
    userModeSelect();
    comCtrlSelect();
    modifierSelect();
    piano.adjusters();
    strings.adjusters();

    /* Modules Nodes */
    if (modifierMode != 7) {
        modifierButtons();  
    }
    else {
        outplexSetMode();
        inplexSetMode();
        logicGateSel();

        if (userMode == 0 || userMode == 1) {
            patch();
        }
        else if (userMode == 2) {
            counterBlue.updateButtons();
            counterRed.updateButtons();
            counterGreen.updateButtons();
            outplexButtons();
            inplexButtons();
            logicButtons();
            splitterButtons();
            metroplexer.updateButtons();
            piano.updateButtons();
            strings.updateButtons();
        }
    }
    
    /* Links Messages */
    linkStateUpdate();


    // TESTS -------------------------------------------------------------------------
 
    // testButtons();
    // testPots();
    // tog.autoTog();


    // MODULE LOGIC ------------------------------------------------------------------

    applyModifierInput();

    counterBlue.update();
    counterRed.update();
    counterGreen.update();
    outplexUpdate();
    inplexUpdate();
    logicUpdate();
    splitterUpdate();
    midiUpdate();
    metroplexer.update();

    if (userMode == 2)
        allOutputButtons();

    applyModifierOutput();


    // KEYBOARD OUTPUT ------------------------------------------------------------------

    piano.detectNotes();    
    strings.detectNotes(); 


    // UPDATES -----------------------------------------------------------------------

    nodesUpdateLastStates();
    updateLEDs();

    buttons.updateLastVals(); 
    pots.updateLastVals();

}


