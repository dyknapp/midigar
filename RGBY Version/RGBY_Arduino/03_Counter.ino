//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

byte cPerms[8][16] {
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, 
	{0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15},
	{0, 2, 4, 6, 1, 3, 5, 7, 8, 10, 12, 14, 9, 11, 13, 15}, 
	{0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15},
	{0, 7, 1, 6, 2, 5, 3, 4, 8, 15, 9, 14, 10, 13, 11, 12}, 
	{0, 3, 2, 1, 4, 7, 6, 5, 8, 11, 10, 9, 12, 15, 14, 13},
	{1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14}, 
	{7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8}
};


//---------------------------------------------------------------
//   OUTPLEX STRUCT
//---------------------------------------------------------------

struct counter {
	int inc;
	int dec;
	int dial;

	int reset;
	int hold;
	int pause;
	int clear;

	int outsA[16];
	int outsB[8];

	long posA, posB;

	unsigned long totalSteps;
	unsigned long lastMillis;
	unsigned int interval;

	int incPatched;

	byte modeSelA, modeSelB;
	byte subSelA, subSelB;
	byte auxSelA, auxSelB;
	byte permSelA, permSelB;

	int directionA, directionB;
	int deckA[16];
	int deckB[8];
	bool bufferA[8];
	bool bufferB[8];
	bool bufferC[16];

	/* MAIN UPDATE ------------------------------------------- */

	void init(int mode, int sub) {
		modeSelB = mode;
		subSelB = sub;
		nodes[outsA[posA]].intState = 1;		
		directionA = 1;
		directionB = 1;
		unshuffle(deckA, 16);
		unshuffle(deckB, 8);
	}	

	void update() {	
		updateDial();

		checkReset();			
		if (!isPaused() && !isCleared()) 
			signalChanged();
		checkHoldRelease();
		checkPauseRelease();
		checkClear();

		updateOutputs();
	}


//---------------------------------------------------------------
//   CLOCK
//---------------------------------------------------------------	

	void updateDial() {
		if (!incPatched) {
			interval = pots.vals[dial];
			interval = fscale(interval, 0, 1023, 15, 2000, -4);

			if (millis() - lastMillis > interval) {
				totalSteps += 1;
				lastMillis = millis();
			}
		}
	}

	void clockSeq() {
		if (totalSteps % 2 == 0) 
			nodes[inc].intState = 0;
		else 
			nodes[inc].intState = 1;	
	}


	/* GENERATING OUTPUT FUNCTIONS --------------------------- */

	void signalChanged() {
		updateSeq();
		updateDivider();
		updateRipple();
		updateBinary();
	}


	void updateOutputs() {
		clockSeq();

		if (!isCleared()) {
			if (!isHeld()) 
				clearOutputs();
			
			outputSeq();
			outputDivider();	
			outputRipple();
			outputBinary();
		}
	}


//----------------------------------------------------------------------------------------------------------------
//   SEQUENCER MODE
//----------------------------------------------------------------------------------------------------------------

	/* CORE FUNCTIONS ---------------------------------------- */

	void updateSeq() {
		if (isSameSub(0, 0)) updateForward(16, &posA);
		if (isSameSub(0, 1)) updatePingPong(16, &directionA, &posA);
		if (isSameSub(0, 2)) updateBrownian(16, &posA);
		if (isSameSub(0, 3)) updateShuffle(16, &posA, deckA);

		if (modeSelA != modeSelB || subSelA != subSelB) {
			if (modeSelA == 0) {
				if (subSelA == 0) updateForward(8, &posA);
				if (subSelA == 1) updatePingPong(8, &directionA, &posA);
				if (subSelA == 2) updateBrownian(8, &posA);
				if (subSelA == 3) updateShuffle(8, &posA, deckA);
			}
			if (modeSelB == 0) {
				if (subSelB == 0) updateForward(8, &posB);
				if (subSelB == 1) updatePingPong(8, &directionB, &posB);
				if (subSelB == 2) updateBrownian(8, &posB);
				if (subSelB == 3) updateShuffle(8, &posB, deckB);
			}
		}	
	}

	void outputSeq() {
		if (modeSelA == 0 || modeSelB == 0) {
			if (modeSelA != modeSelB || subSelA != subSelB) {
				if (modeSelA == 0) setSeqNodes(posA, outsA, permSelA, auxSelA, deckA, subSelA);
				if (modeSelB == 0) setSeqNodes(posB, outsB, permSelB, auxSelB, deckB, subSelB);
			}
			else
				setSeqNodes(posA, outsA, permSelA, auxSelA, deckA, subSelA);	
		}
	}

	void setSeqNodes(long seqPos, int outs[], int permSel, int auxSel, int deck[], int subSel) {
		int outNode;
		if (subSel == 3)
			outNode = outs[deck[seqPos]];
		else
			outNode = outs[cPerms[permSel][seqPos]];

		if (auxSel == 1) {
			if (!isHeld())
				nodes[outNode].intState = nodes[inc].extState;
			else if (isHeld() && nodes[inc].extState == 1)
				nodes[outNode].intState = 1;
		}
		else
			nodes[outNode].intState = 1;		
	}


	/* SUB 0 : FORWARD MODE ------------------------------------ */

