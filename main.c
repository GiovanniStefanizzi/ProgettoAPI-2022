#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define TABLE_SIZE 10000
#define SYM_TABLE_SIZE 64

typedef struct wordNode {
    char *word;
    struct wordNode *next;
}wordNode;

typedef struct BSTNode{
    int index;
    int position;
    bool tiePass;
    struct BSTNode *next;
    struct BSTNode *left, *right;
}BSTNode;

typedef struct{
    char ch;
    bool contains;
    bool *mustBeIn;
    bool *mustNotBeIn;
    int currTimes;
    int times;
    bool exactTimes;
}symbol;

int wordLength;
int wordCount = 0;
int totalWordCount;
int attempts = 0;
wordNode *hashTable[TABLE_SIZE];
BSTNode *head = NULL;
symbol *symTable[SYM_TABLE_SIZE];
BSTNode *root=NULL;
char *tempWord;

wordNode *initWordNode(char *);
void executeCommand();
void addToDictionary();
char *searchWordByIndex(int, int);
wordNode *hashSearch(char *);
bool contains(const char *string, char c);


void printOrderedList()
{
    //printf("\n");
    if(head == NULL){
        printf("list is empty");
        return;
    }
    BSTNode *temp = head;
    while(temp!=NULL){
        if(temp->tiePass) {
            //printf("index = %d, position = %d: ", temp->index, temp->position);
            printf("%s\n", searchWordByIndex(temp->index, temp->position));
        }
        temp = temp->next;
    }
    //printf("");
}


bool isLessThan(const char *a, const char *b){
    const unsigned char *wa = (const unsigned char *)a;
    const unsigned char *wb = (const unsigned char *)b;
    for(int i=0;i<wordLength;i++){
        if(*wb > *wa) return true;
        if(*wa > *wb) return false;
        wa++;
        wb++;
    }
    return false;
}



BSTNode *BSTInsert(BSTNode *node, int index, int position){
    BSTNode *newNode;
    if(node==NULL){
        newNode = (BSTNode *) malloc(sizeof(BSTNode));
        newNode->index = index;
        newNode->position = position;
        newNode->tiePass = true;
        newNode->left = NULL;
        newNode->right = NULL;
        return newNode;
    }
    const char *toInsert = searchWordByIndex(index, position);
    const char *inTheNode = searchWordByIndex(node->index, node->position);
    if(isLessThan(toInsert, inTheNode)) {
        node->left =  BSTInsert(node->left, index, position);
    }
    else node->right = BSTInsert(node->right, index, position);
    return node;
}





void BSTPrint(BSTNode *node){
    if(node!=NULL){
        BSTPrint(node->left);
        if(node->tiePass) printf("%s\n", searchWordByIndex(node->index, node->position));
        BSTPrint(node->right);
    }
}

void BSTReset(BSTNode *node){
    if(node!=NULL){
        BSTReset(node->left);
        node->tiePass = true;
        BSTReset(node->right);
    }
}

void BSTRemoveContains(symbol *sym, BSTNode *node){
    if(node != NULL) {
        BSTRemoveContains(sym, node->left);
        if (node->tiePass) {
            tempWord = searchWordByIndex(node->index, node->position);
            //printf("contains = %d for %s, %c\n", contains(tempWord, sym->ch), tempWord, sym->ch );
            if (contains(tempWord, sym->ch)) {
                //printf("contains = %d for %s, %c\n", contains(tempWord, sym->ch), tempWord, sym->ch );
                node->tiePass = false;
                wordCount--;
            }
        }
        BSTRemoveContains(sym, node->right);
    }
}

