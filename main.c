#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SYM_TABLE_SIZE 64   // Size of the symbol table

/* -------Structure of the Binary Search Tree node------- */
typedef struct BSTNode {
    bool tiePass;                 // If the word is still valid based on current constraints
    struct BSTNode *left, *right;// Pointers for BST structure
    struct BSTNode *next, *nextPass; // Linked list for quick traversal and tiePass checks
    char word[];                 // Word stored in this node (flexible array member)
} BSTNode;

/* -----------------Structure of a symbol---------------- */
typedef struct {
    char ch;                     // The character
    bool contains;              // True if this symbol is known to be in the target word
    bool *mustBeIn;             // Array marking positions the symbol must be in
    bool *mustNotBeIn;          // Array marking positions the symbol must not be in
    int currTimes;              // Count of current occurrences of this character in a guess
    int times;                  // Max number of occurrences seen so far
    bool exactTimes;            // True if the symbol must appear an exact number of times
} symbol;

/* -------------------Global variables------------------- */
int wordLength;                  // Length of the words in the game
int wordCount = 0;               // Number of words remaining in the dictionary
int totalWordCount;              // Total number of words ever added
int attempts = 0;                // Number of attempts allowed in a game

symbol *symTable[SYM_TABLE_SIZE]; // Symbol table for constraint management
char *input;                     // Input buffer for commands/words
char *result;                    // Result string of word comparison
bool started = false;            // If the game has started

BSTNode *root = NULL;            // Root of the BST for word storage
BSTNode *lastOfTheList;          // Last node in linked list traversal
BSTNode *firstToPass;            // First node that passed all constraints
BSTNode *lastToPass;             // Last node that passed all constraints
BSTNode *lastChecked;            // Last node checked in constraint filtering

/* ----------------Function prototypes------------------- */
void executeCommand();
void addToDictionary();
bool contains(const char *string, char c);

/* ------------------Faster string comparison functions------------------- */

// Compare if string a is lexicographically less than b
bool isLessThan(const char *a, const char *b){
    const unsigned char *wa = (const unsigned char *)a;
    const unsigned char *wb = (const unsigned char *)b;
    for(int i=0; i<wordLength; i++){
        if(*wb > *wa) return true;
        if(*wa > *wb) return false;
        wa++;
        wb++;
    }
    return false;
}

// Check if strings a and b are equal
bool isEqual(const char *a, const char *b){
    const unsigned char *wa = (const unsigned char *)a;
    const unsigned char *wb = (const unsigned char *)b;
    for(int i=0; i<wordLength; i++){
        if(*wb != *wa) return false;
        wa++;
        wb++;
    }
    return true;
}

/* -------------Binary Search Tree Functions----------- */

// Initialize a new BST node from current input
BSTNode *BSTInit(){
    BSTNode *newNode = (BSTNode*)malloc(sizeof(BSTNode) + wordLength + 1);
    for(int i=0; i<wordLength; i++) newNode->word[i] = input[i];
    newNode->word[wordLength] = '\0';
    newNode->next = NULL;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->tiePass = true;
    newNode->nextPass = NULL;

    // Append to linked list
    if(lastOfTheList == NULL){
        lastOfTheList = newNode;
        lastToPass = lastOfTheList;
    } else {
        lastOfTheList->next = newNode;
        lastToPass->nextPass = newNode;
        lastOfTheList = newNode;
        lastToPass = newNode;
    }

    return newNode;
}

// Insert a new node into the BST
BSTNode *BSTInsert(BSTNode *node){
    if(node == NULL){
        return BSTInit();
    }
    if(isLessThan(input, node->word)) {
        node->left = BSTInsert(node->left);
    } else {
        node->right = BSTInsert(node->right);
    }
    return node;
}

// Print all valid words in BST in-order
void BSTPrintInOrder(BSTNode *node){
    if(node != NULL){
        BSTPrintInOrder(node->left);
        if(node->tiePass){
            puts(node->word);
        }
        BSTPrintInOrder(node->right);
    }
}

// Reset the BST state before a new match
void BSTReset(){
    if(root != NULL) {
        BSTNode *temp = root;
        while (temp != NULL) {
            temp->tiePass = true;
            temp->nextPass = temp->next;
            temp = temp->next;
        }
        lastChecked = NULL;
        lastToPass = lastOfTheList;
        firstToPass = root;
    }
}

// Search for a word in the BST
BSTNode *BSTSearch(BSTNode *node){
    if(node == NULL || isEqual(node->word, input)) return node;
    if(isLessThan(node->word, input)) return BSTSearch(node->right);
    return BSTSearch(node->left);
}

/* ----------------Symbol Table Functions---------------- */

// Reset the symbol table
void resetSymTable(){
    for(int i=0; i<SYM_TABLE_SIZE; i++){
        if(symTable[i] != NULL){
            free(symTable[i]->mustBeIn);
            free(symTable[i]->mustNotBeIn);
            free(symTable[i]);
            symTable[i] = NULL;
        }
    }
}