	void updateForward(int length, long *seqPos) {
		if (userMode == 2) {
			if (buttons.risingEdge(inc)) *seqPos += 1;
			else if (buttons.risingEdge(dec)) *seqPos -= 1;
		}
		if (nodes[inc].extRisingEdge()) *seqPos += 1;
		else if (nodes[dec].extRisingEdge()) *seqPos -= 1;	
		wrapAround(&*seqPos, length);
	}


	/* SUB 1 : PING-PONG MODE ---------------------------------- */	

	void updatePingPong(int length, int *direction, long *seqPos) {
		if (userMode == 2) {
			if (buttons.risingEdge(inc)) *seqPos += *direction;
			else if (buttons.risingEdge(dec)) *direction = -*direction;
		}
		if (nodes[inc].extRisingEdge()) *seqPos += *direction;
		else if (nodes[dec].extRisingEdge()) *direction = -*direction;	
		bounceBack(&*direction, &*seqPos, length);
	}

	void bounceBack(int *direction, long *seqPos, int length) {
		if (*seqPos == length || *seqPos == -1) {
			*direction = -*direction;
			*seqPos += *direction;
			*seqPos += *direction;
		}
	}


	/* SUB 2 : BROWNIAN ---------------------------------------- */		

	void updateBrownian(int length, long *seqPos) {
		if (userMode == 2) {
			if (buttons.risingEdge(inc)) *seqPos += random(2);
			else if (buttons.risingEdge(dec)) *seqPos -= random(2);
		}
		if (nodes[inc].extRisingEdge()) *seqPos += random(2);
		else if (nodes[dec].extRisingEdge()) *seqPos -= random(2);	
		wrapAround(&*seqPos, length);
	}


	/* SUB 3 : SHUFFLE SEQUENCE -------------------------------- */		

	void updateShuffle(int length, long *seqPos, int deck[]) {
		if (userMode == 2) {
			if (buttons.risingEdge(inc)) *seqPos += 1;
			else if (buttons.risingEdge(dec)) *seqPos -= 1;
		}
		if (nodes[inc].extRisingEdge()) *seqPos += 1;
		else if (nodes[dec].extRisingEdge()) *seqPos -= 1;	
		shuffle(&*seqPos, deck, length);
	}

	void shuffle(long *seqPos, int deck[], int length) { 
		if (*seqPos == length || *seqPos == -1) {
			*seqPos = (*seqPos+length) % length;

		    for (int i = 0; i < length; i++) { 
		        int newPosition = random(length); 
		        int valueToSwap = deck[newPosition];
		        deck[newPosition] = deck[i];
		        deck[i] = valueToSwap;
		    } 
		}
	} 

	void unshuffle(int deck[], int length) {
		for (int i = 0; i < length; i++) 
			deck[i] = i;
	}

	/* HELPER FUNCTIONS THAT HELP ------------------------------ */		

	void wrapAround(long *seqPos, long length) {
		if (*seqPos == length || *seqPos == -1) 
			*seqPos = (*seqPos+length) % length;
	}	


//----------------------------------------------------------------------------------------------------------------
//   SUBDIVIDERS MODE
//----------------------------------------------------------------------------------------------------------------

	/* CORE FUNCTIONS ---------------------------------------- */