void BSTCheckNumAndPosition(symbol *sym, BSTNode *node) {
    if(node != NULL){
        BSTCheckNumAndPosition(sym, node->left);
        bool pass = true;
        int currCount = 0;
        if(node->tiePass) {
            pass = true;
            currCount = 0;
            tempWord = searchWordByIndex(node->index, node->position);
            for (int j = 0; j < wordLength; j++) {
                if (sym->mustBeIn != NULL && sym->mustBeIn[j] && tempWord[j] != sym->ch) {
                    pass = false;
                    break;
                }
                if (sym->mustNotBeIn != NULL && sym->mustNotBeIn[j] && tempWord[j] == sym->ch) {
                    pass = false;
                    break;
                }
                if (sym->ch == tempWord[j]) currCount++;
            }
            if (pass) {
                if (sym->exactTimes) {
                    if (sym->times != currCount) pass = false;
                } else {
                    if (currCount < sym->times) pass = false;
                }
            }

            if (!pass) {
                wordCount--;
                node->tiePass = false;
            }
        }
        BSTCheckNumAndPosition(sym, node->right);
    }
}




void resetSymTable(){
    for(int i=0;i<SYM_TABLE_SIZE;i++){
        if(symTable[i] != NULL){
            free(symTable[i]);
            symTable[i] = NULL;
        }
    }
}


void resetOrderedList(){
    if(head != NULL){
        BSTNode *temp = head;
        while(temp != NULL){
            temp->tiePass = true;
            temp = temp -> next;
        }
    }
}

BSTNode *insertInOrder(int index, int position) {
    BSTNode *newNode;
    newNode = (BSTNode *) malloc(sizeof(BSTNode));
    newNode->index = index;
    newNode->position = position;
    newNode->tiePass = true;
    newNode->next = NULL;
    if (head == NULL) {
        return newNode;
    } else {
        if (isLessThan(searchWordByIndex(index, position), searchWordByIndex(head->index, head->position))) {
            newNode->next = head;
            return newNode;
        }
        BSTNode *temp = head;
        while (temp->next != NULL) {
            if (isLessThan(searchWordByIndex(index, position), searchWordByIndex(temp->next->index, temp->next->position))) {
                newNode->next = temp->next;
                temp->next = newNode;
                return head;
            }
            temp = temp->next;
        }
        temp->next=newNode;
        return head;
    }
}

void insertWordNode(wordNode *toInsert, int index){
    wordNode *newNode = initWordNode(toInsert->word);
    int position = 0;
    if (hashTable[index] == NULL){
        //newNode->position = 0;
        hashTable[index] = newNode;
    }
    else{
        position = 1;
        wordNode *temp = hashTable[index];
        while(temp->next != NULL){
            temp = temp->next;
            position ++;
        }
        //newNode->position = position;
        temp->next = newNode;
    }
    root = BSTInsert(root, index, position);
}

wordNode *initWordNode(char word[]){
    wordNode *node = (wordNode*)malloc(sizeof(wordNode));
    if(node == NULL){
        printf("memory error\n");
        exit(1);
    }
    node->word = malloc(wordLength*sizeof(char));
    for(int i=0;i<wordLength;i++){
        node->word[i]=word[i];
    }
    node->word[wordLength] = '\0';
    node->next = NULL;
    return node;
}

void printList(int index)
{
    if(hashTable[index] == NULL){
        printf("list is empty");
        return;
    }
    wordNode *temp = hashTable[index];
    while(temp!=NULL){
        printf("%s\t",temp->word);
        temp = temp->next;
    }
    printf("\n");
}

