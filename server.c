//Yuly Basulto
//Ayman Nagi

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "game.h"

#define PORTNUM  4778 /* the port number the server will listen to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/

void play(int sock);
int game_mode = 0;
int players = 0;
int alternate = 1;
int is_turn[1];
char turn_done[5];
char invalid_selection[10];

#define SMHKEY ((key_t)7890)
#define SEMKEY ((key_t)400L)
#define NSEMS 1 /*defines the number of semaphores to be created*/

// Semaphore
int semnum, semval, semid, status;
union
{
	int val;
	struct semid_ds *buf;
	ushort *array;
} semctl_arg;

static struct sembuf Wait = {0, -1, 0};
static struct sembuf Signal = {0, 1, 0};

int main( int argc, char *argv[] ) {

   char player[10];
   gameValue = (shared_mem*) malloc(1 * sizeof(shared_mem));
   if(argc == 2)
   {
      if(strcmp(argv[1], "turns") == 0)
      {
         printf("Setting game to turns\n");
         game_mode = 1;
      }
   }
  // Create shared memory for all processes
	char *shmadd;
   int shmid;
	shmadd = (char *)0;
	if ((shmid = shmget(SMHKEY, sizeof(int), IPC_CREAT | 0666)) < 0)
	{
		perror("shmget");
		exit(1);
	}
   if ((gameValue = (shared_mem *)shmat(shmid, shmadd, 0)) == (shared_mem *)-1)
	{
		perror("shmat");
		exit(0);
	}
   gameValue->player_num = 0;
   // Semaphore initialization
	semid = semget(SEMKEY, NSEMS, IPC_CREAT | 0666);
	if (semid < 0)
	{
		printf("error in creating semaphore");
		exit(1);
	}

   semnum = 0;
	semctl_arg.val = 1;
	status = semctl(semid, semnum, SETVAL, semctl_arg);
	semval = semctl(semid, semnum, GETVAL, semctl_arg);

   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int status, pid;
   
   createBoard();
   printBoard();
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM,DEFAULT_PROTOCOL );
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = PORTNUM;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   status =  bind(sockfd, (struct sockaddr *) &serv_addr, sizeof	(serv_addr)); 

   if (status < 0) {
      perror("ERROR on binding");
      exit(1);
   }
   
   /* Now Server starts listening clients wanting to connect. No 	more than 5 clients allowed */
   
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, 	&clilen);
		
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      /* Create child process */
      pid = fork();

      if (pid < 0) {
         perror("ERROR on fork");
         exit(1);
      }
      
      if (pid == 0) {
         /* This is the client process */
         close(sockfd);
         play(newsockfd);
         exit(0);
      }
      else {
         close(newsockfd);
      }
		
   } /* end of while */

   free(gameValue);
}