	void outputDivider() {	
		if (modeSelA == 1 || modeSelB == 1) {
			if (modeSelA != modeSelB || subSelA != subSelB) {
				if (modeSelA == 1) {
					if (subSelA == 0) {
						setDivNodes(posA,   2,   0,   0,  0, outsA, permSelA);
						setDivNodes(posA,   2,   1,   1,  1, outsA, permSelA);
						setDivNodes(posA,   4,   0,   1,  2, outsA, permSelA);
						setDivNodes(posA,   4,   2,   3,  3, outsA, permSelA);
						setDivNodes(posA,   8,   0,   3,  4, outsA, permSelA);
						setDivNodes(posA,   8,   4,   7,  5, outsA, permSelA);
						setDivNodes(posA,  16,   0,   7,  6, outsA, permSelA);
						setDivNodes(posA,  16,   8,  15,  7, outsA, permSelA);
					}
					if (subSelA == 1) {
						setDivNodes(posA,   3,   0,   0,  0, outsA, permSelA);
						setDivNodes(posA,   3,   1,   1,  1, outsA, permSelA);
						setDivNodes(posA,   3,   2,   2,  2, outsA, permSelA);
						setDivNodes(posA,   9,   0,   2,  3, outsA, permSelA);
						setDivNodes(posA,   9,   3,   5,  4, outsA, permSelA);
						setDivNodes(posA,   9,   6,   8,  5, outsA, permSelA);
						setDivNodes(posA,  27,   0,   8,  6, outsA, permSelA);
						setDivNodes(posA,  27,   9,  17,  7, outsA, permSelA);
					}
					if (subSelA == 2) {
						setDivNodes(posA,   4,   0,   0,  0, outsA, permSelA);
						setDivNodes(posA,   4,   1,   1,  1, outsA, permSelA);
						setDivNodes(posA,   4,   2,   2,  2, outsA, permSelA);
						setDivNodes(posA,   4,   3,   3,  3, outsA, permSelA);
						setDivNodes(posA,  16,   0,   3,  4, outsA, permSelA);
						setDivNodes(posA,  16,   4,   7,  5, outsA, permSelA);
						setDivNodes(posA,  16,   8,  11,  6, outsA, permSelA);
						setDivNodes(posA,  16,  12,  15,  7, outsA, permSelA);
					}
					if (subSelA == 3) 
						for (int i = 0; i < 8; i++) 
							nodes[outsA[cPerms[permSelA][i]]].intState = bufferA[i];
	
				}
				if (modeSelB == 1) {
					if (subSelB == 0) {
						setDivNodes(posB,   2,   0,   0,  0, outsB, permSelB);
						setDivNodes(posB,   2,   1,   1,  1, outsB, permSelB);
						setDivNodes(posB,   4,   0,   1,  2, outsB, permSelB);
						setDivNodes(posB,   4,   2,   3,  3, outsB, permSelB);
						setDivNodes(posB,   8,   0,   3,  4, outsB, permSelB);
						setDivNodes(posB,   8,   4,   7,  5, outsB, permSelB);
						setDivNodes(posB,  16,   0,   7,  6, outsB, permSelB);
						setDivNodes(posB,  16,   8,  15,  7, outsB, permSelB);
					}
					if (subSelB == 1) {
						setDivNodes(posB,   3,   0,   0,  0, outsB, permSelB);
						setDivNodes(posB,   3,   1,   1,  1, outsB, permSelB);
						setDivNodes(posB,   3,   2,   2,  2, outsB, permSelB);
						setDivNodes(posB,   9,   0,   2,  3, outsB, permSelB);
						setDivNodes(posB,   9,   3,   5,  4, outsB, permSelB);
						setDivNodes(posB,   9,   6,   8,  5, outsB, permSelB);
						setDivNodes(posB,  27,   0,   8,  6, outsB, permSelB);
						setDivNodes(posB,  27,   9,  17,  7, outsB, permSelB);
					}
					if (subSelB == 2) {
						setDivNodes(posB,   4,   0,   0,  0, outsB, permSelB);
						setDivNodes(posB,   4,   1,   1,  1, outsB, permSelB);
						setDivNodes(posB,   4,   2,   2,  2, outsB, permSelB);
						setDivNodes(posB,   4,   3,   3,  3, outsB, permSelB);
						setDivNodes(posB,  16,   0,   3,  4, outsB, permSelB);
						setDivNodes(posB,  16,   4,   7,  5, outsB, permSelB);
						setDivNodes(posB,  16,   8,  11,  6, outsB, permSelB);
						setDivNodes(posB,  16,  12,  15,  7, outsB, permSelB);
					}
					if (subSelB == 3) 
						for (int i = 0; i < 8; i++) 
							nodes[outsB[cPerms[permSelB][i]]].intState = bufferB[i];
				}			
			}
			else {
				if (isSameSub(1, 0)) {
					setDivNodes(posA,   2,   0,   0,  0, outsA, permSelA);
					setDivNodes(posA,   2,   1,   1,  1, outsA, permSelA);
					setDivNodes(posA,   4,   0,   1,  2, outsA, permSelA);
					setDivNodes(posA,   4,   2,   3,  3, outsA, permSelA);
					setDivNodes(posA,   8,   0,   3,  4, outsA, permSelA);
					setDivNodes(posA,   8,   4,   7,  5, outsA, permSelA);
					setDivNodes(posA,  16,   0,   7,  6, outsA, permSelA);
					setDivNodes(posA,  16,   8,  15,  7, outsA, permSelA);
					setDivNodes(posA,  32,   0,  15,  8, outsA, permSelA);
					setDivNodes(posA,  32,  16,  31,  9, outsA, permSelA);
					setDivNodes(posA,  64,   0,  31, 10, outsA, permSelA);
					setDivNodes(posA,  64,  32,  63, 11, outsA, permSelA);		
					setDivNodes(posA, 128,   0,  63, 12, outsA, permSelA);
					setDivNodes(posA, 128,  64, 127, 13, outsA, permSelA);
					setDivNodes(posA, 256,   0, 127, 14, outsA, permSelA);
					setDivNodes(posA, 256, 128, 255, 15, outsA, permSelA);	
				}
				if (isSameSub(1, 1)) {
					setDivNodes(posA,   3,   0,   0,  0, outsA, permSelA);
					setDivNodes(posA,   3,   1,   1,  1, outsA, permSelA);
					setDivNodes(posA,   3,   2,   2,  2, outsA, permSelA);
					setDivNodes(posA,   9,   0,   2,  3, outsA, permSelA);
					setDivNodes(posA,   9,   3,   5,  4, outsA, permSelA);
					setDivNodes(posA,   9,   6,   8,  5, outsA, permSelA);
					setDivNodes(posA,  27,   0,   8,  6, outsA, permSelA);
					setDivNodes(posA,  27,   9,  17,  7, outsA, permSelA);
					setDivNodes(posA,  27,  18,  26,  8, outsA, permSelA);
					setDivNodes(posA,  81,   0,  26,  9, outsA, permSelA);
					setDivNodes(posA,  81,  27,  53, 10, outsA, permSelA);
					setDivNodes(posA,  81,  54,  80, 11, outsA, permSelA);		
					setDivNodes(posA, 243,   0,  80, 12, outsA, permSelA);
					setDivNodes(posA, 243,  81, 161, 13, outsA, permSelA);
					setDivNodes(posA, 243, 162, 242, 14, outsA, permSelA);
					setDivNodes(posA, 729,   0, 242, 15, outsA, permSelA);
				}
				if (isSameSub(1, 2)) {
					setDivNodes(posA,   4,   0,   0,  0, outsA, permSelA);
					setDivNodes(posA,   4,   1,   1,  1, outsA, permSelA);
					setDivNodes(posA,   4,   2,   2,  2, outsA, permSelA);
					setDivNodes(posA,   4,   3,   3,  3, outsA, permSelA);
					setDivNodes(posA,  16,   0,   3,  4, outsA, permSelA);
					setDivNodes(posA,  16,   4,   7,  5, outsA, permSelA);
					setDivNodes(posA,  16,   8,  11,  6, outsA, permSelA);
					setDivNodes(posA,  16,  12,  15,  7, outsA, permSelA);
					setDivNodes(posA,  64,   0,  15,  8, outsA, permSelA);
					setDivNodes(posA,  64,  16,  31,  9, outsA, permSelA);
					setDivNodes(posA,  64,  32,  47, 10, outsA, permSelA);
					setDivNodes(posA,  64,  48,  63, 11, outsA, permSelA);		
					setDivNodes(posA, 256,   0,  63, 12, outsA, permSelA);
					setDivNodes(posA, 256,  64, 127, 13, outsA, permSelA);
					setDivNodes(posA, 256, 128, 191, 14, outsA, permSelA);
					setDivNodes(posA, 256, 192, 254, 15, outsA, permSelA);	
				}	
				if (isSameSub(1, 3)) 
					for (int i = 0; i < 16; i++) 
						nodes[outsA[cPerms[permSelA][i]]].intState = bufferC[i];			
			}			
		}
	}

