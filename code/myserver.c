#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define USER 0
#define POLICE 1
#define ADMIN 2
#define UNAUTH_USER -1
#define RESPONSE_BYTES 512
#define REQUEST_BYTES 512
#define linesInMS 5
#define EXIT -1

char* RMFC(int clientFD) {
    int nptr = 0;
    int y = read(clientFD, &nptr, sizeof(int));
    if(y <= 0) {
        shutdown(clientFD, SHUT_WR);
        return NULL;
    }
    char *sr = (char*)malloc(nptr*REQUEST_BYTES);
    memset(sr, 0, nptr*REQUEST_BYTES);
    char *sr_pr = sr;
    int j;
    for(j = 0; j < nptr; ++j) {
        int y = read(clientFD, sr, REQUEST_BYTES);
        sr = sr+REQUEST_BYTES;
    }
    return sr_pr;
}

void SMTC(int clientFD, char *sr) {
    int npts = (strlen(sr)-1)/RESPONSE_BYTES + 1;
    int y = write(clientFD, &npts, sizeof(int));
    char *mts = (char*)malloc(npts*RESPONSE_BYTES);
    strcpy(mts, sr);
    int j;
    for(j = 0; j < npts; ++j) {
        int y = write(clientFD, mts, RESPONSE_BYTES);
        mts += RESPONSE_BYTES;
    }
}


char *PMS(char *usr,int clnt_fd)
{
	FILE *filep = fopen(usr,"r");
	char *ms = NULL;
    ms = (char *)malloc(10000*sizeof(char));
    ms[0] = '\0';
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int count=0;

	while(count<linesInMS && (read = getline(&line, &len, filep)) != -1)
	{
		strcat(ms,line);
		count++;
	}

	fclose(filep);

	if(strlen(ms)==0)
		strcpy(ms,"None");

	return ms;
}

void guc(char *usr,char *pss,int clnt_fd)
{
	char *rus,*rps;
	SMTC(clnt_fd,"Enter Username: ");
	rus=RMFC(clnt_fd);

	SMTC(clnt_fd,"Enter Password: ");
	rps=RMFC(clnt_fd);

	int j=0;
	while(rus[j]!='\0' && rus[j]!='\n')
	{
		usr[j]=rus[j];
		j++;
	}

	usr[j]='\0';

	j=0;
	while(rps[j]!='\0' && rps[j]!='\n')
	{
		pss[j]=rps[j];
		j++;
	}
	pss[j]='\0';

}

char *PB(char *usr)
{
	FILE *filep=fopen(usr,"r");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;

    if((read = getline(&line, &len, filep)) != -1)
    {
    	char *tkn,*ptkn;
    	ptkn=(char *)malloc(400*sizeof(char));
    	tkn=strtok(line," \n");
    	while(tkn!=NULL)
    	{
    		strcpy(ptkn,tkn);
    		tkn=strtok(NULL," \n");
    	}
    	fclose(filep);
    	return ptkn;
    }
    else
    {
    	fclose(filep);
    	char *balance=(char *)malloc(2*sizeof(char));
    	balance[0]='0';
    	balance[1]='\0';
    	return balance;
    }
}


void UR(char *usr,char *pss,int clnt_fd)
{
	int fg=1;
	SMTC(clnt_fd,"Enter your choice\n1. Available Balance\n2. Mini Statement\nWrite exit for quitting.");
	char *bf=NULL;
	while(fg)
	{
		if(bf!=NULL)
			bf=NULL;
		bf=RMFC(clnt_fd);

		int ch;

		if(strcmp(bf,"exit")==0)
			ch=3;
		else
		    ch=atoi(bf);
		char *balance,*sr;
		balance=(char *)malloc(1000*sizeof(char));
		sr=(char *)malloc(10000*sizeof(char));
		strcpy(balance,"------------------\nAvailable Balance: ");
		strcpy(sr,"------------------\nMini Statement: \n");
		switch(ch)
		{
			case 1:
				strcat(balance,PB(usr));
				SMTC(clnt_fd,strcat(balance,"\n------------------\n\nEnter your choice\n1. Available Balance\n2. Mini Statement\nWrite exit for quitting."));
				break;
			case 2:
			 	strcat(sr,PMS(usr,clnt_fd));
				SMTC(clnt_fd,strcat(sr,"\n------------------\n\nEnter your choice\n1. Available Balance\n2. Mini Statement\nWrite exit for quitting."));
				free(sr);
				break;
			case 3:
				fg=0;
				break;
			default:
				SMTC(clnt_fd, "Unknown Query");
				break;
		}
	}
}


