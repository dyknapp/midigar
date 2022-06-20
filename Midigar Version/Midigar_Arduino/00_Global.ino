//---------------------------------------------------------------
//   GLOBAL VARIABLES
//---------------------------------------------------------------

const byte totalNodes = 144;
int modeCurrent = 2;            // Start in PLAY Mode

//---------------------------------------------------------------
//   NODES
//---------------------------------------------------------------

struct node {
    const byte ledx;
    const byte ledy;
    const byte type;            // 0 = not patchable, 1 = input, 2 = special input, 3 = output

    bool physicalState;
    bool lastPhysicalState;
    bool internalState;
    bool lastInternalState;
    bool displayState; 
    bool lastDisplayState;

    bool mute;
    bool toggle;
    bool togLock;
    bool invert;

    bool physicalHasChanged()  {
        if (physicalState != lastPhysicalState) 
            return 1;
        else 
            return 0;
    }

    bool physicalRisingEdge()  {
        if (physicalState != lastPhysicalState) 
            if (physicalState == 1) 
                return 1;
            else 
                return 0;
        else
            return 0;
    }

    bool physicalFallingEdge()  {
        if (physicalState != lastPhysicalState) 
            if (physicalState == 0) 
                return 1;
            else 
                return 0;
        else
            return 0;
    }    

    bool internalHasChanged()  {
        if (internalState != lastInternalState) 
            return 1;
        else 
            return 0;
    }

    bool internalRisingEdge()  {
      if (internalState != lastInternalState) 
        if (internalState == 1) 
          return 1;
        else 
          return 0;
      else
          return 0;
    }

    bool internalFallingEdge()  {
      if (internalState != lastInternalState) 
        if (internalState == 0) 
          return 1;
        else 
          return 0;
      else
        return 0;
    }

    bool displayHasChanged()  {
      if (displayState != lastDisplayState) 
        return 1;
      else 
        return 0;
    }

    bool displayRisingEdge()  {
      if (displayState != lastDisplayState) 
        if (displayState == 1) 
          return 1;
        else 
          return 0;
      else
          return 0;
    }

    bool displayFallingEdge()  {
      if (displayState != lastDisplayState) 
        if (displayState == 0) 
          return 1;
        else 
          return 0;
      else
        return 0;
    }
};

node nodes[] {
    {0, 0, 1}, {0, 1, 1}, {0, 2, 1}, {0, 3, 1}, {0, 4, 1}, {0, 5, 1}, { 8, 0, 1}, { 8, 1, 1},      { 8, 2, 4}, { 8, 3, 4}, { 8, 4, 4}, { 8, 5, 4}, {16, 0, 1}, {16, 1, 1}, {16, 2, 1},      {16, 3, 0}, {16, 4, 0}, {16, 5, 0},
    {1, 0, 3}, {1, 1, 3}, {1, 2, 3}, {1, 3, 3}, {1, 4, 3}, {1, 5, 3}, { 9, 0, 3}, { 9, 1, 3},      { 9, 2, 4}, { 9, 3, 4}, { 9, 4, 4}, { 9, 5, 4}, {17, 0, 1}, {17, 1, 1}, {17, 2, 1},      {17, 3, 0}, {17, 4, 0}, {17, 5, 0},
    {2, 0, 1}, {2, 1, 1}, {2, 2, 1}, {2, 3, 1}, {2, 4, 1}, {2, 5, 1}, {10, 0, 1}, {10, 1, 1},      {10, 2, 4}, {10, 3, 4}, {10, 4, 4}, {10, 5, 4},      {18, 0, 1}, {18, 1, 1}, {18, 2, 1}, {18, 3, 3}, {18, 4, 3}, {18, 5, 3},
    {3, 0, 3}, {3, 1, 3}, {3, 2, 3}, {3, 3, 3}, {3, 4, 3}, {3, 5, 3}, {11, 0, 3}, {11, 1, 3},      {11, 2, 4}, {11, 3, 4}, {11, 4, 4}, {11, 5, 4},      {19, 0, 1}, {19, 1, 3}, {19, 2, 3}, {19, 3, 1}, {19, 4, 3}, {19, 5, 3},
    {4, 0, 0}, {4, 1, 0}, {4, 2, 0},      {4, 3, 1}, {4, 4, 1}, {4, 5, 1}, {12, 0, 3}, {12, 1, 3}, {12, 2, 3}, {12, 3, 3}, {12, 4, 1}, {12, 5, 1}, {20, 0, 1},      {20, 1, 1}, {20, 2, 1}, {20, 3, 1}, {20, 4, 3},      {20, 5, 0},                       
    {5, 0, 0}, {5, 1, 0}, {5, 2, 0},      {5, 3, 1}, {5, 4, 1}, {5, 5, 1}, {13, 0, 1}, {13, 1, 1}, {13, 2, 1}, {13, 3, 3}, {13, 4, 1}, {13, 5, 1}, {21, 0, 1},      {21, 1, 1}, {21, 2, 1}, {21, 3, 1}, {21, 4, 3},      {21, 5, 0},
    {6, 0, 0}, {6, 1, 0}, {6, 2, 0},      {6, 3, 0}, {6, 4, 0}, {6, 5, 0}, {14, 0, 0},      {14, 1, 1}, {14, 2, 1}, {14, 3, 1}, {14, 4, 1},      {14, 5, 1}, {22, 0, 1}, {22, 1, 1}, {22, 2, 1},      {22, 3, 1}, {22, 4, 1}, {22, 5, 1},
    {7, 0, 0}, {7, 1, 0}, {7, 2, 0},      {7, 3, 0}, {7, 4, 0}, {7, 5, 0}, {15, 0, 0},      {15, 1, 1}, {15, 2, 1}, {15, 3, 1}, {15, 4, 1}, {15, 5, 1}, {23, 0, 1}, {23, 1, 1}, {23, 2, 1}, {23, 3, 1}, {23, 4, 1}, {23, 5, 1},
};


//---------------------------------------------------------------
//   FUNCTIONS
//---------------------------------------------------------------

void nodesInitialize() {
    for (int i=0; i<totalNodes; i++) {
        nodes[i].physicalState = 0;  
        nodes[i].internalState = 0;
        nodes[i].displayState = 0;
        nodes[i].lastPhysicalState = 0;
        nodes[i].lastInternalState = 0;
        nodes[i].lastDisplayState = 0;
    }
}

void nodesUpdate() {
    for (int i=0; i<totalNodes; i++) {
        nodes[i].lastPhysicalState = nodes[i].physicalState;
        nodes[i].lastInternalState = nodes[i].internalState;
        nodes[i].lastDisplayState = nodes[i].displayState;
    }
}