	void setDivNodes(long seqPos, long divisor, long botRemain, long topRemain, int pos, int outs[], int permSel) {
		if (seqPos % divisor >= botRemain && seqPos % divisor <= topRemain) nodes[outs[cPerms[permSel][pos]]].intState = 1;
		else if (!isHeld()) nodes[outs[cPerms[permSel][pos]]].intState = 0;
	}

	void updateDivider() {	
		if (isSameSub(1, 0)) updateDivCount(256, &posA);
		if (isSameSub(1, 1)) updateDivCount(729, &posA);
		if (isSameSub(1, 2)) updateDivCount(256, &posA);
		if (isSameSub(1, 3)) updateRandomC();

		if (modeSelA != modeSelB || subSelA != subSelB) {
			if (modeSelA == 1) {
				if (subSelA == 0) updateDivCount(16, &posA);
				if (subSelA == 1) updateDivCount(27, &posA);
				if (subSelA == 2) updateDivCount(16, &posA);
				if (subSelA == 3) updateRandomA();
			}
			if (modeSelB == 1) {
				if (subSelB == 0) updateDivCount(16, &posB);
				if (subSelB == 1) updateDivCount(27, &posB);
				if (subSelB == 2) updateDivCount(16, &posB);
				if (subSelB == 3) updateRandomB();
			}
		}	
		
	}

	/* SUB ----------------------------------------------------------- */	

	void updateDivCount(int length, long *seqPos) {
		if (userMode == 2) {
			if (buttons.risingEdge(inc))
				*seqPos += 1;
			else if (buttons.risingEdge(dec)) 
				*seqPos -= 1;
		}
		if (nodes[inc].extRisingEdge()) 
			*seqPos += 1;
		else if (nodes[dec].extRisingEdge()) 
			*seqPos -= 1;	
		wrapAround(&*seqPos, length);
	}

	void updateRandomA() {
		if (userMode == 2) {
			if (buttons.risingEdge(inc)) {
				for (int i = 0; i < 8; i++) {
					if (random(100) <= (i+1)*10) bufferA[i] = 1;	
					else if (!isHeld()) bufferA[i] = 0;	
				}
			}
		}
			
		if (nodes[inc].extRisingEdge()) {
			for (int i = 0; i < 8; i++) {
				if (random(100) <= (i+1)*10) bufferA[i] = 1;	
				else if (!isHeld()) bufferA[i] = 0;	
			}
		}
	}

	void updateRandomB() {
		if (userMode == 2) {
			if (buttons.risingEdge(inc)) {
				for (int i = 0; i < 8; i++) {
					if (random(100) <= (i+1)*10) bufferB[i] = 1;	
					else if (!isHeld()) bufferB[i] = 0;	
				}
			}
		}
			
		if (nodes[inc].extRisingEdge()) {
			for (int i = 0; i < 8; i++) {
				if (random(100) <= (i+1)*10) bufferB[i] = 1;	
				else if (!isHeld()) bufferB[i] = 0;	
			}
		}
	}

	void updateRandomC() {
		if (userMode == 2) {
			if (buttons.risingEdge(inc)) {
				for (int i = 0; i < 16; i++) {
					if (random(100) <= (i+1)*5) bufferC[i] = 1;	
					else if (!isHeld()) bufferC[i] = 0;	
				}
			}
		}
			
		if (nodes[inc].extRisingEdge()) {
			for (int i = 0; i < 16; i++) {
				if (random(100) <= (i+1)*5) bufferC[i] = 1;	
				else if (!isHeld()) bufferC[i] = 0;	
			}
		}
	}


//----------------------------------------------------------------------------------------------------------------
//   DIVIDER MODE
//----------------------------------------------------------------------------------------------------------------