int CU(char *user)
{
	FILE *filep=fopen("login_file","r");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;

	while((read = getline(&line, &len, filep)) != -1) 
	{
		char *tkn=strtok(line," ");
		if(strcmp(tkn,user)==0)
		{
			tkn=strtok(NULL," ");
			tkn=strtok(NULL," ");
			if(tkn[0]=='C')
			{
				fclose(filep);
				return 1;
			}
        }
    }

    fclose(filep);
    return 0;


}

void UT(char *usr,int ch,double balance)
{
	FILE *filep=fopen(usr,"r");
	char * line = NULL;
	char c=(ch==1)?'C':'D';
	char *line1=(char *)malloc(sizeof(char)*10000);
    size_t len = 0;
    ssize_t read;
	time_t ltime; 

	ltime=time(NULL);
	sprintf(line1,"%.*s %c %f\n",(int)strlen(asctime(localtime(&ltime)))-1,asctime(localtime(&ltime)),c,balance);

	while((read = getline(&line, &len, filep)) != -1)
		strcat(line1,line);

	fclose(filep);
	filep=fopen(usr,"w");
	fwrite(line1, sizeof(char), strlen(line1), filep);
	fclose(filep);
}

int qr(char *usr, int clnt_fd)
{
	int fg=1;
	SMTC(clnt_fd,"Choose an option\n1. Credit\n2. Debit\nWrite exit to terminate");
	while(fg)
	{
		char* bf=RMFC(clnt_fd);

		if(strcmp(bf,"exit")==0)
			return EXIT;
		else
		{
			int ch=atoi(bf);
			double balance=strtod(PB(usr),NULL);

			if(ch!=1 && ch!=2)
				SMTC(clnt_fd,"Unknown Query");
			else
			{
				SMTC(clnt_fd,"Enter amount");

				while(1)
				{
					char *bf=RMFC(clnt_fd);
					double amt=strtod(bf,NULL);
				
					if(amt<=0)
						SMTC(clnt_fd,"Enter valid amount");
					else
					{
						if(ch==2 && balance<amt)
						{
							SMTC(clnt_fd,"Insufficient Balance.\n--------------------\n\nEnter username of the account holder or 'exit' to quit.");
							fg=0;
							break;
						}
						else if(ch==2)
							balance-=amt;
						else if(ch==1)
							balance +=amt;

						UT(usr,ch,balance);
						SMTC(clnt_fd,"User updated successfully.\n--------------------\n\nEnter username of the account holder or 'exit' to quit.");
						fg=0;
						break;
					}

				}
			}

		}
	}
}
void AR(int clnt_fd)
{
	SMTC(clnt_fd,"Enter username of the account holder or 'exit' to quit");

	while(1)
	{
		char *bf=NULL;
		bf=RMFC(clnt_fd);


		if(strcmp(bf,"exit")==0)
			break;
		else if(CU(bf))
		{
			char *ureq=(char *)malloc(40*sizeof(char));
			strcpy(ureq,bf);

			if(qr(ureq,clnt_fd)==EXIT)
				break;
		}
		else
			SMTC(clnt_fd,"Wrong Username. Please enter a valid user");
	}



}


char * GBA()
{
	FILE *filep=fopen("login_file","r");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char *rstr=(char *)malloc(10000*sizeof(char));
    rstr[0]='\0';

	while((read = getline(&line, &len, filep)) != -1) 
	{
		char *tkn=strtok(line," ");
		char *tkn1=strtok(NULL," ");
		char *tkn2=strtok(NULL," ");
		if(tkn2[0]=='C')
		{
			strcat(rstr,tkn);
			strcat(rstr," ");
			strcat(rstr,PB(tkn));
			strcat(rstr,"\n");
        }
    }

    return rstr;

}	