// Hash function to get index for character c
int symHash(char c) {
    int ascii = (int)c;
    switch (ascii) {
        case 45: return 0;   // '-'
        case 95: return 1;   // '_'
        default:
            if (ascii >= 48 && ascii <= 57) return ascii - 46; // 0-9
            else if (ascii >= 65 && ascii <= 90) return ascii - 53; // A-Z
            else return ascii - 59; // a-z
    }
}

// Retrieve symbol from symbol table
symbol *symHashSearch(char c){
    int index = symHash(c);
    return symTable[index];
}

/* -----------------Symbol utilities------------------- */

// Initialize a symbol in the table
void initSymbol(char c){
    symbol *sym = (symbol*)malloc(sizeof(symbol));
    symTable[symHash(c)] = sym;
    sym->ch = c;
    sym->times = 0;
    sym->currTimes = 0;
    sym->exactTimes = false;
    sym->mustBeIn = NULL;
    sym->mustNotBeIn = NULL;
}

// Mark a symbol must appear at a position
void mustBeIn(char c, int position){
    symbol *sym = symHashSearch(c);
    if(sym == NULL){
        initSymbol(c);
        sym = symHashSearch(c);
    }
    sym->contains = true;
    if(sym->mustBeIn == NULL){
        sym->mustBeIn = (bool*)malloc(wordLength * sizeof(bool));
        for(int i=0; i<wordLength; i++) sym->mustBeIn[i] = false;
    }
    sym->mustBeIn[position] = true;
}

// Mark a symbol must not be at a position
void mustNotBeIn(char c, int position){
    symbol *sym = symHashSearch(c);
    if(sym == NULL){
        initSymbol(c);
        sym = symHashSearch(c);
    }
    sym->contains = true;
    if(sym->mustNotBeIn == NULL){
        sym->mustNotBeIn = (bool*)malloc(wordLength * sizeof(bool));
        for(int i=0; i<wordLength; i++) sym->mustNotBeIn[i] = false;
    }
    sym->mustNotBeIn[position] = true;
}

// Increase the occurrence counter for a symbol
void increaseSymCount(char c){
    symbol *sym = symHashSearch(c);
    if(!sym->exactTimes){
        sym->currTimes++;
        if(sym->currTimes > sym->times) sym->times = sym->currTimes;
    }
}

// Reset symbol occurrence count
void resetCurrTimes(char c){
    symbol *sym = symHashSearch(c);
    if(sym != NULL) sym->currTimes = 0;
}

// Check if character is in string
bool contains(const char *string, char c){
    for(int i=0; i<wordLength; i++){
        if(string[i] == c) return true;
    }
    return false;
}

// Set contains flag for symbol
void setContains(char c, bool val){
    symbol *sym = symHashSearch(c);
    if(sym == NULL){
        initSymbol(c);
        sym = symHashSearch(c);
    }
    sym->contains = val;
}

// Set exact times flag for symbol
void setExactTimes(char c, bool val){
    symbol *sym = symHashSearch(c);
    if(sym == NULL){
        initSymbol(c);
        sym = symHashSearch(c);
    }
    sym->exactTimes = val;
}

// Filter words that do not meet current symbol constraints
void tiePassCheck(){
    symbol *sym;
    for (int i = 0; i < SYM_TABLE_SIZE; i++) {
        sym = symTable[i];
        if (sym != NULL) {
            BSTNode *temp = firstToPass;
            if (!sym->contains) {
                while(temp != NULL) {
                    if (contains(temp->word, sym->ch)) {
                        temp->tiePass = false;
                        if(temp == firstToPass){
                            lastChecked = NULL;
                            firstToPass = temp->nextPass;
                        } else if(temp == lastToPass){
                            lastToPass = lastChecked;
                            if(lastToPass != NULL) lastToPass->nextPass = NULL;
                        } else {
                            if(lastChecked != NULL) lastChecked->nextPass = temp->nextPass;
                        }
                        wordCount--;
                    } else {
                        lastChecked = temp;
                    }
                    temp = temp->nextPass;
                }
            } else {
                while(temp != NULL){
                    bool pass = true;
                    int currCount = 0;
                    for (int j = 0; j < wordLength; j++) {
                        if (sym->mustBeIn && sym->mustBeIn[j] && temp->word[j] != sym->ch) {
                            pass = false;
                            break;
                        }
                        if (sym->mustNotBeIn && sym->mustNotBeIn[j] && temp->word[j] == sym->ch) {
                            pass = false;
                            break;
                        }
                        if (sym->ch == temp->word[j]) currCount++;
                    }
                    if (pass) {
                        if (sym->exactTimes && sym->times != currCount) pass = false;
                        else if (!sym->exactTimes && currCount < sym->times) pass = false;
                    }
                    if (!pass) {
                        temp->tiePass = false;
                        wordCount--;
                        if(temp == firstToPass){
                            lastChecked = NULL;
                            firstToPass = temp->nextPass;
                        } else if(temp == lastToPass){
                            lastToPass = lastChecked;
                            if(lastToPass != NULL) lastToPass->nextPass = NULL;
                        } else {
                            if(lastChecked != NULL) lastChecked->nextPass = temp->nextPass;
                        }
                    } else {
                        lastChecked = temp;
                    }
                    temp = temp->nextPass;
                }
            }
        }
    }
}