	void outputRipple() {	
		if (modeSelA == 2 || modeSelB == 2) {
			if (modeSelA != modeSelB || subSelA != subSelB) {
				if (modeSelA == 2) {
					if (subSelA == 0) {
						setDivNodes(posA,     2,     1,     2,  0, outsA, permSelA);
						setDivNodes(posA,     4,     2,     3,  1, outsA, permSelA);
						setDivNodes(posA,     8,     4,     7,  2, outsA, permSelA);
						setDivNodes(posA,    16,     8,    15,  3, outsA, permSelA);
						setDivNodes(posA,    32,    16,    31,  4, outsA, permSelA);
						setDivNodes(posA,    64,    32,    63,  5, outsA, permSelA);
						setDivNodes(posA,   128,    64,   127,  6, outsA, permSelA);
						setDivNodes(posA,   256,   128,   255,  7, outsA, permSelA);
					}
					if (subSelA == 1) {
						setDivNodes(posA,     2,     1,     2,  0, outsA, permSelA);
						setDivNodes(posA,     4,     2,     3,  1, outsA, permSelA);
						setDivNodes(posA,     6,     3,     5,  2, outsA, permSelA);
						setDivNodes(posA,     8,     4,     7,  3, outsA, permSelA);
						setDivNodes(posA,    10,     5,     9,  4, outsA, permSelA);
						setDivNodes(posA,    12,     6,    11,  5, outsA, permSelA);
						setDivNodes(posA,    14,     7,    13,  6, outsA, permSelA);
						setDivNodes(posA,    16,     8,    15,  7, outsA, permSelA);
					}
					if (subSelA == 2) {
						setDivNodes(posA,     2,     0,     0,  0, outsA, permSelA);
						setDivNodes(posA,     4,     0,     0,  1, outsA, permSelA);
						setDivNodes(posA,     8,     0,     0,  2, outsA, permSelA);
						setDivNodes(posA,    16,     0,     0,  3, outsA, permSelA);
						setDivNodes(posA,    32,     0,     0,  4, outsA, permSelA);
						setDivNodes(posA,    64,     0,     0,  5, outsA, permSelA);
						setDivNodes(posA,   128,     0,     0,  6, outsA, permSelA);
						setDivNodes(posA,   256,     0,     0,  7, outsA, permSelA);
					}
					if (subSelA == 3) {
						setDivNodes(posA,     6,     3,     5,  0, outsA, permSelA);
						setDivNodes(posA,    12,     6,    11,  1, outsA, permSelA);
						setDivNodes(posA,    18,    12,    17,  2, outsA, permSelA);
						setDivNodes(posA,    24,    18,    23,  3, outsA, permSelA);
						setDivNodes(posA,    30,    24,    29,  4, outsA, permSelA);
						setDivNodes(posA,    36,    30,    35,  5, outsA, permSelA);
						setDivNodes(posA,    42,    36,    41,  6, outsA, permSelA);
						setDivNodes(posA,    48,    42,    47,  7, outsA, permSelA);
					}
	
				}
				if (modeSelB == 2) {
					if (subSelB == 0) {
						setDivNodes(posB,     2,     1,     2,  0, outsB, permSelB);
						setDivNodes(posB,     4,     2,     3,  1, outsB, permSelB);
						setDivNodes(posB,     8,     4,     7,  2, outsB, permSelB);
						setDivNodes(posB,    16,     8,    15,  3, outsB, permSelB);
						setDivNodes(posB,    32,    16,    31,  4, outsB, permSelB);
						setDivNodes(posB,    64,    32,    63,  5, outsB, permSelB);
						setDivNodes(posB,   128,    64,   127,  6, outsB, permSelB);
						setDivNodes(posB,   256,   128,   255,  7, outsB, permSelB);
					}
					if (subSelB == 1) {
						setDivNodes(posB,     2,     1,     2,  0, outsB, permSelB);
						setDivNodes(posB,     4,     2,     3,  1, outsB, permSelB);
						setDivNodes(posB,     6,     3,     5,  2, outsB, permSelB);
						setDivNodes(posB,     8,     4,     7,  3, outsB, permSelB);
						setDivNodes(posB,    10,     5,     9,  4, outsB, permSelB);
						setDivNodes(posB,    12,     6,    11,  5, outsB, permSelB);
						setDivNodes(posB,    14,     7,    13,  6, outsB, permSelB);
						setDivNodes(posB,    16,     8,    15,  7, outsB, permSelB);
					}
					if (subSelB == 2) {
						setDivNodes(posB,     2,     0,     0,  0, outsB, permSelB);
						setDivNodes(posB,     4,     0,     0,  1, outsB, permSelB);
						setDivNodes(posB,     8,     0,     0,  2, outsB, permSelB);
						setDivNodes(posB,    16,     0,     0,  3, outsB, permSelB);
						setDivNodes(posB,    32,     0,     0,  4, outsB, permSelB);
						setDivNodes(posB,    64,     0,     0,  5, outsB, permSelB);
						setDivNodes(posB,   128,     0,     0,  6, outsB, permSelB);
						setDivNodes(posB,   256,     0,     0,  7, outsB, permSelB);
					}
					if (subSelB == 3) {
						setDivNodes(posB,     6,     3,     5,  0, outsB, permSelB);
						setDivNodes(posB,    12,     6,    11,  1, outsB, permSelB);
						setDivNodes(posB,    18,     9,    17,  2, outsB, permSelB);
						setDivNodes(posB,    24,    12,    23,  3, outsB, permSelB);
						setDivNodes(posB,    30,    15,    29,  4, outsB, permSelB);
						setDivNodes(posB,    36,    18,    35,  5, outsB, permSelB);
						setDivNodes(posB,    42,    21,    41,  6, outsB, permSelB);
						setDivNodes(posB,    48,    24,    47,  7, outsB, permSelB);
					}
				}			
			}
			else {
				if (isSameSub(2, 0)) {
					setDivNodes(posA,     2,     1,     2,  0, outsA, permSelA);
					setDivNodes(posA,     4,     2,     3,  1, outsA, permSelA);
					setDivNodes(posA,     8,     4,     7,  2, outsA, permSelA);
					setDivNodes(posA,    16,     8,    15,  3, outsA, permSelA);
					setDivNodes(posA,    32,    16,    31,  4, outsA, permSelA);
					setDivNodes(posA,    64,    32,    63,  5, outsA, permSelA);
					setDivNodes(posA,   128,    64,   127,  6, outsA, permSelA);
					setDivNodes(posA,   256,   128,   255,  7, outsA, permSelA);
					setDivNodes(posA,   512,   256,   511,  8, outsA, permSelA);
					setDivNodes(posA,  1024,   512,  1023,  9, outsA, permSelA);
					setDivNodes(posA,  2048,  1024,  2047, 10, outsA, permSelA);
					setDivNodes(posA,  4096,  2048,  4095, 11, outsA, permSelA);		
					setDivNodes(posA,  8192,  4096,  8191, 12, outsA, permSelA);
					setDivNodes(posA, 16384,  8192, 16383, 13, outsA, permSelA);
					setDivNodes(posA, 32768, 16384, 32767, 14, outsA, permSelA);
					setDivNodes(posA, 65536, 32768, 65535, 15, outsA, permSelA);	
				}
				if (isSameSub(2, 1)) {
					setDivNodes(posA,     2,     1,     2,  0, outsA, permSelA);
					setDivNodes(posA,     4,     2,     3,  1, outsA, permSelA);
					setDivNodes(posA,     6,     3,     5,  2, outsA, permSelA);
					setDivNodes(posA,     8,     4,     7,  3, outsA, permSelA);
					setDivNodes(posA,    10,     5,     9,  4, outsA, permSelA);
					setDivNodes(posA,    12,     6,    11,  5, outsA, permSelA);
					setDivNodes(posA,    14,     7,    13,  6, outsA, permSelA);
					setDivNodes(posA,    16,     8,    15,  7, outsA, permSelA);
					setDivNodes(posA,    18,     9,    17,  8, outsA, permSelA);
					setDivNodes(posA,    20,    10,    19,  9, outsA, permSelA);
					setDivNodes(posA,    22,    11,    21, 10, outsA, permSelA);
					setDivNodes(posA,    24,    12,    23, 11, outsA, permSelA);		
					setDivNodes(posA,    26,    14,    25, 12, outsA, permSelA);
					setDivNodes(posA,    28,    15,    27, 13, outsA, permSelA);
					setDivNodes(posA,    30,    16,    29, 14, outsA, permSelA);
					setDivNodes(posA,    32,    17,    31, 15, outsA, permSelA);
				}
				if (isSameSub(2, 2)) {
					setDivNodes(posA,     2,     0,     0,  0, outsA, permSelA);
					setDivNodes(posA,     4,     0,     0,  1, outsA, permSelA);
					setDivNodes(posA,     8,     0,     0,  2, outsA, permSelA);
					setDivNodes(posA,    16,     0,     0,  3, outsA, permSelA);
					setDivNodes(posA,    32,     0,     0,  4, outsA, permSelA);
					setDivNodes(posA,    64,     0,     0,  5, outsA, permSelA);
					setDivNodes(posA,   128,     0,     0,  6, outsA, permSelA);
					setDivNodes(posA,   256,     0,     0,  7, outsA, permSelA);
					setDivNodes(posA,   512,     0,     0,  8, outsA, permSelA);
					setDivNodes(posA,  1024,     0,     0,  9, outsA, permSelA);
					setDivNodes(posA,  2048,     0,     0, 10, outsA, permSelA);
					setDivNodes(posA,  4096,     0,     0, 11, outsA, permSelA);		
					setDivNodes(posA,  8192,     0,     0, 12, outsA, permSelA);
					setDivNodes(posA, 16384,     0,     0, 13, outsA, permSelA);
					setDivNodes(posA, 32768,     0,     0, 14, outsA, permSelA);
					setDivNodes(posA, 65536,     0,     0, 15, outsA, permSelA);	
				}
				if (isSameSub(2, 3)) {
					setDivNodes(posA,     6,     3,     5,  0, outsA, permSelA);
					setDivNodes(posA,    12,     6,    11,  1, outsA, permSelA);
					setDivNodes(posA,    18,    12,    17,  2, outsA, permSelA);
					setDivNodes(posA,    24,    18,    23,  3, outsA, permSelA);
					setDivNodes(posA,    30,    24,    29,  4, outsA, permSelA);
					setDivNodes(posA,    36,    30,    35,  5, outsA, permSelA);
					setDivNodes(posA,    42,    36,    41,  6, outsA, permSelA);
					setDivNodes(posA,    48,    42,    47,  7, outsA, permSelA);
					setDivNodes(posA,    54,    48,    53,  8, outsA, permSelA);
					setDivNodes(posA,    60,    54,    59,  9, outsA, permSelA);
					setDivNodes(posA,    66,    60,    65, 10, outsA, permSelA);
					setDivNodes(posA,    72,    66,    71, 11, outsA, permSelA);		
					setDivNodes(posA,    78,    72,    77, 12, outsA, permSelA);
					setDivNodes(posA,    84,    78,    83, 13, outsA, permSelA);
					setDivNodes(posA,    90,    84,    89, 14, outsA, permSelA);
					setDivNodes(posA,    96,    90,    95, 15, outsA, permSelA);
				}		
			}			
		}
	}

