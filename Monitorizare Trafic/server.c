#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>

#define PORT 4048

extern int errno;
int idDrum[11][11]={0};
int lungimeDrum[11][11]={0};
int vitezaMaxima[11][11]={0};
int drumMinim[11][11]={0};
int drumBlocat[11][11]={0};
int politie[11][11]={0};
int politie_raportata[11][11]={0};
int clientIndex=0;
int client[100]={0};
int uclient[100]={0};
int q[100][2];
int w[2];
int p[2];
int pipeIndex=0;

void calculDrumMinim()
{
int i,j,k;
for(i=1;i<=10;i++)
		for(j=1;j<=10;j++)
			{
			if(drumBlocat[i][j]==0)drumMinim[i][j]=lungimeDrum[i][j];
			    else drumMinim[i][j]=9999999;
			}
for(k=1;k<=10;k++)
		{for(i=1;i<=10;i++)
			{for(j=1;j<=10;j++)
				{
				    if( drumMinim[i][k] + drumMinim[k][j] < drumMinim[i][j] )
				    	drumMinim[i][j]=drumMinim[i][k]+drumMinim[k][j];
		        }
			}
				}
}

int stradaToId(char *msg)
{
    int g=open("id.txt",O_RDONLY);
    char ch;
			while(read(g,&ch,1))
				{
					int i=0;
					char txt[100]={0};
					while(ch!='\n'){
						txt[i++]=ch;
						read(g,&ch,1);
							}
					int j=0;
					char index[100]={0};
					while(txt[j]>='0' && txt[j]<='9')
						{
							index[j]=txt[j];
							j++;
						}
					strcpy(txt,txt+j+1);
					if(strstr(msg,txt)!=NULL)
						{
						    return atoi(index);
						}
				}
				return -1;
}

