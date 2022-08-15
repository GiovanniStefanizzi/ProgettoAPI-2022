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

typedef struct orderNode{
    int index;
    int position;
    bool tiePass;
    struct orderNode *next;
}orderNode;

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
orderNode *head = NULL;
orderNode *tiedHead = NULL;
symbol *symTable[SYM_TABLE_SIZE];

wordNode *initWordNode(char *);
void executeCommand();
void addToDictionary();
char *searchWordByIndex(int, int);
wordNode *hashSearch(char *);
char *tempWord;

void printOrderedList()
{
    //printf("\n");
    if(head == NULL){
        printf("list is empty");
        return;
    }
    orderNode *temp = head;
    while(temp!=NULL){
        if(temp->tiePass) {
            //printf("index = %d, position = %d: ", temp->index, temp->position);
            printf("%s\n", searchWordByIndex(temp->index, temp->position));
        }
        temp = temp->next;
    }
    //printf("");
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
        orderNode *temp = head;
        while(temp != NULL){
            temp->tiePass = true;
            temp = temp -> next;
        }
    }
}

orderNode *insertInOrder(int index, int position) {
    orderNode *newNode;
    newNode = (orderNode *) malloc(sizeof(orderNode));
    newNode->index = index;
    newNode->position = position;
    newNode->tiePass = true;
    newNode->next = NULL;
    if (head == NULL) {
        return newNode;
    } else {
        if (strcmp(searchWordByIndex(index, position), searchWordByIndex(head->index, head->position)) < 0) {
            newNode->next = head;
            return newNode;
        }
        orderNode *temp = head;
        while (temp->next != NULL) {
            if (strcmp(searchWordByIndex(index, position), searchWordByIndex(temp->next->index, temp->next->position)) <0) {
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
    wordCount++;
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
    head = insertInOrder(index, position);
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

bool singleTiePassCheck(const char *word){
    symbol *sym;
    int currCount;
    for(int i=0;i<SYM_TABLE_SIZE;i++){
        sym = symTable[i];
        if(sym != NULL){
            if(!sym->contains){
                if(contains(word, sym->ch)) return false;
            }
            else{
                currCount = 0;
                for(int j=0;j<wordLength;j++){
                    if(sym->mustBeIn[j] && word[j] != sym -> ch) return false;
                    if(sym->mustNotBeIn[j] && word[j] == sym ->ch) return false;
                    if(sym->ch == word[j]) currCount ++;
                }
                if(sym->exactTimes){
                    if(sym->times != currCount) return false;
                }
                else{
                    if(currCount < sym -> times) return false;
                }
            }
        }
    }
    return true;
}

void tiePassCheck(){
    symbol *sym;
    orderNode *temp;
    int currCount;
    bool pass;
    for(int i=0;i<SYM_TABLE_SIZE;i++){
        sym = symTable[i];
        if(sym != NULL){
            temp = head;
            if(!sym -> contains){
                while(temp != NULL){
                    if(temp -> tiePass) {
                        tempWord = searchWordByIndex(temp->index, temp -> position);
                        if (contains(tempWord, sym->ch)) {
                            temp->tiePass = false;
                            wordCount--;
                        }
                    }
                    temp = temp->next;
                }
            }
            else {
                temp = head;
                while(temp != NULL){
                    if(temp->tiePass) {
                        pass = true;
                        currCount = 0;
                        tempWord = searchWordByIndex(temp->index, temp->position);
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
                            temp->tiePass = false;
                        }

                    }
                    temp = temp->next;
                }

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
            }
            else{
                setContains(s[i], false);
            }
        }
    }
    return result;
}


void startNewMatch(){
    char toBeFound[wordLength];
    char input[wordLength];
    char endInput = 0;
    const char *result;
    wordNode *searched;
    resetOrderedList();
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
            }
            getchar();
            attempts--;
            if(attempts==0 && result[0]!='o') printf("ko\n");
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
        printOrderedList();
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
        //printf("word to insert: %s, hash calculated is %d \n", input, index);
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