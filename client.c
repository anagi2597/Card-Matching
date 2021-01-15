//Yuly Basulto
//Ayman Nagi

/* this program shows how to create sockets for a client.
it also shows how the client connects to a server socket.
and sends a message to it. the server must already be running
on a machine. The name of this machine must be entered in the function gethostbyname in the code below. The port number where the server is listening is specified in PORTNUM. This port number must also be specified in the server code.

 * main program */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include "game.h"

#define PORTNUM  4778 /* the port number that the server is listening to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/

char player_id[1];
char turns[1];
char game_mode[1];
bool player_won = false;


void main()
{
  char player_ready[5];
  int port;
  int socketid;     /*will hold the id of the socket created*/
  int status;       /* error status holder*/
  char buffer[256]; /* the message buffer*/
  struct sockaddr_in serv_addr;

  struct hostent *server;

  /* this creates the socket*/
  socketid = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
  if (socketid < 0)
  {
    printf("error in creating client socket\n");
    exit(-1);
    }

    printf("created client socket successfully\n");

   /* before connecting the socket we need to set up the right     	values in the different fields of the structure server_addr 
   you can check the definition of this structure on your own*/
   
    server = gethostbyname("osnode10"); 

   if (server == NULL)
   {
      printf(" error trying to identify the machine where the 	server is running\n");
      exit(0);
   }

   port = PORTNUM;
/*This function is used to initialize the socket structures with null values. */
   bzero((char *) &serv_addr, sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length); 
   serv_addr.sin_port = htons(port);
   
   /* connecting the client socket to the server socket */

   status = connect(socketid, (struct sockaddr *) &serv_addr, 
                            sizeof(serv_addr));
   if (status < 0)
    {
      printf( " error in connecting client socket with server	\n");
      exit(-1);
    }

   printf("connected client socket to the server socket \n");

   /* now lets send a message to the server. the message will be
     whatever the user wants to write to the server.*/
  
  char selection1;
  char selection2;
  char symbol1;
  char symbol2;
  char game_over[5];
  char symbols_table[20];
  char normal_table[18] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R'};
  bool won = false;

  fflush(stdout);
  printf("Are you ready to play? [yes/no]\n");
  bzero(player_ready, 5);
  fgets(player_ready, 5, stdin);
  status = write(socketid, player_ready, 5);
  status = read(socketid, game_mode, 1);
  //get player id
  // bzero(buffer, 256);

  // status = read(socketid, buffer, 1);
  // printf("The player id is = %d", buffer[0]);

  if (status < 0)
  {
    printf("error while writing player ready to server\n");
  }
  
  printf("Waiting for other players\n");
  while(won != true)
  {
    if(strcmp(game_mode, "1") == 0) // play with turns
    {
      // play with turns
      while (won != true)
      {
        printf("Playing with turns\n");
        bzero(turns,2);
        status = read(socketid, turns, 1);
        //if not my turn read and discard to not cause issues when
        //listening for player turn next iter

        bzero(symbols_table, 18);
        status = read(socketid, symbols_table, 18);
        if (status < 0)
        {
          printf("error while reading symbols from server\n");
        }

        bzero(normal_table, 18);
        status = read(socketid, normal_table, 18);
        if (status < 0)
        {
          printf("error while reading normal from server\n");
        }

        printf("\n\nCurrent Board:\n");
        // Prints the board of cards to show
        int i = 0;
        int j = 0;
        for (i = 0; i < SIZE; i++)
        {
          printf(" %c ", normal_table[i]);
          if (j == 5) // Print new line to make board 6x3
          {
            printf("\n\n");
            j = 0;
            continue;
          }

          j++;
        }
        
        bzero(game_over, 5);
        status = read(socketid, game_over, 5);
        if (strcmp(game_over, "won") == 0)
        {
          player_won = true;
          won = true;
          continue;
        }
        else if (strcmp(game_over, "lost") == 0)
        {
          won = true;
          continue;
        }

        fflush(stdout);
        printf("Please select two cards to match: <AB> or <ab> \n");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        selection1 = toupper(buffer[0]);
        selection2 = toupper(buffer[1]);
        fflush(stdout);

        int c1 = selection1 - 65; // Subtract 65 to get location in array
        int c2 = selection2 - 65;

          // Invalid selections
          while (c1 < 0 || c1 > 17 || c2 < 0 || c2 > 17) // || symbols_table[c1] == ' ' || symbols_table[c2] == ' ') // add check for space
          {
            printf("Invalid selection, please choose a card from the board:\n");
            bzero(buffer, 256);
            fgets(buffer, 255, stdin);
            selection1 = toupper(buffer[0]);
            selection2 = toupper(buffer[1]);

            c1 = selection1 - 65; // Subtract 65 to get location in array
            c2 = selection2 - 65;
          }
          
        // Print the selections the user made 
        for (i = 0; i < SIZE; i++)
        {
          if (selection1 == normal_table[i] || selection2 == normal_table[i])
          {
            printf(" %c ", symbols_table[i]);
          }

          else
          {
            printf(" %c ", normal_table[i]);
          }

          if (j == 5) // Print new line to make board 6x3
          {
            printf("\n\n");
            j = 0;
            continue;
          }

          j++;
        }

        // Give the server the user's choices
        status = write(socketid, buffer, strlen(buffer));
        if (status < 0)
        {
          printf("error while writing to server\n");
        }

        bzero(buffer, 256);
        status = read(socketid, buffer, 255);
        if (status < 0)
        {
          printf("error while reading server\n");
        }

        if (strcmp(buffer, "match") == 0)
        {
          printf("\nThere's a match!\n");
          normal_table[c1] = ' ';
          normal_table[c2] = ' ';
        }
        else if (strcmp(buffer, "won") == 0)
        {
          player_won = true;
          printf("\nGame ended\n");
          won = true;
        }
        else if (strcmp(buffer, "end") == 0)
        {
          printf("\nGame ended\n");
          won = true;
        }
        else if(strcmp(buffer, "lost") == 0)
        {
          printf("\nYou Lost :(\n");
          won = true; //just to end the game
        }
        else // "miss"
          printf("\nNo match sorry!\n");
      }
      
    }
    else // no turns
    {
      // no turns
      while (won != true)
      {
        bzero(symbols_table, 18);
        status = read(socketid, symbols_table, 18);
        if (status < 0)
        {
          printf("error while reading symbols from server\n");
        }

        bzero(normal_table, 18);
        status = read(socketid, normal_table, 18);
        if (status < 0)
        {
          printf("error while reading normal from server\n");
        }

        printf("\n\nCurrent Board:\n");
        // Prints the board of cards to show
        int i = 0;
        int j = 0;
        for (i = 0; i < SIZE; i++)
        {
          printf(" %c ", normal_table[i]);
          if (j == 5) // Print new line to make board 6x3
          {
            printf("\n\n");
            j = 0;
            continue;
          }

          j++;
        }

        bzero(game_over, 5);
        status = read(socketid, game_over, 5);
        if (strcmp(game_over, "won") == 0)
        {
          player_won = true;
          won = true;
          continue;
        }
        else if (strcmp(game_over, "lost") == 0)
        {
          won = true;
          continue;
        }

        fflush(stdout);
        printf("Please select two cards to match: <AB> or <ab> \n");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        selection1 = toupper(buffer[0]);
        selection2 = toupper(buffer[1]);
        fflush(stdout);

        int c1 = selection1 - 65; // Subtract 65 to get location in array
        int c2 = selection2 - 65;

          // Invalid selections
          while (c1 < 0 || c1 > 17 || c2 < 0 || c2 > 17) // || symbols_table[c1] == ' ' || symbols_table[c2] == ' ') // add check for space
          {
            printf("Invalid selection, please choose a card from the board:\n");
            bzero(buffer, 256);
            fgets(buffer, 255, stdin);
            selection1 = toupper(buffer[0]);
            selection2 = toupper(buffer[1]);

            c1 = selection1 - 65; // Subtract 65 to get location in array
            c2 = selection2 - 65;
          }
          
        // Print the selections the user made 
        for (i = 0; i < SIZE; i++)
        {
          if (selection1 == normal_table[i] || selection2 == normal_table[i])
          {
            printf(" %c ", symbols_table[i]);
          }

          else
          {
            printf(" %c ", normal_table[i]);
          }

          if (j == 5) // Print new line to make board 6x3
          {
            printf("\n\n");
            j = 0;
            continue;
          }

          j++;
        }

        // Give the server the user's choices
        status = write(socketid, buffer, strlen(buffer));
        if (status < 0)
        {
          printf("error while writing to server\n");
        }

        bzero(buffer, 256);
        status = read(socketid, buffer, 255);
        if (status < 0)
        {
          printf("error while reading server\n");
        }

        if (strcmp(buffer, "match") == 0)
        {
          printf("\nThere's a match!\n");
          normal_table[c1] = ' ';
          normal_table[c2] = ' ';
        }
        else if (strcmp(buffer, "won") == 0)
        {
          player_won = true;
          printf("\nGame ended\n");
          won = true;
        }
        else if (strcmp(buffer, "end") == 0)
        {
          printf("\nGame ended\n");
          won = true;
        }
        else if(strcmp(buffer, "lost") == 0)
        {
          printf("\nYou Lost :(\n");
          won = true; //just to end the game
        }
        else // "miss"
          printf("\nNo match sorry!\n");
      }
        // play again originally here
    }
  
    if (player_won)
    {
      printf("Congratulations!! You Win!!\n");
      player_won = false;
    }
    else
      printf("You Lose\n");



    printf("Would you like to keep playing?[yes/no]\n");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    printf("the value of buffer is %s\n", buffer);
    buffer[3] = '\0';

    if (strcmp(buffer, "yes") == 0)
    {
      status = write(socketid, "yes", 3);

      bzero(buffer, 256);
      status = read(socketid, buffer, 5);

      if (strcmp(buffer, "true") == 0 || strcmp(buffer, "won") == 0)
      {
        printf("buffer = %s\n", buffer);
        won = false;
      }
      else
      {
        printf("else is fucekd buffer = %s\n", buffer);
        won = true;
      }
      
    }
    else
    {
      status = write(socketid, "no", 3);
    }
  }

  close(socketid);
}
