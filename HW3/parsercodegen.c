/* This program was written by:
    - Roosevelt Desire
    - Bryan Aneyro Hernandez */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

    // Define token types as enumerated values
enum TokenType
{
    oddsym = 1, identsym = 2, numbersym = 3,
    plussym = 4, minussym = 5, multsym = 6, slashsym = 7,
    ifelsym = 8, eqlsym = 9, neqsym = 10, lessym = 11,
    leqsym = 12, gtrsym = 13, geqsym = 14, lparentsym = 15,
    rparentsym = 16, commasym = 17, semicolonsym = 18,
    periodsym = 19, becomessym = 20, beginsym = 21,
    endsym = 22, ifsym = 23, thensym = 24, whilesym = 25,
    dosym = 26, callsym = 27, constsym = 28, varsym = 29,
    procsym = 30, writesym = 31, readsym = 32, elsesym = 33
};


// Implementing Stack to take care of the comments
typedef struct
{
    char* item;
    int top;
    int size;
} Stack;

// Struct for a linked list node
struct TokenNode {
    char lexeme[12];
    enum TokenType type;
    struct TokenNode* next;
};

typedef struct Token {
    char lexeme[12];
    enum TokenType type;
}tokenArray;

typedef struct namerecord_t {
    int kind;      // const = 1, var = 2, proc = 3.
    char* name;   // name up to 11 chars
    int val;       // number (ASCII value)
    int level;     // L level
    int address;   // M address
    int mark;      // 0 = in use for code generation; 1 = unavailable
} namerecord_t;

typedef struct code {
    int OP;
    int L;
    int M;
}code;

#define MAX_ARRAY_SIZE 512

static namerecord_t* symbolTable[MAX_ARRAY_SIZE];
int symbolTableIndex = 0;

static code codeArray[MAX_ARRAY_SIZE];
int codeArrayIndex = 0;

static tokenArray tokens[MAX_ARRAY_SIZE];
tokenArray currentToken;
tokenArray nextToken;
int currentTokenIndex = -1;
int lastToken = 0;

// keep track of the address.
int address = 3;

FILE* fh_output;

// prototype declarations
void init(Stack*, int, FILE*);
void push(Stack*, char, FILE*);
char pop(Stack*, FILE*);
void deallocate(Stack*);
void printInputArray(char* A, int size);
namerecord_t* makeARecord(enum TokenType type, char* name);
void generateSysmbolTable(struct TokenNode* head, struct namerecord_t** sysmbolTable);
void printSysmbolTable(struct namerecord_t** sysmbolTable, int size);
int SYMBOLTABLECHECK(char* target);
void FACTOR();
void TERM();
void EXPRESSION();
void CONDITION();
void STATEMENT();
int VAR();
void CONSTANT();
void BLOCK();
void PROGRAM();
void whatAreMyPointers();
bool CHECKSYMBOLTHETABLE(char*);

// ================== Stack Functions to Help with Comments ====================//

void push(Stack* sp, char value, FILE* fh_output)
{
    if (sp->top == sp->size - 1)
    {
        printf("Stack overflow\n");
        fprintf(fh_output, "Stack overflow\n");
        return;
    }

    sp->top++;
    sp->item[sp->top] = value;
}

char pop(Stack* sp, FILE* fh_output)
{
    if (sp->top == -1)
    {
        printf("Stack Underflow\n");
        fprintf(fh_output, "Stack Underflow\n");
        return '\0';
    }
    char value;
    value = sp->item[sp->top];
    sp->top--;
    return value;
}

/**
 The init method this time dynamically allocates the array
 sp - pointer to the stack object.
 size - size of the stack
*/
void init(Stack* sp, int size, FILE* fh_output)
{
    sp->top = -1;
    // using malloc allocating the stack dynamically

    sp->item = (char*)malloc(sizeof(char) * size);
    if (sp->item == NULL)
    {
        printf("Unable to allocate memory\n");
        fprintf(fh_output, "Unable to allocate memory\n");
        exit(1);
    }
    sp->size = size;
}

/**
 deallocate function will deallocate the
 dynamically allocated array for the stack.
*/
void deallocate(Stack* sp)
{
    if (sp->item != NULL)
        free(sp->item);
    sp->top = -1;
    sp->size = 0;
}

// ================== Linked List Functions to Help with Output ======================//