unsigned long hash(char *word){
    unsigned long hash = 5381;
    int c;
    while((c = *word++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

int symHash(char c){
    int ascii = (int)c;
    if(ascii==45) return 0;
    if(ascii==95) return 1;
    if(ascii>=48 && ascii<=57) return ascii - 46;
    if(ascii>=65 && ascii<=90) return ascii - 53;
    else return ascii - 59;
}

wordNode *hashSearch(char *word){
    int index = hash(word) % TABLE_SIZE;
    wordNode *temp = hashTable[index];
    while(temp!= NULL  && strcmp(word, temp->word) != 0) temp = temp->next;
    return temp;
}

char *searchWordByIndex(int index, int position){
    wordNode *temp = hashTable[index];
    for(int i=0; i<position; i++) temp = temp->next;
    return temp ->word;
}

symbol *symHashSearch(char c){
    int index = symHash(c);
    return symTable[index];
}

void initSymbol(char c){
    symbol *sym;
    sym = (symbol*)malloc(sizeof(symbol));
    symTable[symHash(c)]=sym;
    sym -> ch = c;
    sym -> times = 0;
    sym -> currTimes = 0;
    sym -> exactTimes = false;
    sym ->mustBeIn = NULL;
    sym ->mustNotBeIn = NULL;
}

void mustBeIn(char c, int position){
    symbol *sym;
    sym = symHashSearch(c);
    if(sym == NULL){
        initSymbol(c);
        sym = symHashSearch(c);
    }
    sym->contains = true;
    if(sym->mustBeIn == NULL){
        sym->mustBeIn = (bool*)malloc(wordLength*sizeof(bool));
        for(int i=0;i<wordLength;i++) sym->mustBeIn[i] = false;
    }
    sym->mustBeIn[position] = true;
}

void mustNotBeIn(char c, int position){
    symbol *sym;
    sym = symHashSearch(c);
    if(sym == NULL){
        initSymbol(c);
        sym = symHashSearch(c);
    }
    sym->contains = true;
    if(sym->mustNotBeIn == NULL){
        sym->mustNotBeIn = (bool*)malloc(wordLength*sizeof(bool));
        for(int i=0;i<wordLength;i++) sym->mustNotBeIn[i] = false;
    }
    sym->mustNotBeIn[position] = true;
}

void increaseSymCount(char c){
    symbol *sym;
    sym = symHashSearch(c);
    if(!sym -> exactTimes){
        sym -> currTimes ++;
        if(sym -> currTimes > sym -> times) sym->times = sym -> currTimes;
    }
}

void resetCurrTimes(char c){
    symbol *sym;
    sym = symHashSearch(c);
    if(sym!=NULL){
        sym->currTimes = 0;
    }
}

bool contains(const char *string, char c){
    for(int i=0;i<wordLength;i++){
        if(string[i] == c) return true;
    }
    return false;
}

void setContains(char c, bool val){
    symbol *sym;
    sym = symHashSearch(c);
    if(sym == NULL){
        initSymbol(c);
        sym = symHashSearch(c);
    }
    sym -> contains = val;
}

void setExactTimes(char c, bool val){
    symbol *sym;
    sym = symHashSearch(c);
    if(sym == NULL){
        initSymbol(c);
        sym = symHashSearch(c);
    }
    sym -> exactTimes = val;
}

void tiePassCheck(){
    symbol *sym;
    for(int i=0;i<SYM_TABLE_SIZE;i++){
        sym = symTable[i];
        if(sym != NULL){
            if(!sym -> contains){
                BSTRemoveContains(sym, root);
            }
            else {
                BSTCheckNumAndPosition(sym, root);
            }
        }
    }
}

const char  *compareWords(const char * r, const char * s) {
    char *result;
    result = (char*)malloc(wordLength*sizeof(char));
    for (int i = 0; i < wordLength; i++){
        result[i] = '/';
        resetCurrTimes(s[i]);
    }
    result[wordLength] = '\0';
    //a set of flags indicating if that letter in the answer is used as clue
    bool used[wordLength];
    int rightCount=0;
    //first pass, look for exact matches
    for (int i = 0; i < wordLength; i++) {
        if (s[i] == r[i]) {
            result[i] = '+';
            used[i] = true;
            rightCount++;
            mustBeIn(s[i],i);
            increaseSymCount(s[i]);
        }
    }
    if (rightCount == wordLength) return "ok";
    //second pass, look for there but elsewhere
    for (int i = 0; i < wordLength; i++) {
        if (result[i] == '/') {
            for (int j = 0; j < wordLength; j++) {
                if (s[i] == r[j] && !used[j]) {
                    //a match at another position and has not been used as clue
                    result[i] = '|';
                    mustNotBeIn(s[i], i);
                    increaseSymCount(s[i]);
                    used[j] = true;
                    break; //end this j-loop because we don't want multiple clues from the same letter
                }
            }
        }
    }
    //last pass, adjust ties
    symbol *sym;
    for(int i = 0; i<wordLength; i++){

        if(result[i] == '/'){
            sym = symHashSearch(s[i]);
            if(sym != NULL && sym -> currTimes > 0){
                setExactTimes(s[i], true);
                mustNotBeIn(s[i], i);
            }
            else{
                if(sym==NULL) setContains(s[i], false);
                else if(sym -> contains != true)setContains(s[i], false);
                //if(s[i]=='G')printf("%d cuinonce in %s\n", i, s);
            }
        }
    }
    return result;
}


void startNewMatch(){
    bool found=false;
    char toBeFound[wordLength];
    char input[wordLength];
    char endInput = 0;
    const char *result;
    wordNode *searched;
    BSTReset(root);
    resetSymTable();
    wordCount = totalWordCount;
    while(getchar()!='\n') continue;
    for(int i=0;i<wordLength;i++){
        toBeFound[i] = getchar();
    }
    toBeFound[wordLength] = '\0';
    //printf("to be found = %s\n", toBeFound);
    if(scanf("%d\n", &attempts)>0);
    //printf("attempts = %d\n", attempts);
    while (attempts > 0) {
        result = 0;
        input[0] = getchar();
        if(input[0] == '+'){
            executeCommand();
        }
        else if(input[0]==EOF) break;
        else {
            for (int j = 1; j < wordLength; j++)
                input[j] = getchar();
            input[wordLength] = '\0';
            searched = hashSearch(input);
            //printf("%s\n", input);
            if(searched==NULL){
                printf("not_exists\n");
                attempts++;
            }
            else{
                result = compareWords(toBeFound, searched->word);
                printf("%s\n", result);
                if(result!=NULL && !(result[0]=='o'|| result[0]=='k')) {
                    tiePassCheck();
                    printf("%d\n", wordCount);
                }
                else found=true;
            }
            getchar();
            attempts--;
            if(attempts==0 && !found) {
                printf("ko\n");
            }
        }
    }
    while(endInput!=EOF) {
        endInput = getchar();
        if(endInput=='+'){
            executeCommand();
        }
    }
}

void executeCommand(){
    char command[16];
    command[0] = getchar();
    if(command[0]=='n') {
        //printf("\n\nnuova partita\n");
        startNewMatch();
    }
    if(command[0]=='i') {
        for(int i=0;i<9;i++) command[i] = getchar();
        if(getchar()=='i') {
            //printf("\n\ninserisci inizio");
            for(int i=0;i<5;i++) getchar();
            addToDictionary();
        }
        else{
            //printf("\ninserisci fine\n");
            for(int i=0;i<4;i++) getchar();
        }
    }
    if(command[0]=='s') {
        tiePassCheck();
        BSTPrint(root);
        while(getchar()!='\n') continue;
    }
}

void addToDictionary(){
    char input[wordLength];
    wordNode *toInsert;
    int index;
    while (1) {
        getchar();
        input[0] = getchar();
        if(input[0] == '+') break;
        for (int j = 1; j < wordLength; j++)
            input[j] = getchar();
        input[wordLength] = '\0';
        toInsert = initWordNode(input);
        index = hash(input) % TABLE_SIZE;
        insertWordNode(toInsert, index);
        wordCount++;
        totalWordCount++;
        //printf("INSERT - wordCount = %d (total = %d)\n", wordCount,  totalWordCount);
        //printList(index);
    }

    executeCommand();
}

int main(void){
    for(int i=0;i<SYM_TABLE_SIZE;i++) symTable[i] = NULL;
    for(int i=0;i<TABLE_SIZE;i++) hashTable[i] = NULL;
    if(scanf("%d", &wordLength)>0);
    wordCount = 0;
    totalWordCount = 0;
    tempWord=(char*)malloc(wordLength*sizeof(char));
    tempWord[wordLength] = '\0';
    addToDictionary();
    return 0;
}
