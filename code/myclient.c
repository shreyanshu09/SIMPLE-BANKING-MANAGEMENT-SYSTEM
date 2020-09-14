#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


#define RESPONSE_BYTES 512
#define REQUEST_BYTES 512

void SMTS(int s_f, char *str) {
    int npts = (strlen(str)-1)/REQUEST_BYTES + 1;
    int y = write(s_f, &npts, sizeof(int));
    char *mts = (char*)malloc(npts*REQUEST_BYTES);
    strcpy(mts, str);
    int j;
    for(j = 0; j < npts; ++j) {
        int y = write(s_f, mts, REQUEST_BYTES);
        mts += REQUEST_BYTES;
    }
}

char* RMFS(int s_f) {
    int nptr = 0;
    int y = read(s_f, &nptr, sizeof(int));
    if(y <= 0) {
        shutdown(s_f, SHUT_WR);
        return NULL;
    }
    char *str = (char*)malloc(nptr*RESPONSE_BYTES);
    memset(str, 0, nptr*RESPONSE_BYTES);
    char *str_p = str;
    int j;
    for(j = 0; j < nptr; ++j) {
        int y = read(s_f, str, RESPONSE_BYTES);
        str = str+RESPONSE_BYTES;
    }
    return str_p;
}

int main(int argc,char **argv)
{
	int s_f,p_n;
	struct sockaddr_in serv_addr;
	char *mfs;
    char mts[256];

	s_f=socket(AF_INET, SOCK_STREAM, 0);
	p_n = atoi(argv[2]);

	memset(&serv_addr, 0, sizeof(serv_addr));  
	serv_addr.sin_family = AF_INET;        
    serv_addr.sin_port = htons(p_n);     
    inet_aton(argv[1], &serv_addr.sin_addr);

    connect(s_f, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    printf("Connection Established Successfully.\n");
    

    while(1) {
        mfs = RMFS(s_f);
        if(mfs == NULL)
            break;
        if(strncmp(mfs, "unauth", 6) == 0) {
            printf("Unautherized User.\n");
            shutdown(s_f, SHUT_WR);
            break;
        }
        printf("%s\n",mfs);
        free(mfs);
        
        memset(mts, 0, sizeof(mts));
        scanf("%s", mts);
        SMTS(s_f, mts);
        if(strncmp(mts, "exit", 4) == 0) {
            shutdown(s_f, SHUT_WR);
            break;
        }
    }

    while(1) {
        mfs = RMFS(s_f);
        if(mfs == NULL)
            break;
        printf("%s\n",mfs);
        free(mfs);
    }
    printf("Write end closed by the server.\n");
    shutdown(s_f, SHUT_RD);
    printf("Connection closed Successfully.\n");
    return 0;


}