// Function to create a new node
struct TokenNode* createTokenNode(const char* lexeme, enum TokenType type)
{
    struct TokenNode* newNode = (struct TokenNode*)malloc(sizeof(struct TokenNode));
    if (newNode == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strncpy(newNode->lexeme, lexeme, 12);
    newNode->type = type;
    newNode->next = NULL;
    return newNode;
}

// Function to insert a node at the end of the list
void insertTokenNode(struct TokenNode** head, const char* lexeme, enum TokenType type)
{
    struct TokenNode* newNode = createTokenNode(lexeme, type);

    if (*head == NULL)
    {
        *head = newNode;
    }
    else
    {
        struct TokenNode* current = *head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = newNode;
    }
}

// Function to print the linked list
void printTokenList(struct TokenNode* head, FILE* fh_output)
{
    printf("\n");
    fprintf(fh_output, "\n");

    printf("Token List:\n");
    fprintf(fh_output, "Token List:\n");

    struct TokenNode* current = head;
    while (current != NULL)
    {
        if (current->type != 0) {

            printf("%d ", current->type);
            fprintf(fh_output,"%d ", current->type);
        }

        if (current->type == 2 || current->type == 3) {

            printf("%s ", current->lexeme);
            fprintf(fh_output,"%s ",current->lexeme);
        }

        current = current->next;
    }
    printf("\n");
    fprintf(fh_output, "\n");
}


void freeTokenList(struct TokenNode* head)
{
    struct TokenNode* current = head;
    while (current != NULL)
    {
        struct TokenNode* next = current->next;
        free(current);
        current = next;
    }
}

// ================ Helper Functions ==================//

// Function to check if a string is a PL/0 reserved word
int isReservedWord(char* str)
{
    char* reservedWords[] = {
        "const", "var", "procedure", "call", "begin", "end",
        "if", "then", "ifel", "else", "while", "do", "read", "write", "odd"
    };

    int numReservedWords = sizeof(reservedWords) / sizeof(reservedWords[0]);

    for (int i = 0; i < numReservedWords; i++)
    {
        if (strcmp(str, reservedWords[i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}

// Check character by character for the special symbols
int isSpecialChar(char ch)
{
    // Define the set of allowed special characters
    char specialChars[] = "+-*/()=,.<>;:";

    // Check if the character is in the set of allowed special characters
    for (int i = 0; specialChars[i] != '\0'; i++)
    {
        if (ch == specialChars[i])
        {
            return 1; // Return 1 if it's a special character
        }
    }

    // If the character is not found in the set, return 0
    return 0;
}

// Function to recognize and output tokens
struct TokenNode* recognizeTokens(FILE* sourceFile, int size, Stack* s, FILE* fh_output)
{
    struct Token token;
    char currentChar;
    char lexeme[100];
    int lexemeIndex = 0;
    int lineNumber = 1;
    char table[size];
    int tableSize = 0;
    int tokenType[size];
    struct TokenNode* tokenList = NULL;


    //printf("Lexeme Table: \n");
    //fprintf(fh_output,"Lexeme Table: \n");
   // printf("\n");
   // fprintf(fh_output,"\n");

    while ((currentChar = fgetc(sourceFile)) != EOF)
    {
        for (int i = 0; i < 13; i++)
            lexeme[i] = 0;

        if (currentChar == '\n')
        {
            continue;
        }

        if (isspace(currentChar))
        {
            continue;
        }

        if (currentChar == '\t')
        {
            continue;
        }

        if (isalpha(currentChar))
        {
            lexeme[0] = '\0';
            lexeme[lexemeIndex++] = currentChar;

            while (isalpha(currentChar = fgetc(sourceFile)))
            {
                lexeme[lexemeIndex++] = currentChar;
            }

            lexeme[lexemeIndex] = '\0';

            if (strlen(lexeme) > 11)
            {
                printf("Error: name too long '%s'\n", lexeme);
                fprintf(fh_output, "Error: name too long '%s'\n", lexeme);
                // lexeme[0] = '\0';
                // lexemeIndex = 0;
                // token.type = 0;
                exit(1);
            }
            else
            {
                if (isReservedWord(lexeme))
                {
                    // It's a reserved word, assign the corresponding token type
                    if (strcmp(lexeme, "const") == 0)
                    {
                        token.type = constsym;
                    }
                    else if (strcmp(lexeme, "var") == 0)
                    {
                        token.type = varsym;
                    }
                    else if (strcmp(lexeme, "procedure") == 0)
                    {
                        // HW3 treats it as identifier
                        token.type = identsym;
                    }
                    else if (strcmp(lexeme, "call") == 0)
                    {
                        // HW3 treats it as identifier
                        token.type = identsym;
                    }
                    else if (strcmp(lexeme, "begin") == 0)
                    {
                        token.type = beginsym;
                    }
                    else if (strcmp(lexeme, "end") == 0)
                    {
                        token.type = endsym;
                    }
                    else if (strcmp(lexeme, "if") == 0)
                    {
                        token.type = ifsym;
                    }
                    else if (strcmp(lexeme, "then") == 0)
                    {
                        token.type = thensym;
                    }
                    else if (strcmp(lexeme, "ifel") == 0)
                    {
                        // HW3 treats it as identifier
                        token.type = identsym;
                    }
                    else if (strcmp(lexeme, "else") == 0)
                    {
                        // HW3 treats it as identifier
                        token.type = identsym;
                    }
                    else if (strcmp(lexeme, "while") == 0)
                    {
                        token.type = whilesym;
                    }
                    else if (strcmp(lexeme, "do") == 0)
                    {
                        token.type = dosym;
                    }
                    else if (strcmp(lexeme, "read") == 0)
                    {
                        token.type = readsym;
                    }
                    else if (strcmp(lexeme, "write") == 0)
                    {
                        token.type = writesym;
                    }
                    else if(strcmp(lexeme, "odd") == 0)
                    {
                        token.type = oddsym;
                    }
                }
                else
                {
                    // It's an identifier, assign the identifier token type
                    token.type = identsym;
                }
            }
        }
        // if it's a correct special character, assign the corresponding token type
        else if (ispunct(currentChar))
        {
            if (!isSpecialChar(currentChar))
            {
                printf("Error: wrong special character '%s'\n", lexeme);
                fprintf(fh_output, "Error: wrong special character '%s'\n", lexeme);
                // lexeme[0] = '\0';
                // lexemeIndex = 0;
                // token.type = 0;
                // continue;
                exit(1);
            }

            lexeme[0] = '\0';
            lexeme[lexemeIndex++] = currentChar;

            while (ispunct(currentChar = fgetc(sourceFile)))
            {
                if (!isSpecialChar(currentChar))
                {
                    printf("Error: wrong special character '%s'\n", lexeme);
                    fprintf(fh_output, "Error: wrong special character '%s'\n", lexeme);
                    // lexeme[0] = '\0';
                    // lexemeIndex = 0;
                    // token.type = 0;
                    // continue;
                    exit(1);
                }

                lexeme[lexemeIndex++] = currentChar;

            }

            lexeme[lexemeIndex] = '\0';

            // Takes care of comments
            if (strcmp(lexeme, "/*") == 0)
            {
                token.type = 0; // Set the token type to 0 (comment)

                int prevChar = '\0'; // Store the previous character

                while ((currentChar = fgetc(sourceFile)) != EOF)
                {
                    // Push the current character to the stack
                    push(s, currentChar, fh_output);

                    // Check for the end of the comment (*/)
                    if (prevChar == '*' && currentChar == '/')
                    {
                        break; // Exit the comment loop
                    }

                    // Update the previous character
                    prevChar = currentChar;
                }

                for (int i = 0; i < 13; i++)
                    lexeme[i] = 0;

                lexeme[0] = ungetc(currentChar, sourceFile); // Reset the lexeme

                continue;
            }
            else
            {
                if (strcmp(lexeme, ".") == 0)
                {
                    token.type = periodsym;
                }
                else if (strcmp(lexeme, "=") == 0)
                {
                    token.type = eqlsym;
                }
                else if (strcmp(lexeme, ",") == 0)
                {
                    token.type = commasym;
                }
                else if (strcmp(lexeme, ";") == 0)
                {
                    token.type = semicolonsym;
                }
                else if (strcmp(lexeme, "+") == 0)
                {
                    token.type = plussym;
                }
                else if (strcmp(lexeme, "-") == 0)
                {
                    token.type = minussym;
                }
                else if (strcmp(lexeme, "*") == 0)
                {
                    token.type = multsym;
                }
                else if (strcmp(lexeme, "/") == 0)
                {
                    token.type = slashsym;
                }
                else if (strcmp(lexeme, "(") == 0)
                {
                    token.type = lparentsym;
                }
                else if (strcmp(lexeme, ")") == 0)
                {
                    token.type = rparentsym;
                }
                else if (strcmp(lexeme, "<") == 0)
                {
                    token.type = lessym;
                }
                else if (strcmp(lexeme, "<=") == 0)
                {
                    token.type = leqsym;
                }
                else if (strcmp(lexeme, ">") == 0)
                {
                    token.type = gtrsym;
                }
                else if (strcmp(lexeme, ">=") == 0)
                {
                    token.type = geqsym;
                }
                else if (strcmp(lexeme, "<>") == 0)
                {
                    token.type = neqsym;
                }
                else if (strcmp(lexeme, ":=") == 0)
                {
                    token.type = becomessym;
                }
                else if (strcmp(lexeme, "/"))
                {
                    token.type = 0;
                }
                else
                {
                    token.type = 0;
                    lexeme[0] = '\0';
                    printf("Error: wrong special character '%s'\n", lexeme);
                    fprintf(fh_output, "Error: wrong special character '%s'\n", lexeme);
                    exit(1);

                }

            }
        }
        else if (isdigit(currentChar))
        {

            lexeme[0] = '\0';
            lexeme[lexemeIndex++] = currentChar;

            while (isdigit(currentChar = fgetc(sourceFile)))
            {
                lexeme[lexemeIndex++] = currentChar;
            }

            lexeme[lexemeIndex] = '\0';

            if (strlen(lexeme) > 5)
            {
                long long num = atoll(lexeme);
                printf("Error: number too long '%lld'\n", num);
                fprintf(fh_output, "Error: number too long '%lld'\n", num);
                // lexeme[0] = '\0';
                // token.type = 0;
                exit(1);
            }
            else
            {
                if (strcmp(lexeme, "0"))
                {
                    token.type = numbersym;
                }
                else if (strcmp(lexeme, "1"))
                {
                    token.type = numbersym;
                }
                else if (strcmp(lexeme, "2"))
                {
                    token.type = numbersym;
                }
                else if (strcmp(lexeme, "3"))
                {
                    token.type = numbersym;
                }
                else if (strcmp(lexeme, "4"))
                {
                    token.type = numbersym;
                }
                else if (strcmp(lexeme, "5"))
                {
                    token.type = numbersym;
                }
                else if (strcmp(lexeme, "6"))
                {
                    token.type = numbersym;
                }
                else if (strcmp(lexeme, "7"))
                {
                    token.type = numbersym;
                }
                else if (strcmp(lexeme, "8"))
                {
                    token.type = numbersym;
                }
                else if (strcmp(lexeme, "9"))
                {
                    token.type = numbersym;
                }

            }

        }

        if (token.type != 0)
        {
            printf("%s", lexeme);
            fprintf(fh_output,"%s", lexeme);
            printf("\t %d\n", token.type);
            fprintf(fh_output,"\t %d\n", token.type);


            insertTokenNode(&tokenList, lexeme, token.type); // Insert into linked list
        }




        if (currentChar == EOF)
            break;

        fseek(sourceFile, -1L, SEEK_CUR);

        lexemeIndex = 0;

        // generateSysmbolTable(tokenList, sysmbolTable);

    }

    printf("\n");
    printTokenList(tokenList, fh_output);
    return tokenList;

}

void printFile(FILE* fp, FILE* fh_output)
{
    char currentChar;

    printf("Source Program: \n");
    fprintf(fh_output, "Source Program: \n");

    while ((currentChar = fgetc(fp)) != EOF)
    {
        printf("%c", currentChar);
        fprintf(fh_output, "%c", currentChar);
    }

    printf("\n");
    printf("\n");
    fprintf(fh_output, "\n\n");
}


// ========================= HW3 ============================

// Function to get the next token
void getNextToken() {
    // Check if there are more tokens in the array
    if (currentTokenIndex >= lastToken) {
        currentToken.type = EOF;
        strcpy(currentToken.lexeme, "EOF");
    }
    else {

        currentToken = tokens[currentTokenIndex + 1];
        currentTokenIndex += 1;

        if (currentTokenIndex < lastToken) {
            nextToken = tokens[currentTokenIndex + 1];
        }
        else {
            nextToken.type = EOF;
            strcpy(nextToken.lexeme, "EOF");
        }
    }
}

void printSymbolTable() {
    printf("\n");
    fprintf(fh_output, "\n");

    printf("Symbol Table:\n");
    fprintf(fh_output, "Symbol Table:\n");

    printf("\n");
    fprintf(fh_output, "\n");

    printf("Kind|Name\t | Value | Level | Address | Mark\n");
    fprintf(fh_output, "Kind|Name\t | Value | Level | Address | Mark\n");
    printf("----------------------------------------------------\n");
    fprintf(fh_output, "----------------------------------------------------\n");
    for (int i = 0; i < symbolTableIndex; i++)
    {

        printf("  %d |%-12s| %d\t | %d\t | %d\t   | %d\n", symbolTable[i]->kind, symbolTable[i]->name,
            symbolTable[i]->val, symbolTable[i]->level, symbolTable[i]->address, symbolTable[i]->mark);
        
        fprintf(fh_output, "  %d |%-12s| %d\t | %d\t | %d\t   | %d\n", symbolTable[i]->kind, symbolTable[i]->name,
            symbolTable[i]->val, symbolTable[i]->level, symbolTable[i]->address, symbolTable[i]->mark);

    }

    printf("\n");
    fprintf(fh_output, "\n");
}

// Function to create a namerecord_t
// struct namerecord_t* makeRecord(enum TokenType type, char* name, int mark, int address, int level) {
//     struct namerecord_t* record = malloc(sizeof(struct namerecord_t));
//     record->kind = type;

//     int nameLength = strlen(name) + 1;
//     record->name = malloc(sizeof(char) * nameLength);
//     strcpy(record->name, name);

//     record->mark = mark;
//     record->address = address;
//     record->level = level;

//     return record;
// }

// // Function to add a symbol to the global symbol table
// void addSymbol(tokenArray* token) {

//     // Should check for token in table already???------------------<---------------<-------------
//     if (symbolTableIndex >= MAX_ARRAY_SIZE) {
//         // Handle symbol table full error here
//         printf("Symbol table is full. Cannot add more symbols.\n");
//         return;
//     }

//     // Create a namerecord and add it to the global symbol table
//     symbolTable[symbolTableIndex++] = makeRecord(token->type, token->lexeme, 0, address, 0);
//     address++;
// }

void addSymbol(int kind, char* name, int value, int level, int addr, int mark) {
    if(symbolTableIndex >= MAX_ARRAY_SIZE) {
        printf("Symbol table is full\n");
        exit(1);
    }

    namerecord_t* sym = (namerecord_t*)malloc(sizeof(namerecord_t));
    
    if (sym == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    sym->kind = kind;
    sym->name = (char*)malloc((strlen(name) + 1) * sizeof(char));
    
    if (sym->name == NULL) {
        printf("Memory allocation failed\n");
        free(sym);  // Free previously allocated memory
        exit(1);
    }

    strcpy(sym->name, name);
    sym->val = value;
    sym->mark = mark;
    sym->level = level;

    if(sym->kind == 1) {
        sym->address = 0;
    }
    else {
        sym->address = addr;
        address++;
    }
    
    symbolTable[symbolTableIndex] = sym;
    symbolTableIndex++;
}



// Function to change the mark of a record to 1
void markSymbol(char* name) {
    for (int i = 0; i < symbolTableIndex; i++) {
        if (strcmp(symbolTable[i]->name, name) == 0) {
            symbolTable[i]->mark = 1;
            return; // Assuming the name is unique; no need to continue searching
        }
    }
}

void EMIT(int op, int L, int M)
{
    // At the beginning, always jump to address 3
    if (codeArrayIndex == 0) {
        codeArray[0].OP = 7;
        codeArray[0].L = 0;
        codeArray[0].M = 3;
        codeArrayIndex++;
    }

    if (codeArrayIndex > MAX_ARRAY_SIZE) {
        printf("Error: codeArrayIndex is out of bounds\n");
        exit(1);
    }
    else {
        codeArray[codeArrayIndex].OP = op; //opcode
        codeArray[codeArrayIndex].L = L; // lexicographical level
        codeArray[codeArrayIndex].M = M; // modifier
        codeArrayIndex++;
    }
}

void FACTOR() {
    //    getNextToken();
    int symIdx = 0;
    int number = 0;
    if (currentToken.type == identsym) {
        symIdx = SYMBOLTABLECHECK(currentToken.lexeme);

        if (symIdx == -1) {
            printf("Error: undeclared identifier %s\n", currentToken.lexeme);
            fprintf(fh_output, "Error: undeclared identifier %s\n", currentToken.lexeme);
            exit(1);
        }

        if (symbolTable[symIdx]->kind == 1) {
            // emit LIT (M = table[symIdx].Value)
            EMIT(1, 0, symbolTable[symIdx]->val);
        }
        else {
            // emit LOD (M = table[symIdx].addr)
            EMIT(3, 0, symbolTable[symIdx]->address);
        }

        getNextToken();
        // whatAreMyPointers();
    }
    else if (currentToken.type == numbersym) {

        number = atoi(currentToken.lexeme);
        //printf("\nthis is numbber going to EMIT 1, 0, %d\n", number);

        // emit LIT
        EMIT(1, 0, number);
        getNextToken();
       // whatAreMyPointers();
    }
    else if (currentToken.type == lparentsym) {
        getNextToken();
        EXPRESSION();

        if (currentToken.type != rparentsym) {
            printf("Error: right parenthesis must follow left parenthesis\n");
            fprintf(fh_output, "Error: right parenthesis must follow left parenthesis\n");
            exit(1);
        }

        getNextToken();
    }
    else {
        printf("Error: arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
        fprintf(fh_output, "Error: arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
        exit(1);
    }
}

void CONDITION() {
    //    getNextToken();

            /* skipsym is odd function and we are waiting on the professor
            if token == oddsym
            get next token
            EXPRESSION
            emit ODD*/
    if (currentToken.type == oddsym) {
        getNextToken();
        EXPRESSION();

        // emit ODD
        EMIT(2, 0, 11);

    }
    else {

        EXPRESSION();
        if (currentToken.type == eqlsym) {
            getNextToken();
            EMIT(2, 0, 5);
        }
        else if (currentToken.type == neqsym) {
            getNextToken();
            EXPRESSION();
            EMIT(2, 0, 6);
        }
        else if (currentToken.type == lessym)
        {
            getNextToken();
            EXPRESSION();
            EMIT(2, 0, 7);
        }
        else if (currentToken.type == leqsym)
        {
            getNextToken();
            EXPRESSION();
            EMIT(2, 0, 8);
        }
        else if (currentToken.type == gtrsym)
        {
            getNextToken();
            EXPRESSION();
            EMIT(2, 0, 9);
        }
        else if (currentToken.type == geqsym)
        {
            getNextToken();
            EXPRESSION();
            EMIT(2, 0, 10);
        }
        else {
            printf("Error: condition must contain comparison operator\n");
            fprintf(fh_output, "Error: condition must contain comparison operator\n");
            exit(1);
        }
    }

}

void TERM() {
    // getNextToken();
    FACTOR();
   // whatAreMyPointers();
    while (currentToken.type == multsym || currentToken.type == slashsym) {
        if (currentToken.type == multsym) {
            getNextToken();
           // whatAreMyPointers();
            FACTOR();

            // emit MUL
            EMIT(2, 0, 3);
        }
        else {
            getNextToken();
            FACTOR();

            // emit DIV
            EMIT(2, 0, 4);
        }
    }
}

void EXPRESSION() {
    //    getNextToken();

    if (currentToken.type == minussym) {
        getNextToken();
        TERM();

        while (currentToken.type == plussym || currentToken.type == minussym) {
            if (currentToken.type == plussym) {
                getNextToken();
                TERM();

                // emit ADD
                EMIT(2, 0, 1);
            }
            else {
                getNextToken();
                TERM();

                // emit SUB
                EMIT(2, 0, 2);
            }
        }
    }
    else {
        if (currentToken.type == plussym)
            getNextToken();

        TERM();
        while (currentToken.type == plussym || currentToken.type == minussym) {
            if (currentToken.type == plussym) {
                getNextToken();
                TERM();

                // emit ADD
                EMIT(2, 0, 2);
            }
            else {
                getNextToken();
                TERM();

                // emit SUB
                EMIT(2, 0, 2);
            }
        }
    }
}

// bool CHECKSYMBOLTHETABLE(char* target){

//     for (int i = 0; i < symbolTableIndex; i++) {
//         if (strcmp(symbolTable[i]->name, target) == 0)
//             return true;
//     }

//     return false;
// }

int SYMBOLTABLECHECK(char* target) {

    // true or false?
//    printf("\n%s \t %d\n", target, symbolTableIndex);
    // Empty
    if (symbolTableIndex == 0)
        return -1;

    // Start looking for target and return the index
    for (int i = 0; i < symbolTableIndex; i++) {
        if (strcmp(symbolTable[i]->name, target) == 0)
            return i;
    }

    // Not found
//    printf("Error: undeclared identifier %s", target);
    return -1;
}

void PROGRAM() {

    BLOCK();
    if (tokens[lastToken].type != periodsym){
        printf("Error: Program must end with period\n");
        fprintf(fh_output, "Error: Program must end with period\n");
        exit(1);
    }
    else 
        EMIT(9, 0, 3);

    // Mark var and const with 1 after program is done
    for (int i = 0; i < symbolTableIndex; i++) {
        markSymbol(symbolTable[i]->name);
    }

}

void CONSTANT() {
    //    getNextToken();

    if (currentToken.type == constsym) {
        do {
            getNextToken();

            if (currentToken.type != identsym) {
                printf("Error: const keyword must be followed by identifier\n");
                fprintf(fh_output, "Error: const keyword must be followed by identifier\n");
                exit(1);
            }

            if (SYMBOLTABLECHECK(currentToken.lexeme) != -1) {
                printf("Error: Constant name has already been declared\n");
                fprintf(fh_output, "Error: Constant name has already been declared\n");
                exit(1);
            }

            // if (SYMBOLTABLECHECK(currentToken.lexeme) == -1)
            // {
            //     printf("Error: undeclared identifier %s\n", currentToken.lexeme);
            //     exit(1);
            // }
            

            char const_name[12];
            strcpy(const_name, currentToken.lexeme);

            // printf("const_name in CONSTANT: %s\n", const_name);

            getNextToken();

            if (currentToken.type != eqlsym) {
                printf("Error: constants must be assigned with '='\n");
                fprintf(fh_output, "Error: constants must be assigned with '='\n");
                exit(1);
            }

            getNextToken();

            if (currentToken.type != numbersym) {
                printf("Error: constants must be assigned an integer value\n");
                fprintf(fh_output, "Error: constants must be assigned an integer value\n");
                exit(1);
            }

            int value = atoi(currentToken.lexeme);

            // addSymbol(int kind, char* name, int value, int level, int addr, int mark)
            addSymbol(1, const_name, value, 0, address, 0);
            getNextToken();

        } while (currentToken.type == commasym);

        if (currentToken.type != semicolonsym) {
            printf("Error: constant declarations must be followed by a semicolon\n");
            fprintf(fh_output, "Error: constant declarations must be followed by a semicolon\n");
            exit(1);
        }

        getNextToken();

    }

}

int VAR() {
    // getNextToken();
    int numVars = 0;
    if (currentToken.type == varsym) {
        do {
            numVars++;
            getNextToken();
            // whatAreMyPointers();

            if (currentToken.type != identsym) {
                printf("Error: var keyword must be followed by identifier\n");
                fprintf(fh_output, "Error: var keyword must be followed by identifier\n");
                exit(1);
            }

            if (SYMBOLTABLECHECK(currentToken.lexeme) != -1) {
                printf("Error: symbol name has already been declared\n");
                fprintf(fh_output, "Error: symbol name has already been declared\n");
                exit(1);
            }

            // if (!CHECKSYMBOLTHETABLE(currentToken.lexeme))
            // {
            //     printf("Error: undeclared identifier %s\n", currentToken.lexeme);
            //     exit(1);
            // }

            // addSymbol(int kind, char* name, int value, int level, int addr, int mark)
            addSymbol(2, currentToken.lexeme, 0, 0, address, 0);
            getNextToken();
            // If nextToken.type: 2 and nextToken.lexem: y
            // We know to keep going
            // whatAreMyPointers();

            //while(currentToken.type == commasym || (currentToken.type == semicolonsym));
        } while (currentToken.type == commasym);

        if (currentToken.type != semicolonsym) {
            printf("Error: var declaration must be followed by a semicolon\n");
            fprintf(fh_output, "Error: var declaration must be followed by a semicolon\n");
            exit(1);
        }

        getNextToken();
    }
    

    return numVars;
}

void STATEMENT() {
    //    getNextToken();
    // whatAreMyPointers();
    int symIdx = 0;
    int jpcIdx = 0;
    int loopIdx = 0;

    if (currentToken.type == identsym) {
        symIdx = SYMBOLTABLECHECK(currentToken.lexeme);

        // if (!CHECKSYMBOLTHETABLE(currentToken.lexeme))
        //     {
        //         printf("Error: undeclared identifier %s\n", currentToken.lexeme);
        //         exit(1);
        //     }

        // Here is the problem
        // printf("Here in statement\n");
        if (symIdx == -1) {
            printf("Error: undeclared identifier %s\n", currentToken.lexeme);
            fprintf(fh_output, "Error: undeclared identifier %s\n", currentToken.lexeme);
            exit(1);
        }

        if (symbolTable[symIdx]->kind != 2) {
            printf("Error: only variable values may be altered %s\n", currentToken.lexeme);
            fprintf(fh_output, "Error: only variable values may be altered %s\n", currentToken.lexeme);
            exit(1);
        }

        getNextToken();
        // whatAreMyPointers();

        if (currentToken.type != becomessym) {
            printf("Error: assignment statements must use ':='\n");
            fprintf(fh_output, "Error: assignment statements must use ':='\n");
            exit(1);
        }

        getNextToken();
        // whatAreMyPointers();
        EXPRESSION();

        // STO (M = table[symIdx].addr)
        EMIT(4, 0, symbolTable[symIdx]->address);
        return;
    }

    if (currentToken.type == ifsym) {
        getNextToken();
        // whatAreMyPointers();
        CONDITION();

        jpcIdx = codeArrayIndex;
        // printf("jpcIdx_1: %d\n", jpcIdx);

        // emit JPC
        EMIT(8, 0, 0);

        if (currentToken.type != thensym) {
            printf("Error: if must be followed by then\n");
            fprintf(fh_output, "Error: if must be followed by then\n");
            exit(1);
        }
        else {
            getNextToken();
            // whatAreMyPointers();
            STATEMENT();
            codeArray[jpcIdx].M = codeArrayIndex;

            // printf("jpcIdx_2: %d\n", jpcIdx);
            // printf("codeArray[jpcIdx].M: %d\n", codeArray[jpcIdx].M);
        }

        return;
    }

    if (currentToken.type == whilesym) {
        getNextToken();
        // whatAreMyPointers();
        loopIdx = codeArrayIndex;

        CONDITION();

        if (currentToken.type != dosym) {
            printf("Error: while must be followed by 'do'\n");
            fprintf(fh_output, "Error: while must be followed by 'do'\n");
            exit(1);
        }

        getNextToken();
        // whatAreMyPointers();

        jpcIdx = codeArrayIndex;

        // emit JPC
        EMIT(8, 0, 0);
        STATEMENT();

        // emit JMP (M = loopIdx)
        EMIT(7, 0, loopIdx);
        codeArray[jpcIdx].M = codeArrayIndex;

        return;
    }

    if (currentToken.type == readsym) {
        getNextToken();
        // whatAreMyPointers();

        if (currentToken.type != identsym) {
            printf("Error: read keyword must be followed by identifier\n");
            fprintf(fh_output, "Error: read keyword must be followed by identifier\n");
            exit(1);
        }

        symIdx = SYMBOLTABLECHECK(currentToken.lexeme);
        if (symIdx == -1) {
            printf("Error: undeclared identifier %s\n", currentToken.lexeme);
            fprintf(fh_output, "Error: undeclared identifier %s\n", currentToken.lexeme);
            exit(1);
        }

        if (symbolTable[symIdx]->kind != 2) {
            printf("Error: Read is not followed by a variable\n");
            fprintf(fh_output, "Error: Read is not followed by a variable\n");
            exit(1);
        }

        getNextToken();
        // whatAreMyPointers();

        // SYS 0 2 ----> emit READ
        EMIT(9, 0, 2);

        // emit STO (M = table[symIdx].addr)
        EMIT(4, 0, symbolTable[symIdx]->address);

        return;
    }

    if (currentToken.type == writesym) {
        getNextToken();
        EXPRESSION();

        // SYS 0 1 -----> emit WRITE
        EMIT(9, 0, 1);

        return;
    }

    if (currentToken.type == beginsym) {
        do {
            getNextToken();
            // whatAreMyPointers();
            STATEMENT();
        } while (currentToken.type == semicolonsym);

        if (currentToken.type != endsym) {
            printf("Error: begin must be followed by end\n");
            fprintf(fh_output, "Error: begin must be followed by end\n");
            exit(1);
        }

        getNextToken();
        // whatAreMyPointers();
        return;
    }

}



void BLOCK() {

    //whatAreMyPointers();
    CONSTANT();
    int numVars = VAR();
   // whatAreMyPointers();
    EMIT(6, 0, 3 + numVars);
    STATEMENT();
}

void printGeneratedCode() {

    printf("\n");
    fprintf(fh_output, "\n");

    printf("Line\tOP\tL\tM\n");
    fprintf(fh_output, "Line\tOP\tL\tM\n");

    for (int i = 0; i < codeArrayIndex + 1; i++) {
        int op = codeArray[i].OP;
        int lex = codeArray[i].L;
        int m = codeArray[i].M;

        // printf("m in printGeneratedCode[%d]: %d -----> ", i, codeArray[i].M);

        switch (op)
        {
            // Pushes m onto the stack
            case 1:
                printf("%d\tLIT\t%d\t%d", i, lex, m);
                printf("\n");

                fprintf(fh_output, "%d\tLIT\t%d\t%d", i, lex, m);
                fprintf(fh_output, "\n");

                break;

                // Operation to be performed on the data at the top of the stack.
            case 2:

                // Perform operation depending on M
                switch (m)
                {
                    // Return from a subroutine and prints the formatted output
                case 0:
                    printf("%d\tRTN\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tRTN\t%d\t%d", i, lex, m);
                    break;

                    // Adds the last two elements in the stack and prints the formatted output
                case 1:
                    printf("%d\tADD\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tADD\t%d\t%d", i, lex, m);
                    break;

                    // Subtracts the last two elements in the stack and prints the formatted output
                case 2:
                    printf("%d\tSUB\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tSUB\t%d\t%d", i, lex, m);
                    break;

                    // Multiplies the last two elements in the stack and prints the formatted output
                case 3:
                    printf("%d\tMUL\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tMUL\t%d\t%d", i, lex, m);
                    break;

                    // Divides the last two elements in the stack and prints the formatted output
                case 4:
                    printf("%d\tDIV\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tDIV\t%d\t%d", i, lex, m);
                    break;

                    // Compares if the last two elements in the stack are equal and prints the formatted output
                case 5:
                    printf("%d\tEQL\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tEQL\t%d\t%d", i, lex, m);
                    break;

                    // Compares if the last two elements in the stack are not equal and prints the formatted output
                case 6:
                    printf("%d\tNEQ\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tNEQ\t%d\t%d", i, lex, m);
                    break;

                    // Compares if the last element of the stack is greater than the previous element and
                    // prints the formatted output
                case 7:
                    printf("%d\tLSS\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tLSS\t%d\t%d", i, lex, m);
                    break;

                    // Compares if the last element of the stack is greater or equal than the previous element and
                    // prints the formatted output
                case 8:
                    printf("%d\tLEQ\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tLEQ\t%d\t%d", i, lex, m);
                    break;

                    // Compares if the last element of the stack is less than the previous element and
                    // prints the formatted output
                case 9:
                    printf("%d\tGTR\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tGTR\t%d\t%d", i, lex, m);
                    break;

                    // Compares if the last element of the stack is less or equal than the previous element and
                    // prints the formatted output
                case 10:
                    printf("%d\tGEQ\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tGEQ\t%d\t%d", i, lex, m);
                    break;

                case 11:
                    printf("%d\tODD\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "%d\tODD\t%d\t%d", i, lex, m);
                    break;

                default:
                    break;
                }

                printf("\n");
                fprintf(fh_output, "\n");
                break;

                // Load value to top of stack from the stack location at offset M in AR located
                // L lexicographical levels down
            case 3:
                printf("%d\tLOD\t%d\t%d", i, lex, m);
                printf("\n");

                fprintf(fh_output, "%d\tLOD\t%d\t%d", i, lex, m);
                fprintf(fh_output, "\n");
                break;

                // Store value at top of stack in the stack location at offset M In AR located
                // L lexicographical levels down
            case 4:
                printf("%d\tSTO\t%d\t%d", i, lex, m);
                printf("\n");

                fprintf(fh_output, "%d\tSTO\t%d\t%d", i, lex, m);
                fprintf(fh_output, "\n");
                break;

                // Call procedure at code index M (generates new Activation Record)
            case 5:
                printf("%d\tCAL\t%d\t%d", i, lex, m);
                printf("\n");

                fprintf(fh_output, "%d\tCAL\t%d\t%d", i, lex, m);
                fprintf(fh_output, "\n");
                break;

                // Allocate M memory words (increment SP by M)
            case 6:
                printf("%d\tINC\t%d\t%d", i, lex, m);
                printf("\n");

                fprintf(fh_output, "%d\tINC\t%d\t%d", i, lex, m);
                fprintf(fh_output, "\n");
                break;

                // Jump to instruction M
            case 7:
                printf("%d\tJMP\t%d\t%d", i, lex, m);
                printf("\n");

                fprintf(fh_output, "%d\tJMP\t%d\t%d", i, lex, m);
                fprintf(fh_output, "\n");
                break;

                // Jump to instruction M if top stack element is 0
            case 8:
                printf("%d\tJPC\t%d\t%d", i, lex, m);
                printf("\n");

                fprintf(fh_output, "%d\tJPC\t%d\t%d", i, lex, m);
                fprintf(fh_output, "\n");
                break;

                // Instruction used for interacting with the external environment depending on M
            case 9:

                // Print the top stack element to the screen
                if (m == 1)
                {
                    printf("%d\tSYS\t%d\t%d", i, lex, m);
                    printf("\n");

                    fprintf(fh_output, "%d\tSYS\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "\n");
                }

                // Read in input from the user and store it on top of the stack
                else if (m == 2)
                {
                    printf("%d\tSYS\t%d\t%d", i, lex, m);
                    printf("\n");

                    fprintf(fh_output, "%d\tSYS\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "\n");
                }

                // End of program
                else
                {
                    // Formatted output with the stack
                    printf("%d\tSYS\t%d\t%d", i, lex, m);
                    printf("\n");

                    fprintf(fh_output, "%d\tSYS\t%d\t%d", i, lex, m);
                    fprintf(fh_output, "\n");
                }

                break;

            default:
                break;

        }
    }
}   


// return the size of tokens array
int populateTokenArray(struct TokenNode* head) {
    if (head == NULL) {
        printf("Token List (Linked List) null");
        return 0;
    }

    struct TokenNode* curr = head;

    int i = 0;
    while (curr != NULL) {
        strcpy(tokens[i].lexeme, curr->lexeme);
        tokens[i].type = curr->type;
        i++;
        curr = curr->next;
    }

    return i - 1;

}

// To help debug the program
void whatAreMyPointers() {
    printf("\n");
    printf("currentTokenIndex: %d\n", currentTokenIndex);
    printf("currentToken.type: %d and currentToken.lexem: %s\n", currentToken.type, currentToken.lexeme);
    printf("nextToken.type: %d and nextToken.lexem: %s\n", nextToken.type, nextToken.lexeme);
    printf("symbolTableIndex: %d\n", symbolTableIndex);
    printf("lastToken: %d\n", lastToken);
    printf("\n");

}

int main(int argc, char* argv[])
{
    Stack s;
    FILE* filePointer;
    // FILE* fh_output;
    char* filename;

    fh_output = fopen("myOut.txt", "w");

    // if there is not enough arguments passed from the terminal
    if (argc < 2)
    {
        printf("Filename missing!\n");
        fprintf(fh_output, "Filename missing!\n");
        exit(1);
    }
    else
    {
        // get the file name from the passed argument.
        filename = argv[1];
    }

    // read and print the file
    filePointer = fopen(filename, "r");

    // if file cannot be opened properly
    if (!filePointer)
    {
        printf("Failed to open file\n");
        fprintf(fh_output, "Failed to open file\n");
        exit(1);
    }

    // Get size of file 
    fseek(filePointer, 0L, SEEK_END);
    int fileSize = ftell(filePointer);
    rewind(filePointer);

    // Initialize stack to take care of the comments
    init(&s, fileSize, fh_output);

    // Print the source file
    printFile(filePointer, fh_output);
    rewind(filePointer);

    // Main function the takes care of all tokenization
    struct TokenNode* tokenL = recognizeTokens(filePointer, fileSize, &s, fh_output);
    // printTokenList2(tokenL);
    // makeSymbolTable(tokenL);


    lastToken = populateTokenArray(tokenL);
    currentToken = tokens[0];
    nextToken = tokens[1];
    symbolTableIndex = 0;
    currentTokenIndex = 0;

    // printf("\n");
    // printf("Token List: ");
    // for (int i = 0; i < lastToken; i++) {
    //     printf("(%s, %d) ", tokens[i].lexeme, tokens[i].type);
    // }
    // printf("\n");

    PROGRAM();
    printGeneratedCode();
    printSymbolTable();


    // Deallocate the stack and close file once we are done
    deallocate(&s);
    fclose(filePointer);
    fclose(fh_output);

    return 0;
}
