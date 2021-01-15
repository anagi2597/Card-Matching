#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>

#define SIZE 18

// Shared memory structure
typedef struct
{
	char shown[SIZE];
    char player_ready[5];
    int player_score[5];
    int player_turn;
    char symbols[SIZE]; 
    bool won;
    bool takeTurns;
    int player_num;
    int player1Points;
    int player2Points;
    int matchCount;
    int restart;
} shared_mem;
shared_mem *gameValue;

void createBoard();
void printBoard();
void playGameWithTurns();
void playGameWithoutTurns();
bool checkMatch(char a, char b);
void printChoices(int a, int b);

char temp_symbol[SIZE] = {'=', '&', '?', '#', '!', '$', '*', '@', '%', '=', '&', '?', '#', '!', '$', '*', '@', '%'}; // Symbols used in the hidden array, 9 unique symbols, 18 total
char temp_shown[SIZE] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R'};   // Cards to show the users to select

void createBoard()
{
    // Assigning values to gameValue
    gameValue->won = false;
    gameValue->takeTurns = true;
    gameValue->player1Points = 0;
    gameValue->player2Points = 0;
    gameValue->matchCount = 0;
    int i = 0;
    for(;i < 5; i++);
        gameValue->player_score[i] = 0;
    strcpy(gameValue->symbols, temp_symbol);
    strcpy(gameValue->shown, temp_shown);

    for(i = 0; i < SIZE; i++)
    {
        gameValue->symbols[i] = temp_symbol[i];
    }

    // Randomizing placement of hidden symbols
    srand(time(0));
    for (i = 0; i < SIZE; i++)
    {
        int j = i + rand() / (RAND_MAX / (SIZE - i) + 1);
        int t = gameValue->symbols[j];
        gameValue->symbols[j] = gameValue->symbols[i];
        gameValue->symbols[i] = t;
    }  
}

void printBoard()
{
    int i = 0, j = 0;
    printf("\n**** Letters ****\n");

    // Prints the board of cards to show
    for (i = 0; i < SIZE; i++)
    {
        printf(" %c ", gameValue->shown[i]);
        if (j == 5) // Print new line to make board 6x3
        {
            printf("\n\n");
            j = 0;
            continue;
        }

        j++;
    }

    printf("\n**** Symbols ****\n");

    // Prints the board of cards to show
    for (i = 0; i < SIZE; i++)
    {
        printf(" %c ", gameValue->symbols[i]);
        if (j == 5) // Print new line to make board 6x3
        {
            printf("\n\n");
            j = 0;
            continue;
        }

        j++;
    }
}


void playGameWithTurns()
{
    char selection1, selection2;

    while (!gameValue->won)
    {
        bool isMatch = false;

        fflush(stdout);

    //pthread_mutex_lock(&mutex);
        printf("\n\nCurrent Board:");
        printBoard();
        printf("Please select two cards to match: \n");

        scanf("%c %c", &selection1, &selection2);
        fflush(stdin); // Flush buffer so input doesn't carry over

        selection1 = toupper(selection1); // Make it uppercase to match Cards on shown array
        selection2 = toupper(selection2);

        isMatch = checkMatch(selection1, selection2);
    //pthread_mutex_unlock(&mutex);

        if (isMatch == 1)//&& pthread_self() == th1 ) // If user found a match
        {
            gameValue->matchCount++; // Max 9 matches to be made
            gameValue->player1Points++;
        }
        
        else if (isMatch == 1) //&& pthread_self() == th2)
        {
            gameValue->matchCount++; // Max 9 matches to be made
            gameValue->player2Points++;
        }

        // printf("\n\n");

        if (gameValue->matchCount == 9) // && pthread_self() == th1) // If all matches found, end game
        {
            printf("Player 1 won!");
            gameValue->won = true;
        }

        else if (gameValue->matchCount == 9) //&& pthread_self() == th2)
        {
            printf("Player 2 won!");
            gameValue->won = true;
        }
        
        sleep(2); // Give time in between printing boards
    }
}
    

void playGameWithoutTurns()
{
    //int i = 0;
    //int matchCount = 0; 
    bool isMatch = 0;
    char selection1, selection2;

    while (!gameValue->won)
    {
        fflush(stdout);

        printf("\n\nCurrent Board:");
        printBoard();
        printf("Please select two cards to match: \n");

        scanf("%c %c", &selection1, &selection2);
        fflush(stdin); // Flush buffer so input doesn't carry over

        selection1 = toupper(selection1); // Make it uppercase to match Cards on shown array
        selection2 = toupper(selection2);

    //pthread_mutex_lock(&mutex);
        isMatch = checkMatch(selection1, selection2);
    //pthread_mutex_unlock(&mutex);

        if (isMatch == true)// && pthread_self() == th1 ) // If user found a match
        {
            gameValue->matchCount++; // Max 9 matches to be made
            gameValue->player1Points++;
        }
        
        else if (isMatch == true)//  && pthread_self() == th2)
        {
            gameValue->matchCount++; // Max 9 matches to be made
            gameValue->player2Points++;
        }

        // printf("\n\n");

        if (gameValue->matchCount == 9 && gameValue->player1Points > gameValue->player2Points) // If all matches found, end game
        {
            printf("Player 1 won!");
            gameValue->won = true;
        }

        else if (gameValue->matchCount == 9 && gameValue->player2Points > gameValue->player1Points)
        {
            printf("Player 2 won!");
            gameValue->won = true;
        }
        
        sleep(2); // Give time in between printing boards
    }
}

bool checkMatch(char a, char b)
{
    int c1 = a - 65; // Subtract 65 to get location in array
    int c2 = b - 65;
    bool isMatch = false; // No match

    if(gameValue->won == true)
        false;

    // Invalid selections
    if (c1 < 0 || c1 > 17 || c2 < 0 || c2 > 17 || gameValue->symbols[c1] == ' ' || gameValue->symbols[c2] == ' ') // add check for space
    {
        printf("Invalid selection, please choose a card from the board.\n");
        return isMatch;
    }

    //printf("%d %d", c1, c2);

    // Print the two hidden cards they selected
    printChoices(c1, c2);

    // If the two cards match, remove the cards and just print a space so they don't select it anymore
    if (gameValue->symbols[c1] == gameValue->symbols[c2])
    {
        printf("Its a match! \n\n");

        gameValue->shown[c1] = ' '; 
        gameValue->shown[c2] = ' ';
        gameValue->symbols[c1] = ' ';
        gameValue->symbols[c2] = ' ';

        isMatch = true; // Found match
    }

    else
    {
        printf("Sorry, try again! \n\n");
    }

    return isMatch;
}

void printChoices(int a, int b)
{
    // Shows the board and only reveals the two cards that a player selected
    // printf("\n\n");
    int i = 0, j = 0;
    for (i = 0; i < SIZE; i++)
    {
        // If the index is at either of the two user choices print the corresponding symbol from hidden array
        if (i == a || i == b)
        {
            printf(" %c ", gameValue->symbols[i]);
        }

        // Else print the normal cards of the shown array
        else
        {
            printf(" %c ", gameValue->shown[i]);
        }

        if (j == 5) // Format for 6x3 board
        {
            printf("\n\n");
            j = 0;
            continue;
        }
        j++;
    }

    printf("\n");
    sleep(1);
}

#endif