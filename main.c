#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define SYM_TABLE_SIZE 64

typedef struct BSTNode{
    char *word;
    bool tiePass;
    bool taken;
    struct BSTNode *left, *right, *next, *nextPass;
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
symbol *symTable[SYM_TABLE_SIZE];
char *input;
char *result;


BSTNode *root=NULL;
BSTNode *lastOfTheList;
BSTNode *firstToPass;
BSTNode *lastToPass;
BSTNode *lastChecked;

int memCnt=0;
int allocCnt=0;

void executeCommand();
void addToDictionary();
bool contains(const char *string, char c);

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

bool isEqual(const char *a, const char *b){
    const unsigned char *wa = (const unsigned char *)a;
    const unsigned char *wb = (const unsigned char *)b;
    for(int i=0;i<wordLength;i++){
        if(*wb != *wa) return false;
        wa++;
        wb++;
    }
    return true;
}

BSTNode *BSTInit (BSTNode *node){
    node->word=malloc(wordLength*sizeof(char*));
    for(int i=0;i<wordLength;i++){
        node->word[i]=input[i];
    }
    node->word[wordLength]='\0';
    node->next=NULL;
    node->left=NULL;
    node->right=NULL;
    node->tiePass=true;
    node->nextPass=NULL;
    node->taken=true;
    if(lastOfTheList==NULL){
        lastOfTheList = node;
        lastToPass=lastOfTheList;
    }
    else {
        lastOfTheList->next = node;
        lastToPass -> nextPass= node;
        lastOfTheList = node;
        lastToPass = node;
    }
    return node;
}

BSTNode *BSTAlloc(BSTNode *node){
    node = (BSTNode*)malloc(4096*sizeof(BSTNode));
    for(int i =0;i<4096;i++) node[i].taken=false;
    memCnt=4096;
    allocCnt=1;
    return node;
}

BSTNode *BSTRealloc(BSTNode *node){
    
    BSTNode *temp;
    temp=root;
    
    
    for(int i=0;i<allocCnt-1;i++){
        temp = &temp[4095];
        temp = temp->next;
    }
    
    temp = &temp[4095];
    temp->next = (BSTNode*)malloc(4096*sizeof(BSTNode));
    temp = temp->next;
    
    allocCnt++;
    memCnt=4096;
    return node;
}


BSTNode *BSTInsert(BSTNode *node){
    if(node==NULL && allocCnt==0) root = BSTAlloc(root);
   
    if(node==NULL){
    
        memCnt--;
        if(totalWordCount != 0 && totalWordCount%4096 == 0){
            //reallocate
            root = BSTRealloc(root);
    	}
        BSTNode *temp = root;
        for(int i=0;i<allocCnt-1;i++){
         //printf("*");
         temp = &temp[4095];
         temp = temp->next;
        }
        //printf("%s\n", temp->word);
        int index = totalWordCount-((allocCnt-1)<<12); 
        //printf("%d \n", index);
        return BSTInit(&temp[index]);
        
    }
    if(isLessThan(input, node->word)) {
        node->left =  BSTInsert(node->left);
    }
    else node->right = BSTInsert(node->right);
    return node;
}

void BSTPrintInOrder(BSTNode *node){
    if(node!=NULL){
        BSTPrintInOrder(node->left);
        if(node->tiePass){
            puts(node->word);
        }
        BSTPrintInOrder(node->right);
    }
}

void BSTReset(){
    if(root!=NULL) {
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

BSTNode *BSTSearch(BSTNode *node){
    if(node ==NULL || isEqual(node->word, input))  return node;
    if (isLessThan(node->word, input)) return BSTSearch(node->right);
    return BSTSearch(node->left);
}


void resetSymTable(){
    for(int i=0;i<SYM_TABLE_SIZE;i++){
        if(symTable[i] != NULL){
            free(symTable[i]->mustBeIn);
            free(symTable[i]->mustNotBeIn);
            free(symTable[i]);
            symTable[i] = NULL;
        }
    }
}

int symHash(char c){
    int ascii = (int)c;
    if(ascii==45) return 0;
    if(ascii==95) return 1;
    if(ascii>=48 && ascii<=57) return ascii - 46;
    if(ascii>=65 && ascii<=90) return ascii - 53;
    else return ascii - 59;
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
    for (int i = 0; i < SYM_TABLE_SIZE; i++) {
        sym = symTable[i];
        if (sym != NULL) {
            BSTNode *temp = firstToPass;
            if (!sym->contains) {
                while(temp != NULL) {
                    if (contains(temp->word, sym->ch)) {
                        temp->tiePass = false;
                        if(temp==firstToPass){
                            lastChecked=NULL;
                            firstToPass=temp->nextPass;
                        }
                        else if(temp==lastToPass){
                            lastToPass=lastChecked;
                            if(lastToPass!=NULL) lastToPass->nextPass = NULL;
                        }
                        else{
                            if(lastChecked!=NULL)lastChecked->nextPass=temp->nextPass;
                        }
                        wordCount--;
                    }
                    else lastChecked = temp;
                    temp = temp->nextPass;
                }
            }
            else {
                while(temp != NULL){
                    char *tempWord;
                    bool pass = true;
                    int currCount = 0;
                    pass = true;
                    currCount = 0;
                    tempWord = temp->word;
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
                        if(temp==firstToPass){
                            lastChecked=NULL;
                            firstToPass=temp->nextPass;
                        }
                        else if(temp==lastToPass){
                            lastToPass=lastChecked;
                            if(lastToPass!=NULL) lastToPass->nextPass = NULL;
                        }
                        else{
                            if(lastChecked!=NULL)lastChecked->nextPass=temp->nextPass;
                        }
                    }else lastChecked = temp;
                    temp = temp->nextPass;
                }
            }
        }
    }
}

void  compareWords(const char * r, const char * s) {
    bool used[wordLength];
    for (int i = 0; i < wordLength; i++){
        result[i] = '/';
        resetCurrTimes(s[i]);
        used[i] = false;
    }
    result[wordLength] = '\0';
    int rightCount=0;
    for (int i = 0; i < wordLength; i++) {
        if (s[i] == r[i]) {
            result[i] = '+';
            used[i] = true;
            rightCount++;
            mustBeIn(s[i],i);
            increaseSymCount(s[i]);
        }
    }
    if (rightCount == wordLength) {
        result[0] = 'o';
        result[1] = 'k';
        for(int i=2;i<wordLength;i++) result[i] = '\0';
        return;
    }
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
            }
        }
    }
    return;
}