	void updateRipple() {	
		if (isSameSub(2, 0)) updateRippleCount(65536, &posA);
		if (isSameSub(2, 1)) updateRippleCount(32, &posA);
		if (isSameSub(2, 2)) updateRippleCount(65536, &posA);
		if (isSameSub(2, 3)) updateRippleCount(96, &posA);

		if (modeSelA != modeSelB || subSelA != subSelB) {
			if (modeSelA == 2) {
				if (subSelA == 0) updateRippleCount(256, &posA);
				if (subSelA == 1) updateRippleCount(16, &posA);
				if (subSelA == 2) updateRippleCount(256, &posA);
				if (subSelA == 3) updateRippleCount(48, &posA);
			}
			if (modeSelB == 2) {
				if (subSelB == 0) updateRippleCount(256, &posB);
				if (subSelB == 1) updateRippleCount(16, &posB);
				if (subSelB == 2) updateRippleCount(256, &posB);
				if (subSelB == 3) updateRippleCount(48, &posB);
			}
		}	
	}

	/* SUB ----------------------------------------------------------- */	

	void updateRippleCount(long length, long *seqPos) {
		if (userMode == 2) {
			if (buttons.hasChanged(inc))
				*seqPos += 1;
		}
		if (nodes[inc].extHasChanged()) 
			*seqPos += 1;

		wrapAround(&*seqPos, length);
	}


//-----------------------------------------------------------------------------------------
//   BINARIES
//-----------------------------------------------------------------------------------------
	void outputBinary() {	
		if (modeSelA == 3 || modeSelB == 3) {
			if (modeSelA != modeSelB || subSelA != subSelB) {
				if (modeSelA == 3) {
					if (subSelA == 0) {

					}
					if (subSelA == 1) {

					}
					if (subSelA == 2) {

					}
					if (subSelA == 3) {

					}
	
				}
				if (modeSelB == 3) {
					if (subSelB == 0) {

					}
					if (subSelB == 1) {

					}
					if (subSelB == 2) {

					}
					if (subSelB == 3) {

					}
				}			
			}
			else {
				if (isSameSub(3, 0)) {

				}
				if (isSameSub(3, 1)) {

				}
				if (isSameSub(3, 2)) {
	
				}
				if (isSameSub(3, 3)) {

				}		
			}			
		}
	}