void play (int sock) 
{
   int status;
   int local_id;
   char selection1, selection2;
   char buffer[256];
   char game_board[20];
   bool play_again = true;
   gameValue->restart = 1;
   bzero(buffer,256);

   printf("Waiting for players to ready up\n");
   status = read(sock, buffer, 5);
   buffer[strcspn(buffer, "\n")] = 0;

   if(game_mode == 1)
      status = write(sock, "1", 1);
   else
      status = write(sock, "0", 1);


   int i;
   for (i = 0; i < 18; i++)
      game_board[i] = gameValue->symbols[i];
   
   if(strcmp(buffer, "yes") == 0)
   {
      do
      {  
         if(play_again == true)
         {
            status = semop(semid, &Wait, 1);
            gameValue->player_num++;
            gameValue->won = false;
            gameValue->matchCount = 0;
            char string_player_num[1];
            snprintf(string_player_num, 1, "%d", gameValue->player_num);
            gameValue->player_turn = gameValue->player_num;
            local_id = gameValue->player_num;
            gameValue->player_score[local_id - 1] = 0;
            printf("Player %d joined the game\n", local_id);
            if(local_id == 1)
               gameValue->restart = 1;
            status = semop(semid, &Signal, 1);
            alternate = 1;
            play_again = false;
         }

         if (game_mode == 1 && (gameValue->player_num >= 2)) // turns based game
         {
            // turns based game
            while(!gameValue->won)
            {
               printf("\nPlaying with turns\n");

               if(alternate == 1)
               {
                  status = semop(semid, &Wait, 1);
                  printf("Player 1's turn\n");
                  status = write(sock, "1", 1);
                  gameValue->player_turn = 1;
                  alternate = 2;
               } 
               else if(alternate == 2)
               {
                  status = semop(semid, &Wait, 1);
                  printf("Player 2's turn\n");
                  status = write(sock, "2", 1);
                  gameValue->player_turn = 2;
                  alternate = 1;
               }

               bool isMatch = false;
               
               //give client the board
               bzero(buffer, 256);
               status = write(sock,gameValue->symbols,18);
               if (status < 0)
               {
                  perror("ERROR writing socket");
                  exit(1);
               }

               status = write(sock, gameValue->shown, 18);
               if (status < 0)
               {
                  perror("ERROR writing socket");
                  exit(1);
               }

               //get input from client
               status = read(sock, buffer, 255);
               if (status < 0)
               {
                  perror("ERROR reading from socket");
                  exit(1);
               }

               selection1 = buffer[0];
               selection2 = buffer[1];
               selection1 = toupper(selection1); // Make it uppercase to match Cards on shown array
               selection2 = toupper(selection2);
               isMatch = checkMatch(selection1, selection2);

               if (isMatch == 1 && gameValue->matchCount == 8) // If user found a match
               {
                  printf("match count = %d", gameValue->matchCount);
                  gameValue->won = true;
                  status = write(sock, "won", 255);
               }

               else if(isMatch == 1)
               {
                  status = write(sock, "match", 255);
                  gameValue->matchCount++; // Max 9 matches to be made
               }
               else
                  status = write(sock, "miss", 255);

               status = semop(semid, &Signal, 1);
            }
         }
         else if ((game_mode == 0) && (gameValue->player_num >= 2)) //not turn based
         {
            //not turn based
            while (!gameValue->won)
            {

               //give client the board
               status = write(sock, gameValue->symbols, 18);
               if (status < 0)
               {
                  perror("ERROR writing socket");
                  exit(1);
               }
               
               status = write(sock, gameValue->shown, 18);
               if (status < 0)
               {
                  perror("ERROR writing socket");
                  exit(1);
               }
               
               printf("Playing with no turns\n");
               bool isMatch = false;

               if (gameValue->won)
               {
                  int largest = gameValue->player_score[0];
                  int i;
                  for (i = 1; i < gameValue->player_num; i++)
                  {
                     if (gameValue->player_score[i] > largest)
                        largest = gameValue->player_score[i];
                  }

                  if (largest == gameValue->player_score[local_id-1])
                  {
                     status = write(sock, "won", 255);
                     continue;
                  }
                  else
                  {
                     status = write(sock, "lost", 255);
                     continue;
                  }
               }
               else
                  status = write(sock, "no", 5);


               // Get selections from the client and check if match 
               bzero(buffer, 255);
               status = read(sock, buffer, 255);
               if (status < 0)
               {
                  perror("ERROR reading from socket");
                  exit(1);
               }

               selection1 = buffer[0];
               selection2 = buffer[1];
               selection1 = toupper(selection1); // Make it uppercase to match Cards on shown array
               selection2 = toupper(selection2);

               //protecting critical section
               status = semop(semid, &Wait, 1);
               isMatch = checkMatch(selection1, selection2);
               status = semop(semid, &Signal, 1);

               if (gameValue->won)
               {
                  int largest = gameValue->player_score[0];
                  int i;
                  for (i = 1; i < gameValue->player_num; i++)
                  {
                     printf("The current value of score[i] = %d\n", gameValue->player_score[i]);
                     if (gameValue->player_score[i] > largest)
                     {
                        largest = gameValue->player_score[i];
                     }
                  }

                  printf("Game Over\n", gameValue->matchCount);
                  printf("Lsrgest is %d and player score is %d\n", largest, gameValue->player_score[local_id-1]);

                  if (largest == gameValue->player_score[local_id-1])
                     status = write(sock, "won", 255);
                  else
                     status = write(sock, "lost", 255);
               }

               else if (isMatch == 1 && gameValue->matchCount == 8) // If user found a match
               {
                  printf("\n\n***********This is a bug*************\n\n");
                  gameValue->won = true;
                  status = write(sock, "end", 255);
               }
               else if (isMatch == 1)
               {
                  status = write(sock, "match", 255);
                  gameValue->matchCount++; // Max 9 matches to be made
                  gameValue->player_score[local_id-1]++;
                  printf("Player %d gets a point, total = %d\n", local_id, gameValue->player_score[local_id-1]);
               }
               else if (gameValue->won)
               {
                  int largest = gameValue->player_score[0];
                  int i;
                  for (i = 1; i < gameValue->player_num; i++)
                  {
                     printf("The current value of score[i] = %d\n", gameValue->player_score[i]);
                     if (gameValue->player_score[i] > largest)
                     {
                        largest = gameValue->player_score[i];
                     }
                  }

                  printf("Game Over\n", gameValue->matchCount);
                  printf("Lsrgest is %d and player score is %d\n", largest, gameValue->player_score[local_id-1]);

                  if (largest == gameValue->player_score[local_id-1])
                     status = write(sock, "won", 255);
                  else
                     status = write(sock, "lost", 255);
               }
               else status = write(sock, "miss", 255);
            }
               
            if (status < 0)
            {
                  perror("2222222222222222");
                  exit(1);
            }
            bzero(buffer, 256);
            status = read(sock, buffer, 3);

            status = semop(semid, &Wait, 1);
            if(gameValue->restart == 1 && local_id == 1)
            {
               gameValue->player_num = 0;
               gameValue->restart = 0;
               createBoard();
               printBoard();
            }
            status = semop(semid, &Signal, 1);

            if (strcmp(buffer, "yes") == 0)
            {
               printf("Playing again\n");

               status = semop(semid, &Wait, 1);
               gameValue->won = false;
               gameValue->player_score[local_id - 1] = 0;
               gameValue->matchCount = 0;
               status = semop(semid, &Signal, 1);

               status = write(sock, "true", 5);

               play_again = true;
            }
            else
            {
               printf("buffer = %s\n", buffer);
               status = semop(semid, &Wait, 1);
               gameValue->player_score[local_id - 1] = 0;
               status = semop(semid, &Signal, 1);

               printf("Not playing again\n");
            }
         }
         
      }
      while(!gameValue->won);
   }
}