void sighandler(int sig)
{
	pid_t pid;
	if(sig==SIGUSR2)
	    {
	        char msg[100]={0};
	        read(w[0],&msg,100);
	        if(strstr(msg,"kill")!=NULL)
	        {
                int pipeIndex1;
                read(w[0],&pipeIndex1,4);
                uclient[pipeIndex1]=1;
            }
            else if(strstr(msg,"Raporteaza politie")!=NULL)
            {
                int str,i,j,c1,c2;
                int pipeIndex1;
                read(w[0],&pipeIndex1,4);
                read(q[pipeIndex1][0],&str,4);
                for(i=1;i<=10;i++)
                    for(j=1;j<=10;j++)
                    if(idDrum[i][j]==str)
                        {
                            politie[i][j]++;
                            c1=i;
                            c2=j;
					        break; 
					    }
                char avt[100];
                strcpy(avt,"\n");
                sprintf(avt+1,"%d",politie[c1][c2]);
			    strcat(avt," persoane au raportat politie pe strada ");
			    char street[100]={0};
			    char ch;
			    int f=open("id.txt",O_RDONLY);
                while(read(f,&ch,1))
				{
					int i=0;
					char txt[100]={0};
					while(ch!='\n'){
						txt[i++]=ch;
						read(f,&ch,1);
							}
					int j=0;
					char index[100]={0};
					while(txt[j]>='0' && txt[j]<='9')
						{
							index[j]=txt[j];
							j++;
						}
					j--;
					if(atoi(index)==idDrum[c1][c2])
						{
							strcpy(street,txt+j+2);
							strcat(avt,street);
							strcat(avt,".\n");
							break;
						}
				}
                pid_t pid;
                pid=fork();
	            if(pid==0)
		        {
				for(int i=0;i<clientIndex;i++)
			        {
			            if(uclient[i]==0)write(client[i],&avt,100);
			        }
				exit(0);
				}
            }
             else if(strstr(msg,"Politie raportata")!=NULL)
             {
                int str,i,j,c1,c2;
                int pipeIndex1;
                read(w[0],&pipeIndex1,4);
                read(q[pipeIndex1][0],&str,4);
                for(i=1;i<=10;i++)
                    for(j=1;j<=10;j++)
                    if(idDrum[i][j]==str && i<=j )
                    {
                        char avr[100]={0};
                        strcpy(avr,"\n");
                        sprintf(avr+1,"%d",politie[i][j]);
                        strcat(avr," persoane au raportat politie pe strada dumnevoastra.\n");
                        if(uclient[pipeIndex1]==0)write(client[pipeIndex1],&avr,100);
                    }                
             }
	    }
	else
	{
	if(sig==SIGUSR1)
	        {
	            int r[2];
	            int action;
	            pipe(r);
	            dup2(p[1],r[1]);
	            dup2(p[0],r[0]);
				char msg[100]={0};
				char msg1[100]={0};
				int pipeIndex1;
				read(r[0],&pipeIndex1,4);
				read(q[pipeIndex1][0],&msg,100);
				if(strstr(msg,"Strada deblocata")!=NULL){ action=1; strcpy(msg1,msg+17); }
				else if(strstr(msg,"Strada blocata")!=NULL){ action=0; strcpy(msg1,msg+15); }
				if(action==0)
				        {
				int stradaB=stradaToId(msg1);
				if(stradaB!=-1){
				for(int i=1;i<=10;i++)
				    for(int j=1;j<=10;j++)
				        if(idDrum[i][j]==stradaB)
				            drumBlocat[i][j]=1;
				calculDrumMinim();
				pid_t pid;
	            pid=fork();
	            if(pid==0)
		        {
				for(int i=0;i<clientIndex;i++)
			       {
			            if(uclient[i]==0)write(client[i],&msg,100);
			            if(uclient[i]==0)write(client[i],&action,4);
			        }
			    for(int i=0;i<clientIndex;i++)
			       {
			            if(uclient[i]==0)write(q[i][1],&action,4);
			            if(uclient[i]==0)write(q[i][1],&stradaB,4);
			       }
			       
				exit(0);
				}
			                }
			    else
			    {
			        char msg[100]={0};
			        strcpy(msg,"Strada gresita\n");
			        write(client[pipeIndex1],&msg,100);
			    }
			    
			            }
			       else if(action==1)
			       {
			            int stradaD=stradaToId(msg1);
			            if(stradaD!=-1){
			                for(int i=1;i<=10;i++)
			                    for(int j=1;j<=10;j++)
			                        if(idDrum[i][j]==stradaD)
			                            drumBlocat[i][j]=0;
			            calculDrumMinim();
			            pid=fork();
			            if(pid==0)
			            {
			            for(int i=0;i<clientIndex;i++)
			        {
			            if(uclient[i]==0)write(client[i],&msg,100);
			            if(uclient[i]==0)write(client[i],&action,4);
			        }
			    for(int i=0;i<clientIndex;i++)
			       {
			            if(uclient[i]==0)write(q[i][1],&action,4);
			            if(uclient[i]==0)write(q[i][1],&stradaD,4);    
			       }
				exit(0);
			            }
			          }
			          else
			          {
			             char msg[100]={0};
			             strcpy(msg,"Strada gresita\n");
			             write(client[pipeIndex1],&msg,100);
			          }
			       }
			            
		}
	}
}

void shortestPath(int punctCurent,int sosire,int* drumScurt)
{
	int c=0;
	int k,ok=0;
	drumScurt[c]=punctCurent;
	c++;
	while(punctCurent!=sosire && ok==0 )
	{
		for(k=1;k<=10;k++)
			if(ok==0)
				if(lungimeDrum[k][punctCurent]+drumMinim[k][sosire]==drumMinim[punctCurent][sosire] && punctCurent!=k && drumBlocat[k][punctCurent]==0)
					{
						drumScurt[c]=k;
						c++;
						punctCurent=k;
						if(punctCurent==sosire)ok=1;
							break;
					}
	}
}