	void updateBinary() {	
		if (isSameSub(3, 0)) updateBinaryCount();
		if (isSameSub(3, 1)) updateBinaryCount();
		if (isSameSub(3, 2)) updateBinaryCount();
		if (isSameSub(3, 3)) updateBinaryCount();

		if (modeSelA != modeSelB || subSelA != subSelB) {
			if (modeSelA == 3) {
				if (subSelA == 0) updateBinaryCount();
				if (subSelA == 1) updateBinaryCount();
				if (subSelA == 2) updateBinaryCount();
				if (subSelA == 3) updateBinaryCount();
			}
			if (modeSelB == 3) {
				if (subSelB == 0) updateBinaryCount();
				if (subSelB == 1) updateBinaryCount();
				if (subSelB == 2) updateBinaryCount();
				if (subSelB == 3) updateBinaryCount();
			}
		}	
	}

	/* SUB ----------------------------------------------------------- */	

	void updateBinaryCount() {

	}

//-----------------------------------------------------------------------------------------
//   MODULE CONTROLS AND SETTINGS
//-----------------------------------------------------------------------------------------

	/* RESET, HOLD, PAUSE, AND CLEAR ------------------------- */

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

	void checkReset() {
		if (nodes[reset].extRisingEdge()) {
			resetSeq();
			resetOutputs();
		}
	}

	void checkHoldRelease() {
		if (nodes[hold].extFallingEdge() && !isPaused()) 
			resetOutputs();	
	}

	void checkPauseRelease() {
		if (nodes[pause].extFallingEdge() && !isHeld()) 
			resetOutputs();		
	}

