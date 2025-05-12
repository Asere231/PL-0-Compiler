// This program was written by: Bryan Aneyro Hernandez

#include <stdio.h>

// Define array size as a constant
#define ARRAY_SIZE 512

// Declare global pointers
int PC;
int BP;
int SP;
int start_stack;

// Initialize array to store instructions and implement stack
int pas[ARRAY_SIZE];


/**********************************************/
/*		Find base L levels down		           */
/*							                   */
/**********************************************/
 
int base(int BP, int L)
{
	int arb = BP;	// arb = activation record base
	while ( L > 0)     //find base L levels down
	{
		arb = pas[arb];
		L--;
	}
	return arb;
}



int main()
{
    // Initializing global variable as the array pointers
    PC = 0;
    BP = 0;
    SP = BP - 1;
    int halt = 0;

    // Initialize all elements of the array to zero
    for(int j = 0; j < ARRAY_SIZE; j++)
    {
        pas[j] = 0;
    }

    // Read file and store in the array
    FILE *fp  = fopen("ELF.txt", "r");
    int i = 0;
    int OP = 0;
    int L = 0;
    int M = 0;
    int line = 0;

    while(!feof(fp))
    {
        fscanf(fp,"%d %d %d", &OP, &L, &M);
        pas[i++] = OP;
        pas[i++] = L;
        pas[i++] = M;
        line++;
    }

    // Update pointers
    BP = i;
    SP = BP - 1;
    start_stack = BP;

    // Formatting the output
    printf("\t\tPC\tBP\tSP\tstack\n");
    printf("Initial values: %d\t%d\t%d\n", PC, BP, SP);

    // Execute the VM
    while(!halt)
    {
        // Initialize and and hold the needed data to perform desirable instructions        
        int instructions = pas[PC++];
        int lex = pas[PC++];
        int m = pas[PC++];

        // Execute the needed instruction
        switch (instructions) 
        {
            // Pushes m onto the stack
            case 1:
                SP++;
                pas[SP] = m;

                // Formatted output with the stack
                printf("    LIT %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                for(int k = start_stack; k <= SP; k++)
                {
                    if(k != start_stack && k % 6 == 0)
                        printf("| ");

                    printf("%d ", pas[k]);

                }

                printf("\n");

                break;

            // Operation to be performed on the data at the top of the stack.
            case 2:

                // Perform operation depending on M
                switch (m)
                {
                    // Return from a subroutine and prints the formatted output
                    case 0:
                        SP = BP - 1;
                        BP = pas[SP + 2];
                        PC = pas[SP + 3];

                        printf("    RTN %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Adds the last two elements in the stack and prints the formatted output
                    case 1:
                        pas[SP - 1] = pas[SP - 1] + pas[SP];
                        SP--;

                        printf("    ADD %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Subtracts the last two elements in the stack and prints the formatted output
                    case 2:
                        pas[SP - 1] = pas[SP - 1] - pas[SP];
                        SP--;

                        printf("    SUB %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Multiplies the last two elements in the stack and prints the formatted output
                    case 3:
                        pas[SP - 1] = pas[SP - 1] * pas[SP];
                        SP--;

                        printf("    MUL %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Divides the last two elements in the stack and prints the formatted output
                    case 4:
                        pas[SP - 1] = pas[SP - 1] / pas[SP];
                        SP--;

                        printf("    DIV %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Compares if the last two elements in the stack are equal and prints the formatted output
                    case 5:
                        pas[SP - 1] = pas[SP - 1] == pas[SP];
                        SP--;

                        printf("    EQL %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Compares if the last two elements in the stack are not equal and prints the formatted output
                    case 6:
                        pas[SP - 1] = pas[SP - 1] != pas[SP];
                        SP--;

                        printf("    NEQ %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Compares if the last element of the stack is greater than the previous element and 
                    // prints the formatted output
                    case 7:
                        pas[SP - 1] = pas[SP - 1] < pas[SP];
                        SP--;

                        printf("    LSS %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Compares if the last element of the stack is greater or equal than the previous element and 
                    // prints the formatted output
                    case 8:
                        pas[SP - 1] = pas[SP - 1] <= pas[SP];
                        SP--;

                        printf("    LEQ %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Compares if the last element of the stack is less than the previous element and 
                    // prints the formatted output
                    case 9:
                        pas[SP - 1] = pas[SP - 1] > pas[SP];
                        SP--;
                        
                        printf("    GTR %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    // Compares if the last element of the stack is less or equal than the previous element and 
                    // prints the formatted output
                    case 10:
                        pas[SP - 1] = pas[SP - 1] >= pas[SP];
                        SP--;

                        printf("    GEQ %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;

                    case 11:
                        pas[SP - 1] = pas[SP - 1] % 2;
                        SP--;

                        printf("    ODD %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);
                        break;
                    
                    default:
                        break;
                }

                // Stack output
                for(int k = start_stack; k <= SP; k++)
                {
                    if(k != start_stack && k % 6 == 0)
                        printf("| ");

                    printf("%d ", pas[k]);
                }

                printf("\n");
                break;

            // Load value to top of stack from the stack location at offset M in AR located  
            // L lexicographical levels down
            case 3:

                SP++;
                pas[SP] = pas[base(BP, lex) + m];

                // Formatted output with the stack
                printf("    LOD %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                for(int k = start_stack; k <= SP; k++)
                {
                    if(k != start_stack && k % 6 == 0)
                        printf("| ");

                    printf("%d ", pas[k]);

                }

                printf("\n");
                break;

            // Store value at top of stack in the stack location at offset M In AR located  
            // L lexicographical levels down
            case 4:

                pas[base(BP, lex) + m] = pas[SP];
                SP--;

                // Formatted output with the stack
                printf("    STO %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                for(int k = start_stack; k <= SP; k++)
                {
                    if(k != start_stack && k % 6 == 0)
                        printf("| ");

                    printf("%d ", pas[k]);

                }

                printf("\n");
                break;

            // Call procedure at code index M (generates new Activation Record)
            case 5:

                pas[SP + 1] = base(BP, lex);
                pas[SP + 2] = BP;
                pas[SP + 3] = PC;
                BP = SP + 1;
                PC = m;

                // Formatted output with the stack
                printf("    CAL %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                for(int k = start_stack; k <= SP; k++)
                {
                    if(k != start_stack && k % 6 == 0)
                        printf("| ");

                    printf("%d ", pas[k]);

                }

                printf("\n");
                break;

            // Allocate M memory words (increment SP by M)
            case 6:

                SP += m;

                // Formatted output with the stack
                printf("    INC %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                for(int k = start_stack; k <= SP; k++)
                {
                    if(k != start_stack && k % 6 == 0)
                        printf("| ");

                    printf("%d ", pas[k]);
                }

                printf("\n");
                break;

            // Jump to instruction M 
            case 7:
                PC = m;

                // Formatted output with the stack
                printf("    JMP %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                for(int k = start_stack; k <= SP; k++)
                {
                    if(k != start_stack && k % 6 == 0)
                        printf("| ");

                    printf("%d ", pas[k]);
                }

                printf("\n");
                break;

            // Jump to instruction M if top stack element is 0
            case 8:
                
                if (pas[SP] == 0)
                    PC = m;

                SP--;

                // Formatted output with the stack
                printf("    JPC %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                for(int k = start_stack; k <= SP; k++)
                {
                    if(k != start_stack && k % 6 == 0)
                        printf("| ");

                    printf("%d ", pas[k]);
                }

                printf("\n");
                break;

            // Instruction used for interacting with the external environment depending on M
            case 9:

                // Print the top stack element to the screen
                if (m == 1)
                {
                    printf("Output result is: %d\n", pas[SP]);
                    SP--;

                    // Formatted output with the stack
                    printf("    SYS %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                    for(int k = start_stack; k <= SP; k++)
                    {
                       if(k != start_stack && k % 6 == 0)
                            printf("| ");

                        printf("%d ", pas[k]);
                    }

                    printf("\n");
                }

                // Read in input from the user and store it on top of the stack
                else if (m == 2)
                {
                    SP++;
                    printf("Please Enter an Integer: ");
                    scanf("%d", &pas[SP]);

                    // Formatted output with the stack
                    printf("    SYS %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                    for(int k = start_stack; k <= SP; k++)
                    {
                       if(k != start_stack && k % 6 == 0)
                            printf("| ");

                        printf("%d ", pas[k]);
                    }

                    printf("\n");
                }

                // End of program 
                else
                {
                    // Formatted output with the stack
                    printf("    SYS %d   %d\t%d\t%d\t%d\t", lex, m, PC, BP, SP);

                    for(int k = start_stack; k <= SP; k++)
                    {
                        if(k != start_stack && k % 6 == 0)
                            printf("| ");

                        printf("%d ", pas[k]);
                    }

                    printf("\n");
                    halt = 1;
                }

                break;

            default:
                break;

        }
    }

    fclose(fp);

    return 0;
}