void startNewMatch(){
    bool found=false;
    char toBeFound[wordLength];
    char endInput = 0;
    BSTNode *searched;
    BSTReset(root);
    resetSymTable();
    wordCount = totalWordCount;
    while(getchar()!='\n') continue;
    for(int i=0;i<wordLength;i++){
        toBeFound[i] = getchar();
    }
    if(scanf("%d\n", &attempts)>0);
    while (attempts > 0) {
        input[0] = getchar();
        if(input[0] == '+'){
            executeCommand();
        }
        else if(input[0]==EOF) break;
        else {
            for (int j = 1; j < wordLength; j++)
                input[j] = getchar();
            input[wordLength] = '\0';
            searched = BSTSearch(root);
            if(searched==NULL){
                printf("not_exists\n");
                attempts++;
            }
            else{
                compareWords(toBeFound, searched->word);
                puts(result);
                if(result!=NULL && !(result[0]=='o'|| result[0]=='k')) {
                    tiePassCheck();
                    printf("%d\n", wordCount);
                }
                else found=true;
            }
            getchar();
            attempts--;
        }
    }
    if(!found) {
        printf("ko\n");
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
        startNewMatch();
    }
    if(command[0]=='i') {
        for(int i=0;i<9;i++) command[i] = getchar();
        if(getchar()=='i') {
            for(int i=0;i<5;i++) getchar();
            addToDictionary();
        }
        else{
            for(int i=0;i<4;i++) getchar();
        }
    }
    if(command[0]=='s') {
        tiePassCheck();
        BSTPrintInOrder(root);
        while(getchar()!='\n') continue;
    }
}

void addToDictionary(){
    while (1) {
        getchar();
        input[0] = getchar();
        if(input[0] == '+') break;
        for (int j = 1; j < wordLength; j++)
            input[j] = getchar();
        input[wordLength] = '\0';
        root = BSTInsert(root);
        wordCount++;
        totalWordCount++;
    }
    executeCommand();
}

int main(void){
    for(int i=0;i<SYM_TABLE_SIZE;i++) symTable[i] = NULL;
    if(scanf("%d", &wordLength)>0);
    wordCount = 0;
    totalWordCount = 0;
    input = malloc(wordLength*sizeof(char*));
    result = malloc(wordLength*sizeof(char*));
    addToDictionary();
    return 0;
}
