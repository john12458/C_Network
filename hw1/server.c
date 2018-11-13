#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXSIZE 8096

int main(int argc,char **argv){
	int i,pid,listenfd, socketfd;
	size_t length;
	static struct sockaddr_in client;
	static struct sockaddr_in server;
	
	char buf[80];
	getcwd(buf,sizeof(buf));
	if(chdir(buf)==-1){
		printf("ERR: %s\n",argv[2]);
		exit(4);
	}
	
	// AF_INET -> IPv4 protocols
	// SOCK_STREAM -> stream socket
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
		exit(3);
	
	server.sin_family = AF_INET;  
	server.sin_addr.s_addr = htonl(INADDR_ANY);	// htonl : host to network long
	server.sin_port = htons(80);	// htons host to network short
	
	// assign ip to server ip_addr referred to by the file descriptor sockfd
	if (bind(listenfd, (struct sockaddr *)&server,sizeof(server))<0) 
        	exit(3);
	
	if(listen(listenfd,32)<0)
		exit(3);

	while(1){
		printf("reading...\n");
		if((socketfd = accept(listenfd,(struct sockaddr *)&client, &length))<0)
		exit(3);

		if((pid = fork())<0){
			exit(3);		
		}else {
			if(pid == 0){ // child proccess
				int file;
				long relen;
				char readbuffer[MAXSIZE+1];
				char writebuffer[MAXSIZE+1];
				
				close(listenfd);
    				relen = read(socketfd,readbuffer,MAXSIZE); //return length 
				printf("%s\n",readbuffer);
				if (relen==0||relen==-1) 
					exit(3);
				if((file=open("index.html",O_RDONLY))==-1)
					write(socketfd, "Failed!", 7);
				while ((relen=read(file, writebuffer, MAXSIZE))>0) {
					write(socketfd,writebuffer,relen);
				}

				exit(1);
			}else{ 
				close(socketfd);
			    }
		}
		
	}
	
	return 0;
}