/* ----------------Word comparison logic---------------- */

// Compare guess with the target word
void compareWords(const char * r, const char * s) {
    bool used[wordLength];
    for (int i = 0; i < wordLength; i++){
        result[i] = '/';
        resetCurrTimes(s[i]);
        used[i] = false;
    }
    result[wordLength] = '\0';
    int rightCount = 0;

    // First pass: check correct positions
    for (int i = 0; i < wordLength; i++) {
        if (s[i] == r[i]) {
            result[i] = '+';
            used[i] = true;
            rightCount++;
            mustBeIn(s[i], i);
            increaseSymCount(s[i]);
        }
    }

    // If word is fully correct
    if (rightCount == wordLength) {
        result[0] = 'o';
        result[1] = 'k';
        for(int i = 2; i < wordLength; i++) result[i] = '\0';
        return;
    }

    // Second pass: check correct letters in wrong positions
    for (int i = 0; i < wordLength; i++) {
        if (result[i] == '/') {
            for (int j = 0; j < wordLength; j++) {
                if (s[i] == r[j] && !used[j]) {
                    result[i] = '|';
                    mustNotBeIn(s[i], i);
                    increaseSymCount(s[i]);
                    used[j] = true;
                    break;
                }
            }
        }
    }

    // Third pass: process letters not found at all
    for(int i = 0; i < wordLength; i++){
        if(result[i] == '/'){
            symbol *sym = symHashSearch(s[i]);
            if(sym != NULL && sym->currTimes > 0){
                setExactTimes(s[i], true);
                mustNotBeIn(s[i], i);
            } else {
                if(sym == NULL) setContains(s[i], false);
                else if(!sym->contains) setContains(s[i], false);
                else mustNotBeIn(s[i], i);
            }
        }
    }
}

/* ----------------Game Control Functions---------------- */

// Start a new match/game
void startNewMatch(){
    started = true;
    bool found = false;
    char toBeFound[wordLength];
    char endInput = 0;
    BSTNode *searched;

    BSTReset();
    resetSymTable();
    wordCount = totalWordCount;

    while(getchar() != '\n') continue; // skip to word
    for(int i = 0; i < wordLength; i++) toBeFound[i] = getchar();
    scanf("%d\n", &attempts); // read number of attempts

    while (attempts > 0 && !found) {
        input[0] = getchar();
        if(input[0] == '+') {
            executeCommand();
        } else if(input[0] == EOF) break;
        else {
            for (int j = 1; j < wordLength; j++) input[j] = getchar();
            input[wordLength] = '\0';
            searched = BSTSearch(root);
            if(searched == NULL){
                printf("not_exists\n");
                attempts++;
            } else {
                compareWords(toBeFound, searched->word);
                puts(result);
                if(!(result[0] == 'o' || result[0] == 'k')) {
                    tiePassCheck();
                    printf("%d\n", wordCount);
                } else {
                    found = true;
                    break;
                }
            }
            getchar();
            attempts--;
        }
    }

    if(!found && attempts == 0) {
        printf("ko\n");
    }

    while(endInput != EOF) {
        endInput = getchar();
        if(endInput == '+') {
            executeCommand();
        }
    }
}

// Handle input commands
void executeCommand(){
    char command[16];
    command[0] = getchar();
    if(command[0] == 'n') startNewMatch();
    else if(command[0] == 'i') {
        for(int i=0;i<9;i++) command[i] = getchar();
        if(getchar() == 'i') {
            for(int i=0;i<5;i++) getchar(); // Skip "nput"
            addToDictionary();
        } else {
            if(started) for(int i=0;i<4;i++) getchar();
            else {
                for(int i=0;i<3;i++) getchar();
                addToDictionary();
            }
        }
    } else if(command[0] == 's') {
        tiePassCheck();
        BSTPrintInOrder(root);
        while(getchar() != '\n') continue;
    }
}

// Add words to the dictionary
void addToDictionary(){
    while (1) {
        getchar();
        input[0] = getchar();
        if(input[0] == '+') break;
        for (int j = 1; j < wordLength; j++) input[j] = getchar();
        input[wordLength] = '\0';
        root = BSTInsert(root);
        wordCount++;
        totalWordCount++;
    }
    executeCommand();
}

// Entry point
int main(void){
    for(int i=0; i<SYM_TABLE_SIZE; i++) symTable[i] = NULL;
    scanf("%d", &wordLength);
    wordCount = 0;
    totalWordCount = 0;
    input = malloc(wordLength * sizeof(char*));
    result = malloc(wordLength * sizeof(char*));
    addToDictionary();
    return 0;
}