void PR(int clnt_fd)
{
	SMTC(clnt_fd, "Enter your choice\n1. Print Balance of all users\n2. Get mini Statement\nWrite exit to terminate");
	int fg=1;

	while(fg)
	{
		char *bf=NULL;
		bf=RMFC(clnt_fd);
		char *balance,*sr;
		balance=(char *)malloc(1000*sizeof(char));
		sr=(char *)malloc(10000*sizeof(char));
		strcpy(balance,"------------------\nAvailable Balance: \n");
		strcpy(sr,"------------------\nMini Statement: \n");
		if(strcmp(bf,"exit")==0)
			break;
		else
		{
			int ch=atoi(bf);
			if(ch==1)
			{
				strcat(balance,GBA());
				SMTC(clnt_fd,strcat(balance,"\n--------------------\n\nEnter your choice\n1. Print Balance of all users\n2. Get mini Statement\nWrite exit to terminate"));
			}
			else if(ch==2)
			{
				SMTC(clnt_fd,"Enter Username or exit to terminate");

				while(1)
				{
					bf=RMFC(clnt_fd);

					if(strcmp(bf,"exit")==0)
					{
						fg=1;
						break;
					}
					else if(CU(bf))
					{
						char *usr=(char *)malloc(sizeof(char)*40);
						strcpy(usr,bf);
						strcat(sr,PMS(usr,clnt_fd));
						SMTC(clnt_fd,strcat(sr,"\n--------------------\n\nEnter your choice\n1. Print Balance of all users\n2. Get mini Statement\nWrite exit to terminate"));
						break;
					}
					else
						SMTC(clnt_fd,"Wrong Username. Please enter a valid user");
				}
			}
		}
	}
}

int Auth(char* usr,char *pss)
{
	printf("Authorizing\n");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;


	FILE *filep=fopen("login_file","r");
	while((read = getline(&line, &len, filep)) != -1) 
	{
		char *tkn=strtok(line," ");
		if(strcmp(tkn,usr)==0)
		{
			tkn=strtok(NULL," ");
			if(strcmp(tkn,pss)==0)
			{
				tkn=strtok(NULL," ");
                if(tkn[0]=='C')
                {
					fclose(filep);
                    return USER;    
                }
                else if(tkn[0]=='A')
                {
                    fclose(filep);
                    return ADMIN;
                }
                else if(tkn[0]=='P')
                {
                    fclose(filep);
                    return POLICE;
                }
            }
        }
    }
    if(line!=NULL)
        free(line);

    fclose(filep);
	return UNAUTH_USER;
}





void CC(int clnt_fd,char *sr)
{
	SMTC(clnt_fd, sr);
    shutdown(clnt_fd, SHUT_RDWR);
}



void TTC(int clnt_fd)
{
	char *usr,*pss;
	usr=(char *)malloc(100);
	pss=(char *)malloc(100);
	int utp;
	
	guc(usr,pss,clnt_fd);
	utp=Auth(usr,pss);

	char *sr=(char *)malloc(sizeof(char)*60);
	strcpy(sr,"Thanks ");

	switch(utp)
	{
		case USER:
			UR(usr,pss,clnt_fd);
			CC(clnt_fd,strcat(sr,usr));
			break;
		case ADMIN:
			AR(clnt_fd);
			CC(clnt_fd,strcat(sr,usr));
			break;	
		case POLICE:
			PR(clnt_fd);
			CC(clnt_fd,strcat(sr,usr));
			break;	
		case UNAUTH_USER:
			CC(clnt_fd,"unauthorised");
			break;
		default:
			CC(clnt_fd,"unauthorised");
			break;
	}
}



int main(int argc,char **argv)
{
	int sock_fd,clnt_fd,port_no;
	struct sockaddr_in serv_addr, cli_addr;

	memset((void*)&serv_addr, 0, sizeof(serv_addr));
	port_no=atoi(argv[1]);

	sock_fd=socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_port = htons(port_no);       
	serv_addr.sin_family = AF_INET;             
	serv_addr.sin_addr.s_addr = INADDR_ANY;    

	if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	    printf("Error on binding.\n");
	    exit(EXIT_FAILURE);
	}
	int reuse=1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	listen(sock_fd, 5); 
	int clisize=sizeof(cli_addr);

	while(1) {
	   
	    memset(&cli_addr, 0, sizeof(cli_addr));
	    if((clnt_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &clisize)) < 0) {
	        printf("Error on accept.\n");
	        exit(EXIT_FAILURE);
	    }

	    switch(fork()) {
	        case -1:
	            printf("Error in fork.\n");
	            break;
	        case 0: {
	            close(sock_fd);
	            TTC(clnt_fd);
	            exit(EXIT_SUCCESS);
	            break;
	        }
	        default:
	            close(clnt_fd);
	            break;
	    }
	}

}