void recommendPathToClient(int* drumScurt,int* client,int clientIndex)
{
	int f=open("id.txt",O_RDONLY);
	int g=open("locatii.txt",O_RDONLY);
	int k;
	int lDrum=0;
	for(k=0;drumScurt[k]!=0;k++)
		lDrum++;
	write(client[clientIndex],&lDrum,4);
	for(k=0;drumScurt[k]!=0;k++)
		{
			char msg[100]={0};
			char ch;
			while(read(g,&ch,1))
				{
					int i=0;
					char txt[100]={0};
					while(ch!='\n'){
						txt[i++]=ch;
						read(g,&ch,1);
							}
					int j=0;
					char index[100]={0};
					while(txt[j]>='0' && txt[j]<='9')
						{
							index[j]=txt[j];
							j++;
						}
					j--;
					if(atoi(index)==drumScurt[k])
						{
							strcpy(msg,txt+j+3);
							write(client[clientIndex],&msg,100);
							break;
						}
				}
			lseek(g,0,SEEK_SET);
			int id=idDrum[drumScurt[k]][drumScurt[k+1]];
			char msg1[100]={0};
			while(read(f,&ch,1))
				{
					int i=0;
					char txt[100]={0};
					while(ch!='\n'){
						txt[i++]=ch;
						read(f,&ch,1);
							}
					int j=0;
					char index[100]={0};
					while(txt[j]>='0' && txt[j]<='9')
						{
							index[j]=txt[j];
							j++;
						}
					j--;
					if(atoi(index)==id)
						{
							strcpy(msg1,txt+j+2);
							write(client[clientIndex],&msg1,100);
							break;
						}
				}
			lseek(f,0,SEEK_SET);
		}
}

void idToLocatie(int id,char* msg)
{   
            char ch;
            int g=open("locatii.txt",O_RDONLY);
            while(read(g,&ch,1))
				{
					int i=0;
					char txt[100]={0};
					while(ch!='\n'){
						txt[i++]=ch;
						read(g,&ch,1);
							}
					int j=0;
					char index[100]={0};
					while(txt[j]>='0' && txt[j]<='9')
						{
							index[j]=txt[j];
							j++;
						}
					j--;
					if(atoi(index)==id)
						{
							strcpy(msg,txt+j+3);
							break;
						}
                }
}

