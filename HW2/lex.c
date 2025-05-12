/* This program was written by:
    - Roosevelt Desire
    - Bryan Aneyro Hernandez */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

    // Define token types as enumerated values
enum TokenType
{
    skipsym = 1, identsym = 2, numbersym = 3,
    plussym = 4, minussym = 5, multsym = 6, slashsym = 7,
    ifelsym = 8, eqlsym = 9, neqsym = 10, lessym = 11,
    leqsym = 12, gtrsym = 13, geqsym = 14, lparentsym = 15,
    rparentsym = 16, commasym = 17, semicolonsym = 18,
    periodsym = 19, becomessym = 20, beginsym = 21,
    endsym = 22, ifsym = 23, thensym = 24, whilesym = 25,
    dosym = 26, callsym = 27, constsym = 28, varsym = 29,
    procsym = 30, writesym = 31, readsym = 32, elsesym = 33
};

// Define a structure to represent tokens
struct Token
{
    int type;      // Token type (enum value)
    char lexeme[12]; // Lexeme (up to 11 characters)
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

// prototype declarations
void init(Stack*, int size);
void push(Stack*, char);
char pop(Stack*);
void deallocate(Stack*);

// ================== Stack Functions to Help with Comments ====================//

void push(Stack* sp, char value)
{
    if (sp->top == sp->size - 1)
    {
        printf("Stack overflow\n");
        return;
    }

    sp->top++;
    sp->item[sp->top] = value;
}

char pop(Stack* sp)
{
    if (sp->top == -1)
    {
        printf("Stack Underflow\n");
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
void init(Stack* sp, int size)
{
    sp->top = -1;
    // using malloc allocating the stack dynamically

    sp->item = (char*)malloc(sizeof(char) * size);
    if (sp->item == NULL)
    {
        printf("Unable to allocate memory\n");
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
void printTokenList(struct TokenNode* head) 
{
    printf("Token List:\n");

    struct TokenNode* current = head;
    while (current != NULL) 
    {
        if (current->type != 0)
            printf("%d ", current->type);

        if(current->type == 2 || current->type == 3)
            printf("%s ",current->lexeme);
        
        current = current->next;
    }
    printf("\n");
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
        "if", "then", "ifel", "else", "while", "do", "read", "write"
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
void recognizeTokens(FILE* sourceFile, int size, Stack* s)
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

    printf("Lexeme Table: \n");
    printf("\n");

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
                printf("Name too long\n");
                lexeme[0] = '\0';
                lexemeIndex = 0;
                token.type = 0;
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
                        token.type = procsym;
                    }
                    else if (strcmp(lexeme, "call") == 0)
                    {
                        token.type = callsym;
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
                        token.type = ifelsym;
                    }
                    else if (strcmp(lexeme, "else") == 0)
                    {
                        token.type = elsesym;
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
                printf("Wrong special character\n");
                lexeme[0] = '\0';
                lexemeIndex = 0;
                token.type = 0;
                continue;
            }

            lexeme[0] = '\0';
            lexeme[lexemeIndex++] = currentChar;

            while (ispunct(currentChar = fgetc(sourceFile)))
            {
                if (!isSpecialChar(currentChar))
                {
                    printf("Wrong special character\n");
                    lexeme[0] = '\0';
                    lexemeIndex = 0;
                    token.type = 0;
                    continue;
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
                    push(s, currentChar);

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
                    printf("Wrong special character\n");

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
                printf("Number too long\n");
                lexeme[0] = '\0';
                token.type = 0;
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

        if(token.type != 0)
        {
            printf("%s", lexeme);
            printf("\t %d\n", token.type);
        }

        insertTokenNode(&tokenList, lexeme, token.type); // Insert into linked list


        if (currentChar == EOF)
            break;

        fseek(sourceFile, -1L, SEEK_CUR);

        lexemeIndex = 0;

    }

    printf("\n");
    printTokenList(tokenList);
    freeTokenList(tokenList);

}

void printFile(FILE* fp)
{
    char currentChar;

    printf("Source Program: \n");

    while ((currentChar = fgetc(fp)) != EOF)
    {
        printf("%c", currentChar);
    }

    printf("\n");
    printf("\n");
}

int main(int argc, char* argv[])
{
    Stack s;
    FILE* filePointer;
    char* filename;

    // if there is not enough arguments passed from the terminal
    if (argc < 2)
    {
        printf("Filename missing!\n");
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
        exit(1);
    }

    // Get size of file 
    fseek(filePointer, 0L, SEEK_END);
    int fileSize = ftell(filePointer);
    rewind(filePointer);

    // Initialize stack to take care of the comments
    init(&s, fileSize);

    // Print the source file
    printFile(filePointer);
    rewind(filePointer);

    // Main function the takes care of all tokenization
    recognizeTokens(filePointer, fileSize, &s);

    // Deallocate the stack and close file once we are done
    deallocate(&s);
    fclose(filePointer);

    return 0;
}