	void checkClear() {
		if (nodes[clear].extRisingEdge())
			clearOutputs();
		if (nodes[clear].extFallingEdge()) 
			resetOutputs();	
	}	


	/* MANAGEMENT -------------------------------------------- */
	
	void updateButtons() {
		detectPress(&nodes[reset], reset);
		detectPress(&nodes[hold], hold);	
		detectPress(&nodes[pause], pause);
		detectPress(&nodes[clear], clear);			
	}	

	void resetSeq() {
		posA = 0;
		posB = 0;	
		directionA = 1;
		directionB = 1;	
		unshuffle(deckA, 16);
		unshuffle(deckB, 8);
	}	

	void resetOutputs() {
		clearOutputs();

		if (modeSelA == 0) 
			nodes[outsA[cPerms[permSelA][posA]]].intState = 1;

		if (modeSelA != modeSelB || subSelA != subSelB) 
			if (modeSelB == 0) 
				nodes[outsB[cPerms[permSelB][posB]]].intState = 1;
	
	}	

	void clearOutputs() {
		for (int i = 0; i < 16; i++) 
			nodes[outsA[i]].intState = 0;
	}


	/* PERFORMANCE SELECTION --------------------------------- */

	bool inputDetected() {
		if (nodes[inc].extRisingEdge() || nodes[dec].extRisingEdge() || buttons.risingEdge(inc) || buttons.risingEdge(dec))
			return true;
		else
			return false;
	}

	bool isSameSub(int mode, int sub) {
		if (modeSelA == mode && modeSelB == mode && subSelA == sub && subSelB == sub)
			return true;
		else
			return false;
	}	

	void changeMode() {
		for (int i = 0; i < 4; i++) {
			detectChangeMode(&modeSelA, i, outsA);
			detectChangeMode(&subSelA,  i+4, outsA);
			detectChangeMode(&modeSelB, i, outsB);
			detectChangeMode(&subSelB,  i+4, outsB);
		}
	}

	void detectChangeMode(byte *mode, int index, int outs[]) {
		if (buttons.risingEdge(outs[index])) {
			if (index > 3) {
				*mode = index-4;
			}
			else {
				*mode = index;
			}
			resetSeq();
			resetOutputs();
		}
	}

	void changeAux() {			
		for (int i = 0; i < 8; i++) {
			if (buttons.risingEdge(outsA[i])) {
				auxSelA = i;
				resetOutputs();
			}
			if (buttons.risingEdge(outsB[i])) {
				auxSelB = i;	
				resetOutputs();	
			}
		}	
	}

	void changePerm() {			
		for (int i = 0; i < 8; i++) {
			if (buttons.risingEdge(outsA[i])) {
				permSelA = i;
				resetOutputs();
			}
			if (buttons.risingEdge(outsB[i])) {
				permSelB = i;	
				resetOutputs();	
			}
		}	
	}	

	void displayMode() {
		leds.drawPixel(nodes[outsA[modeSelA]].ledx, nodes[outsA[modeSelA]].ledy, 1);
		leds.drawPixel(nodes[outsA[subSelA+4]].ledx, nodes[outsA[subSelA+4]].ledy, 1);
		leds.drawPixel(nodes[outsB[modeSelB]].ledx, nodes[outsB[modeSelB]].ledy, 1);
		leds.drawPixel(nodes[outsB[subSelB+4]].ledx, nodes[outsB[subSelB+4]].ledy, 1);
	}	

	void displayAux() {
		leds.drawPixel(nodes[outsA[auxSelA]].ledx, nodes[outsA[auxSelA]].ledy, 1);
		leds.drawPixel(nodes[outsB[auxSelB]].ledx, nodes[outsB[auxSelB]].ledy, 1);
	}	

	void displayPerm() {
		leds.drawPixel(nodes[outsA[permSelA]].ledx, nodes[outsA[permSelA]].ledy, 1);
		leds.drawPixel(nodes[outsB[permSelB]].ledx, nodes[outsB[permSelB]].ledy, 1);
	}

	/* CHECK TO SEE IF THE NODE HAS BEEN LINKED -------------- */

	void linked(int nodeR) {
		if (nodeR == inc) incPatched = 1;
	}

	void unlinked(int nodeR) {
		if (nodeR == inc) incPatched = 0;
	}

};


//-----------------------------------------------------------------------------------------
//   INITIALIZE STRUCTS
//-----------------------------------------------------------------------------------------

counter counterBlue = {
	138, 147, 7, 159, 146, 135, 145,
	{127, 123, 137, 143, 131, 151, 139 ,153, 125, 126, 122, 121, 155, 130, 129, 154},
	{125, 126, 122, 121, 155, 130, 129, 154},
};

counter counterRed = {
	134, 120, 14, 133, 136, 140, 152,
	{156, 149, 150, 128, 132, 158, 142, 141, 157, 124, 144, 148,  88,  96, 101, 102},
	{157, 124, 144, 148,  88,  96, 101, 102}
};

counter counterGreen = {
	110, 116, 13, 108, 80, 92, 109,
	{ 93, 112, 100, 104,  84, 117, 118,  94, 304, 304, 304, 304, 304, 304, 304, 304},
	{304, 304, 304, 304, 304, 304, 304, 304}
};