void locatieToId(char* p,int* nod)
{           
            char ch;
            char msg[100]={0};
            int g=open("locatii.txt",O_RDONLY);
			while(read(g,&ch,1))
				{
					int i=0;
					char txt[100]={0};
					while(ch!='\n'){
						txt[i++]=ch;
						read(g,&ch,1);
							}
					int j=0;
					char index[100]={0};
					while(txt[j]>='0' && txt[j]<='9')
						{
							index[j]=txt[j];
							j++;
						}
					j--;
					strcpy(msg,txt+j+3);
					if(strcmp(msg,p)==0)
					{
					    *nod=atoi(index);
					}
				}
}

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;
  int sd;			//descriptorul de socket
  int i,j,k;
  pipe(p);
  pipe(w);
  for(int o=0;o<=99;o++)
  {
    pipe(q[o]);
  }
  pid_t pid;

	//populare drumuri
	for(i=1;i<=10;i++)
		for(j=1;j<=10;j++)
			{
			lungimeDrum[i][j]=999999;
			lungimeDrum[j][i]=999999;
			if(i==j)
				lungimeDrum[i][j]=0;
			}
  	idDrum[1][2]=1;idDrum[1][3]=2;idDrum[2][4]=3;idDrum[3][4]=4;idDrum[3][5]=5;idDrum[5][6]=6;idDrum[6][7]=7;idDrum[7][8]=8;idDrum[6][8]=9;idDrum[4][8]=10;idDrum[8][10]=11;idDrum[9][10]=12;idDrum[4][9]=13;
	lungimeDrum[1][2]=3;lungimeDrum[1][3]=4;lungimeDrum[2][4]=2;lungimeDrum[3][4]=5;lungimeDrum[3][5]=10;lungimeDrum[5][6]=8;lungimeDrum[6][7]=3;lungimeDrum[7][8]=2;lungimeDrum[6][8]=6;lungimeDrum[4][8]=15;lungimeDrum[8][10]=1;lungimeDrum[9][10]=2;lungimeDrum[4][9]=7;
	vitezaMaxima[1][2]=50;vitezaMaxima[1][3]=60;vitezaMaxima[2][4]=50;vitezaMaxima[3][4]=70;vitezaMaxima[3][5]=90;vitezaMaxima[5][6]=90;vitezaMaxima[6][7]=50;vitezaMaxima[7][8]=50;vitezaMaxima[6][8]=70;vitezaMaxima[4][8]=130;vitezaMaxima[8][10]=30;vitezaMaxima[9][10]=50;vitezaMaxima[4][9]=90;
  	for(i=1;i<=10;i++)
		for(j=1;j<=10;j++)
			if(idDrum[i][j]!=0){
				idDrum[j][i]=idDrum[i][j];
				lungimeDrum[j][i]=lungimeDrum[i][j];
					}
	for(i=1;i<=10;i++)
		for(j=1;j<=10;j++)
			{
				drumMinim[i][j]=lungimeDrum[i][j];
				if(vitezaMaxima[i][j]!=0)vitezaMaxima[j][i]=vitezaMaxima[i][j];
			}
	calculDrumMinim();
	//populare drumuri
  // crearea unui socket
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }

    // pregatirea structurilor de date
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);

  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  if (listen (sd, 5) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }

    signal(SIGUSR1,sighandler);
    signal(SIGUSR2,sighandler);
  while (1)
    {
      int length = sizeof (from);

      client[clientIndex] = accept (sd, (struct sockaddr *) &from, &length);

      if (client[clientIndex] < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
      if( (pid = fork() ) == -1 )
	perror("Eroare la fork");

	else
		if( pid == 0 )
		{ // Aici se preia comanda clientului
			int vitezaCurenta;
			int drumCurent;
			int nodInitial=0;
			float dplus=0;
			int pipeIndex1=pipeIndex;
			int nodFinal=0;
			int destPeco=0;
			int contor=0;
			//char plecare[100]={0};
			//char sosire[100]={0};
            char informatii[4]={0};
            int idPeco=0;
			int info;
			int intrebare1=0;
			int intrebare2=0;
			int pso=0;
			for(int o=0;o<=99;o++)
			    if(o!=pipeIndex1)close(q[o][1]);
			while(nodInitial==0)
			{
			    char plecare1[100]={0};
			    read(client[clientIndex],&plecare1,100);//citesc locatia de plecare
			    locatieToId(plecare1,&nodInitial);
			    write(client[clientIndex],&nodInitial,4);
			}
			while(nodFinal==0)
			{
			    char sosire1[100]={0};
			    read(client[clientIndex],&sosire1,100);//citesc locatia de plecare
			    locatieToId(sosire1,&nodFinal);
			    write(client[clientIndex],&nodFinal,4);
			}
			read(client[clientIndex],&informatii,4);
			if(strstr(informatii,"Da")!=NULL)info=1;
			    else info=0;
			int drumScurt[31]={0};
			shortestPath(nodInitial,nodFinal,drumScurt);
			recommendPathToClient(drumScurt,client,clientIndex);
			dplus=lungimeDrum[drumScurt[0]][drumScurt[1]];
			drumCurent=idDrum[drumScurt[0]][drumScurt[1]];
			while(1)
			{
				char comanda[100]={0};
				read(client[clientIndex],&comanda,100);
				if(strstr(comanda,"Strada blocata")!=NULL || strstr(comanda,"Strada deblocata")!=NULL )
					{
					    int r[2];
					    pipe(r);
					    dup2(p[1],r[1]);
					    dup2(p[0],r[0]);
						kill(getppid(),SIGUSR1);
						write(r[1],&pipeIndex1,4);
						write(q[pipeIndex1][1],&comanda,100);
						close(r[1]);
						close(r[0]);
					}
				    else if(strstr(comanda,"kill")!=NULL)
					{
					    char msg[100]={0};
					    strcpy(msg,"kill");
					    kill(getppid(),SIGUSR2);
					    write(w[1],&msg,100);
					    write(w[1],&pipeIndex1,4);
					    exit(0);     
					}
					else if(strstr(comanda,"Cerere traseu nou")!=NULL)
					    {
					        char mesaj[100]={0};
					        char viteza[100]={0};
					        char mesaj1[100]={0};
					        int bsd;
					        read(q[pipeIndex1][0],&bsd,4);
					        int stradaB;
					        int stradaD;
					        if(bsd==0)
					        read(q[pipeIndex1][0],&stradaB,4);
					        if(bsd==1)
					        read(q[pipeIndex1][0],&stradaD,4);
					        if(bsd==1 && drumBlocat[drumScurt[contor]][drumScurt[contor+1]]==1 && idDrum[drumScurt[contor]][drumScurt[contor+1]]==stradaD)
					            vitezaCurenta=0;
					        for(i=1;i<=10;i++)
					            for(j=1;j<=10;j++)
					               {
					                if(bsd==0)if(idDrum[i][j]==stradaB)
					                {
					                    drumBlocat[i][j]=1;
					                    drumBlocat[j][i]=1;
					                }
					                if(bsd==1)if(idDrum[i][j]==stradaD)
					                {
					                    drumBlocat[i][j]=0;
					                    drumBlocat[j][i]=0;
					                } 
					               }
					        if(drumBlocat[drumScurt[contor]][drumScurt[contor+1]]==1)
					        {   
					            //int action=0;
					            calculDrumMinim();
					            //write(client[clientIndex],&action,4);
					            strcpy(mesaj,"Sunteti pe drumul blocat, vom astepta deblocarea acestuia.\n");
					            vitezaCurenta=-1;
					            int lunDrum=-1;
					            write(client[clientIndex],&lunDrum,4);   
					            write(client[clientIndex],&mesaj,100);
					            if(intrebare1==1){char msg[100]={0};strcpy(msg,"Ati ajuns la Peco.\nDoriti sa reluati drumul?\n"); write(client[clientIndex],&msg,100);}
				                if(intrebare2==1)
				                {
				                    char msg[100]={0};
					                strcpy(msg,"Sunteti la ");
					                char dplp[10]={0};
					                char rasp[100]={0};
					                sprintf(dplp,"%d",drumMinim[drumScurt[contor+1]][idPeco]);
					                strcat(msg,dplp);
					                strcat(msg," km distanta de cel mai apropiat Peco.\n");
					                strcat(msg,"Doriti sa schimbati traseul?\n");
					                write(client[clientIndex],&msg,100);
					            }
					        }
					        else
					        {
					                int drumScurt1[31]={0};
					                int action=2;
					                calculDrumMinim();
					                if(destPeco==1)shortestPath(drumScurt[contor+1],idPeco,drumScurt1);
					                    else shortestPath(drumScurt[contor+1],nodFinal,drumScurt1);
					                recommendPathToClient(drumScurt1,client,clientIndex);
					                if(intrebare1==1){char msg[100]={0};strcpy(msg,"Ati ajuns la Peco.\nDoriti sa reluati drumul?\n"); write(client[clientIndex],&msg,100);}
				                    if(intrebare2==1)
				                {
				                    char msg[100]={0};
					                strcpy(msg,"Sunteti la ");
					                char dplp[10]={0};
					                char rasp[100]={0};
					                sprintf(dplp,"%d",drumMinim[drumScurt[contor+1]][idPeco]);
					                strcat(msg,dplp);
					                strcat(msg," km distanta de cel mai apropiat Peco.\n");
					                strcat(msg,"Doriti sa schimbati traseul?\n");
					                write(client[clientIndex],&msg,100);
					            }
					                    drumScurt[0]=drumScurt[contor];
					                    for(int o=0;o<=29;o++)
					                    {
					                    drumScurt[o+1]=drumScurt1[o];
					                    //i++;
					                    }
					                    contor=0;
					                    //dplus=lungimeDrum[drumScurt[0]][drumScurt[1]];
					        }
					        read(client[clientIndex],&viteza,100);
					        strcpy(mesaj1,"Circulati cu viteza ");
					        strcat(mesaj1,viteza);
					        strcat(mesaj1,".\n");
					        if(atoi(viteza)>vitezaMaxima[drumScurt[contor]][drumScurt[contor+1]])
					        {
					            strcat(mesaj1,"Viteza maxima pe aceasta portiune de drum este de ");
                                char vit[10]={0};
                                sprintf(vit,"%d",vitezaMaxima[drumScurt[contor]][drumScurt[contor+1]]);
                                strcat(mesaj1,vit);
                                strcat(mesaj1,".\n");
					        }
					        if(vitezaCurenta!=-1 && pso==0 ){
					            write(client[clientIndex],&mesaj1,100);
					            if(intrebare1==1){char msg[100]={0};strcpy(msg,"Ati ajuns la Peco.\nDoriti sa reluati drumul?\n"); write(client[clientIndex],&msg,100);}
				                if(intrebare2==1)
				                {
				                    char msg[100]={0};
					                strcpy(msg,"Sunteti la ");
					                char dplp[10]={0};
					                char rasp[100]={0};
					                sprintf(dplp,"%d",drumMinim[drumScurt[contor+1]][idPeco]);
					                strcat(msg,dplp);
					                strcat(msg," km distanta de cel mai apropiat Peco.\n");
					                strcat(msg,"Doriti sa schimbati traseul?\n");
					                write(client[clientIndex],&msg,100);
					            }
					            }
					     }
					     else if(strstr(comanda,"Statie peco")!=NULL)
					     {
					        if(info==0)
					        {
					            char opt[100]={0}; 
					            strcpy(opt,"Nu ati selectat aceasta optiune."); 
					            write(client[clientIndex],&opt,100);
					        }
					        else
					        {
					            int drumScurt2[31]={0};
					            for(int o=1;o<=10;o++)
					            {
					                char loc[100]={0}; 
					                idToLocatie(o,loc);
					                if(strstr(loc,"Peco")!=NULL)
					                    {
					                        if(idPeco==0)idPeco=o;
					                        else if(drumMinim[drumScurt[contor+1]][o]<drumMinim[drumScurt[contor+1]][idPeco])
					                                idPeco=o;
					                    }
					            }				            
					            char msg[100]={0};
					            strcpy(msg,"Sunteti la ");
					            char dplp[10]={0};
					            char rasp[100]={0};
					            sprintf(dplp,"%d",drumMinim[drumScurt[contor+1]][idPeco]);
					            strcat(msg,dplp);
					            strcat(msg," km distanta de cel mai apropiat Peco.\n");
					            strcat(msg,"Doriti sa schimbati traseul?\n");
					            write(client[clientIndex],&msg,100);
					            intrebare2=1;
					        }
					     }
					     else if(strstr(comanda,"Raporteaza politie")!=NULL)
					     {
					        if(politie_raportata[drumScurt[contor]][drumScurt[contor+1]]==0)
					        {
					            politie_raportata[drumScurt[contor]][drumScurt[contor+1]]=1;
					            kill(getppid(),SIGUSR2);
					            write(w[1],&comanda,100);
					            write(w[1],&pipeIndex,4);
					            write(q[clientIndex][1],&idDrum[drumScurt[contor]][drumScurt[contor+1]],4);
					        }
					        else
					        {
					            char avr[100]={0};
					            strcpy(avr,"\nAti raportat deja politie pe aceasta strada.\n");
					            write(client[clientIndex],&avr,100);
					        }
					     }
					     else if(strstr(comanda,"Politie raportata")!=NULL)
					     {
					        kill(getppid(),SIGUSR2);
					        write(w[1],&comanda,100);
					        write(w[1],&pipeIndex,4);
					        write(q[clientIndex][1],&idDrum[drumScurt[contor]][drumScurt[contor+1]],4);
					     }
					     else if(strstr(comanda,"Da")!=NULL || strstr(comanda,"Nu")!=NULL)
					     {
					        if(intrebare2==1)
					        {
					            if(strstr(comanda,"Da")!=NULL)
					                {
					                    destPeco=1;
					                    char traseu[100]={0};
					                    int drumScurt2[31]={0};
					                    strcpy(traseu,"Traseu nou");
					                    write(client[clientIndex],&traseu,100);
					                    shortestPath(drumScurt[contor+1],idPeco,drumScurt2);
					                    recommendPathToClient(drumScurt2,client,clientIndex);
					                    //i=0;
					                    drumScurt[0]=drumScurt[contor];
					                    for(int o=0;o<=29;o++)
					                    {
					                    drumScurt[o+1]=drumScurt2[o];
					                    //i++;
					                    }
					                    contor=0;
					                    //dplus=lungimeDrum[drumScurt[0]][drumScurt[1]];
					                }
					           intrebare2=0;
					        }
					        if(intrebare1==1)
					        {
					            if(strstr(comanda,"Da")!=NULL)
					                {
					                    destPeco=0;
					                    char traseu[100]={0};
					                    int drumScurt2[31]={0};
					                    strcpy(traseu,"Traseu nou");
					                    write(client[clientIndex],&traseu,100);
					                    shortestPath(idPeco,nodFinal,drumScurt2);
					                    recommendPathToClient(drumScurt2,client,clientIndex);
					                    for(int o=0;o<=30;o++)
					                    {
					                    drumScurt[o]=drumScurt2[o];
					                    }
					                    contor=0;
					                    dplus=lungimeDrum[drumScurt[0]][drumScurt[1]];
					                    vitezaCurenta=0;
					                }
					                else
					                {
					                    char dest[100]={0};
					                    strcpy(dest,"Ati ajuns la destinatie.\n");
					                    write(client[clientIndex],&dest,100);
					                }
					                intrebare1=0;
					        }
					     }
					     else if(strstr(comanda,"Pret combustibil")!=NULL)
					        {
					        if(info==1)
					        {
					        int fd=open("Pret_combustibil.txt",O_RDONLY);
					        char text[100]={0};
					        char text1[256]={0};
					        strcpy(text,"Info");
					        write(client[clientIndex],&text,100);
					        read(fd,&text1,256);
					        write(client[clientIndex],&text1,256);
					        close(fd);
					        }
					        else
					        {
					            char opt[100]={0}; 
					            strcpy(opt,"Nu ati selectat aceasta optiune."); 
					            write(client[clientIndex],&opt,100);
					        } 
					        }
					       	else if(strstr(comanda,"Sport")!=NULL)
					        {
					        if(info==1)
					        {
					        int fd=open("Sport.txt",O_RDONLY);
					        char text[100]={0};
					        char text1[256]={0};
					        strcpy(text,"Info");
					        write(client[clientIndex],&text,100);
					        read(fd,&text1,256);
					        write(client[clientIndex],&text1,256);
					        close(fd);
					        }
					        else
					        {
					            char opt[100]={0}; 
					            strcpy(opt,"Nu ati selectat aceasta optiune."); 
					            write(client[clientIndex],&opt,100);
					        } 
					        }
					     else if(strstr(comanda,"Meteo")!=NULL)
					     {
					        if(info==1)
					        {
					        int fd=open("Meteo.txt",O_RDONLY);
					        char text[100]={0};
					        char text1[256]={0};
					        strcpy(text,"Info");
					        write(client[clientIndex],&text,100);
					        read(fd,&text1,256);
					        write(client[clientIndex],&text1,256);
					        close(fd);
					        }
					        else
					        {
					        	char opt[100]={0}; 
					            strcpy(opt,"Nu ati selectat aceasta optiune."); 
					            write(client[clientIndex],&opt,100);
					        }
					      }
                          else if(strstr(comanda,"Oprire")!=NULL)
                          {
                            vitezaCurenta=-1;
                            char opr[100]={0};
                            if(pso==0)
                            {
                            pso=1;
                            strcpy(opr,"\nTraseul va fi oprit momentan...\n");
                            write(client[clientIndex],&opr,100);
                            }
                            else
                            {
                            strcpy(opr,"\nTraseul este deja oprit.\n");
                            write(client[clientIndex],&opr,100);
                            }
                          }
                         else if(strstr(comanda,"Pornire")!=NULL)
                          {
                            vitezaCurenta=0;
                            char opr[100]={0};
                            if(pso==1)
                            {
                            pso=0;
                            strcpy(opr,"\nVom relua traseul.\n");
                            write(client[clientIndex],&opr,100);
                            }                          
                            else
                            {
                            strcpy(opr,"\nTraseul este deja pornit.\n");
                            write(client[clientIndex],&opr,100);
                            }
                          } 
				else if(vitezaCurenta!=-1){
                      vitezaCurenta=atoi(comanda);
                      if(vitezaCurenta>vitezaMaxima[drumScurt[contor]][drumScurt[contor+1]])
                        {
                        char mesajAvertizare[100]={0};
                        strcpy(mesajAvertizare,"Circulati cu viteza ");
                        strcat(mesajAvertizare,comanda);
                        strcat(mesajAvertizare,".\nLimita pe aceasta portiune de drum este de ");
                        char vit[10]={0};
                        sprintf(vit,"%d",vitezaMaxima[drumScurt[contor]][drumScurt[contor+1]]);
                        strcat(mesajAvertizare,vit);
                        strcat(mesajAvertizare,".\n");
                        write(client[clientIndex],mesajAvertizare,100);
                        if(intrebare1==1)
                        {
                            char msg[100]={0};
                            strcpy(msg,"Ati ajuns la Peco.\nDoriti sa reluati drumul?\n"); 
                            write(client[clientIndex],&msg,100);
                        }
				        if(intrebare2==1)
				                {
				                    char msg[100]={0};
					                strcpy(msg,"Sunteti la ");
					                char dplp[10]={0};
					                char rasp[100]={0};
					                sprintf(dplp,"%d",drumMinim[drumScurt[contor+1]][idPeco]);
					                strcat(msg,dplp);
					                strcat(msg," km distanta de cel mai apropiat Peco.\n");
					                strcat(msg,"Doriti sa schimbati traseul?\n");
					                write(client[clientIndex],&msg,100);
					            }
                        }
                      dplus=dplus-(float)vitezaCurenta/60;
                      if(dplus<=0 && drumScurt[contor+2]!=0)
                        {
                            char locatie[100]={0};
                            idToLocatie(drumScurt[contor+1],locatie);
                            strcat(locatie,"\n");
                            write(client[clientIndex],&locatie,100);
                            contor++;
                            if(intrebare1==1)
                        {
                            char msg[100]={0};
                            strcpy(msg,"Ati ajuns la Peco.\nDoriti sa reluati drumul?\n"); 
                            write(client[clientIndex],&msg,100);
                        }
				        if(intrebare2==1)
				                {
				                    char msg[100]={0};
					                strcpy(msg,"Sunteti la ");
					                char dplp[10]={0};
					                char rasp[100]={0};
					                sprintf(dplp,"%d",drumMinim[drumScurt[contor+1]][idPeco]);
					                strcat(msg,dplp);
					                strcat(msg," km distanta de cel mai apropiat Peco.\n");
					                strcat(msg,"Doriti sa schimbati traseul?\n");
					                write(client[clientIndex],&msg,100);
					            }
                            drumCurent=idDrum[drumScurt[contor]][drumScurt[contor+1]];
                            dplus=(float)lungimeDrum[drumScurt[contor]][drumScurt[contor+1]]+dplus;
                        }
                       else if(dplus<=0 && drumScurt[contor+2]==0)
                            {
                            char locatie[100]={0};
                            idToLocatie(drumScurt[contor+1],locatie);
                            write(client[clientIndex],&locatie,100);
                            if(destPeco==0)
                            {                            
                                strcat(locatie,"\nAti ajuns la destinatie");
                                write(client[clientIndex],&locatie,100);
                            }
                            else if(destPeco==1)
                            {
                                vitezaCurenta=-1;
                                char lc[100]={0};
                                strcpy(lc,"Ati ajuns la Peco.\nDoriti sa reluati traseul?\n");
                                intrebare1=1;
                                write(client[clientIndex],&lc,100);            
                            }
                        } 
			        }
			}
			exit(0);
		}// Aici se termina procesul copil

		else //Revin in procesul paritne
			{
			clientIndex++;
			pipeIndex++;
			}
    }			/* while */
}				/* main */
