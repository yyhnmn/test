/*
    Server that performs read or write operations to a string array
*/

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include <errno.h>
#include "common.h"
#include "timer.h"
#include <signal.h>
#define STRINGSIZE 100

int NUM_STR;
double elapsed_time;
pthread_mutex_t mutex;
char** theArray;
void *ServerEcho(void *args);

int main(int argc, char* argv[]){
	struct sockaddr_in sock_var;
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;
	int i;
	int j;
	pthread_t t[COM_NUM_REQUEST];
	pthread_mutex_init(&mutex, NULL);
	
	if (argc != 4){ 
		fprintf(stderr, "usage: %s <Size of theArray_ on server> <server ip> <server port>\n", argv[0]);
		exit(0);
	}

	NUM_STR = strtol(argv[1], NULL, 10);
	char *server_ip = argv[2];
	int server_port = strtol(argv[3], NULL, 10);
	
	theArray = malloc(NUM_STR * sizeof(char*));
	// initialize the string array
	for (i=0; i<NUM_STR;i++) {
		theArray[i] = malloc(STRINGSIZE * sizeof(char));
		sprintf(theArray[i], "String %d: the intial value", i);
		printf("%s\n",theArray[i]);
	}	

	/* Initialize socket address and port*/
	sock_var.sin_addr.s_addr=inet_addr(server_ip);
	sock_var.sin_port=server_port;
	sock_var.sin_family=AF_INET;

	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	    {
		printf("socket has been created\n");
		listen(serverFileDescriptor,2000); 
		while(1)        //loop infinity
		{

		    for(j=0;j<COM_NUM_REQUEST;j++)      //can support 20 clients at a time
		    {
		        clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
		        printf("Connected to client %d\n",clientFileDescriptor);
		        pthread_create(&t[j],NULL,ServerEcho,(void *)(long)clientFileDescriptor);
			pthread_detach(t[j]);			 
		    }

		    FILE* op;		
    		    if ((op = fopen("server_output_time_aggregated","a+")) == NULL){
     			   printf("Error opening the output file: server_output_time_aggregated.\n");
        		   exit(1);
    			}
  		    fprintf(op, "%e\n", elapsed_time);
		    printf("elapsed_time = %f \n" , elapsed_time);
    		    fclose(op);
		    elapsed_time = 0;
		}
		close(serverFileDescriptor);
	    }
	    else{
		printf("socket creation failed\n");
	    }
	pthread_mutex_destroy(&mutex);
	return 0;
}

void *ServerEcho(void *args)
{
    int clientFileDescriptor=(int)args;
    char str_recv[STRINGSIZE];
    char str_sent[STRINGSIZE];
    double start, end;
    ClientRequest rqst;

    read(clientFileDescriptor,str_recv,STRINGSIZE);
    GET_TIME(start);
    
    ParseMsg(str_recv,&rqst);
    printf("reading from client:%s\n",rqst.msg);
    
    
    // set content for a write request
    if(!rqst.is_read){	
	pthread_mutex_lock(&mutex);
    	setContent(rqst.msg,rqst.pos,theArray);
	pthread_mutex_unlock(&mutex);
    }
    // get content for a read request
    else{   
	   pthread_mutex_lock(&mutex);
 	   getContent(str_sent,rqst.pos,theArray);
	   pthread_mutex_unlock(&mutex);    
	}
    GET_TIME(end);

    pthread_mutex_lock(&mutex);
    elapsed_time = elapsed_time + end - start;
    pthread_mutex_unlock(&mutex);    
    
    write(clientFileDescriptor,str_sent,STRINGSIZE);
   
    close(clientFileDescriptor);
    return NULL;
}
