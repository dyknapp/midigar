//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

const int linksMax = 128;

struct link {
    int sender;
    int receiver;
};

link links[linksMax];
link undoBuffer = {999, 999};
int linkCount = 0;

int activeNode = 999;


//---------------------------------------------------------------
//   MAIN FUNCTIONS
//---------------------------------------------------------------

void linksInit() {
    for (int i = 0; i < linksMax; i++)
        links[i] = {999, 999};
}


void patch() {
    for (int i = 0; i < buttonsMax; i++) {
            
        // Set an Active Node --------------------------------------------------------

        if (activeNode == 999 && buttons.risingEdge(i) && nodes[i].type != 0)
            activeNode = i;  
        else if (activeNode != 999 && buttons.fallingEdge(activeNode)) 
            activeNode = 999;

        // When Active Node is Set ---------------------------------------------------

        if (activeNode != 999) {
            if (buttons.risingEdge(i)) {
                if (isNodeOutput(activeNode) && isNodeInput(i)) {
                    if (linkExists(activeNode, i)) 
                        removeLink(activeNode, i);
                    else
                        addLink(activeNode, i);  
                }
                else if (isNodeOutput(i) && isNodeInput(activeNode)) {
                    if (linkExists(i, activeNode)) 
                        removeLink(i, activeNode);
                    else
                        addLink(i, activeNode);
                }           
            }
        }  
    }     
}


//---------------------------------------------------------------
//   CHECK FUNCTIONS
//---------------------------------------------------------------

bool isNodeInput(int index) {
    if (nodes[index].type == 1 || nodes[index].type == 2)
        return true;
    else return false;
}

bool isNodeOutput(int index) {
    if (nodes[index].type == 3)
        return true;
    else return false;
}

bool linkExists(int nodeS, int nodeR) {
    for (int i = 0; i < linkCount; i++)            
        if (links[i].sender == nodeS && links[i].receiver == nodeR) 
            return true;
    return false;
}


//---------------------------------------------------------------
//   MAKE AND UNMAKE LINK FUNCTIONS
//---------------------------------------------------------------

void addLink(int nodeS, int nodeR) {
    counterBlue.linked(nodeR);
    counterRed.linked(nodeR);
    counterGreen.linked(nodeR);
    
    links[linkCount].sender   = nodeS;
    links[linkCount].receiver = nodeR;

    linkCount++;
    screenLinkCount();

    if (linkCount >= linksMax) {      
        linkCount = 127;
        screenPrint("LMAX");
    }
}

void removeLink(int nodeS, int nodeR) {
    counterBlue.unlinked(nodeR);
    counterRed.unlinked(nodeR);
    counterGreen.unlinked(nodeR);

    nodes[nodeR].intState = 0; 
    int indexToRemove = 999;

    for (int i = 0; i < linkCount; i++) 
        if (links[i].sender == nodeS && links[i].receiver == nodeR)  
            indexToRemove = i;   

    for (int i=indexToRemove; i < linksMax; i++) {     
        if (i == linksMax - 1) {
            links[i].sender   = 999;
            links[i].receiver = 999;
        }
        else 
            links[i] = links[i+1];
    }     
    linkCount--;
    screenLinkCount();
}


//---------------------------------------------------------------
//   DISPLAY FUNCTIONS
//---------------------------------------------------------------

void screenLinkCount() {
    screenPrint(linkCount);
    screen.writeDigitAscii(0, 'L');
    screen.writeDisplay();
}


//---------------------------------------------------------------
//   UPDATE STATES FUNCTIONS
//---------------------------------------------------------------

void linkStateUpdate() {
    for (int i = 0; i < linkCount; i++) {
        byte x = nodes[links[i].receiver].ledx;
        byte y = nodes[links[i].receiver].ledy;
        bool senderState = nodes[links[i].sender].extState;

        if (senderState == 1) {
            nodes[links[i].receiver].intState = 1;       
        }
        else if (senderState == 0) {
            if (isAdjusterNode(links[i].receiver)) {
                nodes[links[i].receiver].intState = orGate(i);
            }
            else if (buttons.vals[links[i].receiver] != 1)  {
                nodes[links[i].receiver].intState = orGate(i);
            }
        }
        leds.drawPixel(x, y, nodes[links[i].receiver].intState);
    }
}

bool orGate(int index) {
    for (int i = 0; i < index; i++) 
        if (links[i].receiver == links[index].receiver) 
            if (nodes[links[i].sender].extState == 1)
                return 1;
    return 0;
}

bool isAdjusterNode(int r) {
    if (r == 113 || r == 115 || r == 43 || r == 75 || r == 66 || r == 71 || r == 26 || r == 17 || r == 19 || r == 31 || r == 9)
        return true;
    else
        return false;
}
