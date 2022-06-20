// Basic useful --------------------------------------------------------------------------------------

const int Review = 1;
const int reviewNode = 90;

const int Patch = 0;
const int patchNode = 91;

const int Play = 2;
const int playNode = 92;

// For boolean matrix implementation: ----------------------------------------------------------------

const int bit_density = 64;
const uint64_t one  = 1;
const uint64_t zero = 0;

typedef int bit_index;

struct bmat{
    uint64_t* bmatrix_data;
    int rows;
    int cols;
    int number_of_chunks;
};

struct data_index{
    int c_idx; int d_idx;
};

struct macro_index {
    int row; int col;
};



// For linked list implementation: -------------------------------------------------------------------
// A struct for keeping a node index in a bucket along with the pointer for the next bucket.
struct bucket {
    int node_idx;
    struct bucket* next;
};

// Start a linked list
struct bucket* makeBucket(int node_idx);
struct bucket* makeBucket(int node_idx){
    bucket* head = NULL;
    head = (bucket*) malloc(sizeof(bucket));

    head->node_idx = node_idx;
    head->next = NULL;

    return(head);
}

// Add a bucket to the tail of the linked list
void push(bucket* head, int node_idx);
void push(bucket* head, int node_idx) {
    // Are we at the tail?  If not, traverse to tail.
    bucket* current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    // Allocate new tail in memory
    current->next = (bucket*) malloc(sizeof(bucket));
    current->next->node_idx = node_idx;
    current->next->next = NULL;
}

// Retrieve + remove bucket from the head of the linked list
int pop(bucket** head);
int pop(bucket** head) {
    int result = -1;
    bucket* next = NULL;

    if (*head != NULL) {
        next = (*head)->next;
        result = (*head)->node_idx;
        free(*head);
        *head = next;
    }
    
    return(result);
}
