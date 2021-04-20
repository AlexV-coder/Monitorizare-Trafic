#include <sys/types.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define ANSI_COLOR_RED "\x1b[31m"

#define ANSI_COLOR_GREEN "\x1b[32m"

#define ANSI_COLOR_YELLOW "\x1b[33m"

#define ANSI_COLOR_BLUE "\x1b[34m"

#define ANSI_COLOR_MAGENTA "\x1b[35m"

#define ANSI_COLOR_CYAN "\x1b[36m"

#define ANSI_COLOR_RESET "\x1b[0m"

int sd;			// descriptorul de socket

pid_t pid,pid1;
int p[2];
void sighandler(int sig)
{
    if(sig==SIGUSR1)
	    {
	        int z;
	        read(p[0],&z,4);
		}
	if(sig==SIGINT)
	    {
	        //printf("AAAAAAAAAAAAAAA\n");
	        char m[100];
	        //printf("!!!!\n");
	        strcpy(m,"kill");
	        write(sd,&m,100);
	        kill(pid,SIGINT);
	        kill(pid1,SIGINT);
	        exit(1);
	    }
}

void getRecommendedPath()
{
    int lDrum=0;
	read(sd,&lDrum,4);
	if(lDrum!=-1)
	{
	printf("Va recomandam traseul:\n\n");
	for(int i=1;i<=(2*lDrum)-2;i++)
		{
			char msg[100]={0};
			read(sd,&msg,100);
            printf(ANSI_COLOR_GREEN "%s-" ANSI_COLOR_RESET,msg);
			fflush(stdout);
		}
	char msg1[100]={0};
	read(sd,&msg1,100);
	printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n\n" ,msg1);
	}
	else
	{
	char msg1[100]={0};
	read(sd,&msg1,100);
	printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n\n" ,msg1);	
	}
}

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
    srand(time(NULL)); 
    signal(SIGUSR1,sighandler);
  struct sockaddr_in server;	// structura folosita pentru conectare

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
//	char plecare[256];
//	char sosire[256];
	//char plecare[100];
	//char sosire[100];
	int ok1=0;
	int ok2=0;
	printf(ANSI_COLOR_YELLOW "Selectati locatia de plecare: " ANSI_COLOR_RESET);
	while(ok1==0)
	{
	char plecare[100]={0};
	scanf("%s",&plecare);
	write(sd,&plecare,100);
	read(sd,&ok1,4);
	if(ok1==0)
        printf(ANSI_COLOR_YELLOW "Locatie gresita." ANSI_COLOR_RESET "\n" ANSI_COLOR_YELLOW "Introduceti o noua locatie: " ANSI_COLOR_RESET);
	}
//	scanf("%[^\n]%*c", plecare);
	printf(ANSI_COLOR_YELLOW "Selectati locatia de sosire: " ANSI_COLOR_RESET);
//	scanf("%[^\n]%*c", sosire);
	while(ok2==0)
	{
	char sosire[100]={0};
	scanf("%s",&sosire);
	write(sd,&sosire,100);
	read(sd,&ok2,4);
	if(ok2==0)
        printf(ANSI_COLOR_YELLOW "Locatie gresita." ANSI_COLOR_RESET "\n" ANSI_COLOR_YELLOW "Introduceti o noua locatie: " ANSI_COLOR_RESET);
	}
	printf(ANSI_COLOR_YELLOW "\nDoriti informatii suplimentare?\nRaspuns: " ANSI_COLOR_RESET);
	fflush(stdout);
	char info[4]={0};
	read(0,&info,4);
	write(sd,&info,4);
	getRecommendedPath();
	pipe(p);
	//pid_t pid;
	pid=fork();
	    if(pid==0)
			{
			    while(1)
			    {
				char comanda[100]={0};
				read(0,&comanda,100);
				write(sd,&comanda,100);
				}
			}
		else
			{
		//	    pid_t pid1;
			    pid1=fork();
			    if(pid1==0)
			    {
			        while(1)
			        {
				    char comanda1[100]={0};
				    read(sd,comanda1,100);
				    //printf("Comanda: %s\n",comanda1);
				    if(strstr(comanda1,"Strada blocata")!=NULL || strstr(comanda1,"Strada deblocata")!=NULL)
				        {
				        //printf("3: Am primit mesajul de la server: %s \n",comanda1);
				        int action;
				        read(sd,&action,4);
				        printf(ANSI_COLOR_RED "%s" ANSI_COLOR_RESET "\n",comanda1);
				        fflush(stdout);
				        char traseuNou[100]={0};
				        strcpy(traseuNou,"Cerere traseu nou");
				        write(sd,&traseuNou,100);
				        //sleep(10);
				        int number;
			            number=rand()%100+30;
			            char numarRandom[100]={0};
			            sprintf(numarRandom,"%d",number);
				        //printf("Action: %d\n",action);
				        getRecommendedPath();
				        write(sd,&numarRandom,100);
				      //  tcflush(STDIN_FILENO);
				        }
				        else if(strstr(comanda1,"Ati ajuns la destinatie")!=NULL)
                        {
                            	        char m[100]={0};
	                                    //printf("!!!!\n");
	                                    printf("\n" ANSI_COLOR_MAGENTA "Ati ajuns la destinatie" ANSI_COLOR_RESET "\n");
	                                    strcpy(m,"kill");
	                                    write(sd,&m,100);
                                        kill(getppid(),SIGINT);
                        }
                        else if(strstr(comanda1,"Circulati cu viteza")!=NULL)
                            {
                    //          printf("13 13 13 13 13\n");
				              printf(ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "\n",comanda1);      
				        //      clean_stdin();
                            }
                        else if(strstr(comanda1,"Traseu")!=NULL && strstr(comanda1,"Traseul")==NULL )
                        {
                            getRecommendedPath();
                        }
                        else if(strstr(comanda1,"Info")!=NULL)
                        {
                            char m[256]={0};
                            read(sd,&m,256);
                            printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n",m);
                        }
                        else if(strstr(comanda1,"oprit momentan")!=NULL)
                        {
                            printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n",comanda1);
                            kill(getppid(),SIGUSR1); 
                        }
                        else if(strstr(comanda1,"relua traseul")!=NULL)
                        {
                            printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n",comanda1);
                            int z=0;
                            write(p[1],&z,4);
                        }
				        else
				            { 
				              printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n",comanda1);
				        //    tcflush(STDIN_FILENO);
				            }
				    
				    }
			    }
			    else
			    {
			        signal(SIGINT,sighandler);
			        int number;
			        while(1)
			        {
			        int t;
			        //char comanda[100]={0};
			        //if(read(0,&comanda,100)<=0)break;
			        //write(sd,&comanda,100);
			        for(t=1;t<=15;t++)
			            sleep(1);
			        number=rand()%100+30;
			        char numarRandom[100]={0};
			        sprintf(numarRandom,"%d",number);
			        write(sd,&numarRandom,100);
			        }
			        return 0;
			    }
            }
  close (sd);
}
