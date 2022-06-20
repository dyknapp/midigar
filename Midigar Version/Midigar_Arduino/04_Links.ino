//---------------------------------------------------------------
//   VARIABLES
//---------------------------------------------------------------

// Keeping track of links.
bmat all_links;
macro_index current_bit;

// Keeping track of button presses
bucket* next;
int numberInputs;
int numberOutputs;
int savedResult;
int connections;
int savedInputs = 0;
int savedOutputs = 0;
String printString;

//---------------------------------------------------------------
//   MAIN FUNCTIONS
//---------------------------------------------------------------

void linksInitialize(){
  // TODO:  A lot of memory can be saved by making this inputs x outputs or so forth, not that we really need to worry about it.
  // Use dictionary
  all_links = new_bmatrix(144, 144);
}

void patch(){
  // Patching process:
  // 1. Find all pressed buttons, push to linked list.
  // 2. Convert linked list to array.
  // 3. Every simultaneously pushed input and output are linked to each other using nested for loops iterating over the array.
  
  // Start a linked list to keep track of inputs that are on.
  bucket* onInputs = NULL;
  
  // Iterate over all buttons.
  for (int i = 0; i < totalNodes; i++){
    
    // If it's an input that's pressed...
    // Once we only iterate over inputs, we don't need to check the type -> TODO
    if ((nodes[i].physicalState == 1 || nodes[i].physicalFallingEdge()) && (nodes[i].type == 1 || nodes[i].type == 3)){

      // If there haven't been any recorded inputs yet, allocate an actual spot in memory for that.
      if(onInputs == NULL){
        onInputs = makeBucket(i);
        next = onInputs;
      }
      
      // If there are already recorded inputs, push them to the end of the list.
      // Variable "next" keeps track of the end of the list, so that pushing occurs in constant time.
      else{
        push(next, i);        
        // Keep track of the tail
        next = next->next;
      }

      // Keep track of how many inputs and outputs were pressed.
      if(nodes[i].type == 1){
        numberInputs++;
      }
      else{
        numberOutputs++;
      }
    }
  }

  // Below, all string-related commands are for debug
  printString = " (" + (String)numberInputs + "," + (String)numberOutputs + "):";

  // We only need to worry about patching if both inputs and outputs are pressed.
  if(numberInputs > 0 && numberOutputs > 0){
    
    //Initialize arrays to hold the inputs and outputs, for easier iteration
    int  inputArray[ numberInputs];
    int outputArray[numberOutputs];
    
    // Iterate over the linked list from the head.
    savedInputs = 0;
    savedOutputs = 0;
    while (onInputs != NULL){
      // Grab the saved result from the linked list.
      // The node containing the saved result is erased from memory.
      savedResult = pop(&onInputs);
      printString += (String)savedResult + " ";

      // Array conversion.
      if(nodes[savedResult].type == 1){
        inputArray[savedInputs] = savedResult;
        savedInputs++;
      }
      else{
        outputArray[savedOutputs] = savedResult;
        savedOutputs++;
      }
    }
    // Iterate over every input and output combination and link / unlink them.
    for(int i = 0; i < numberInputs; i++){
      for(int o = 0; o < numberOutputs; o++){
        // Toggle the link for the input / output pair on the falling edge.
        current_bit = {inputArray[i], outputArray[o]};
        if(nodes[inputArray[i]].physicalFallingEdge() || nodes[outputArray[o]].physicalFallingEdge()){
          toggle_bit(all_links, current_bit);
        }
      }
    }
  }
  else{
    if((numberInputs == 1 && numberOutputs == 0) || (numberOutputs == 1 && numberInputs == 0)){
      savedResult = pop(&onInputs);
      connections = displayConnections(savedResult);
      printString += (String)connections + " PATCHES @" + (String)savedResult;
    }
    else{
      printString += "NO PAIR";
    }
  }
  // Free memory to avoid memory leak, set counters to zero just in case (although they should be zero anyways.)
  free(onInputs);
  numberInputs = 0;
  numberOutputs = 0;
  
  screenPrint(printString);
}

// Returns number of connections
int displayConnections(int displayNode){
  connections = 0;
  if(nodes[displayNode].type == 1){
    for(int o = 0; o < totalNodes; o++){
      current_bit = {displayNode, o};
      if ((bool)get_bit(all_links, current_bit)){
        nodes[o].displayState = 1;
        connections++;
      }
    }
  }
  else if (nodes[displayNode].type == 3){
    for(int i = 0; i < totalNodes; i++){
      current_bit = {i, displayNode};
      if ((bool)get_bit(all_links, current_bit)){
        nodes[i].displayState = 1;
        connections++;
      }
    }
  }
  return connections;
}

//---------------------------------------------------------------
//   DISPLAY FUNCTIONS
//---------------------------------------------------------------

void screenLinkCount() {

}


//---------------------------------------------------------------
//   UPDATE STATES FUNCTIONS
//---------------------------------------------------------------

void linkStateUpdate() {

}
