#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXSIZE 8096

int main(int argc,char **argv){
	int i,j,listenfd, newfd;
	size_t length;
	struct sockaddr_in client;
	struct sockaddr_in server;
	fd_set master;
	fd_set read_fds;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	int fdmax;	
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
	
	if(listen(listenfd,10)<0)
		exit(3);
	
	FD_SET(listenfd,&master);
	fdmax = listenfd;
	while(1){
		printf("reading...\n");	
		read_fds = master;
		select(fdmax+1,&read_fds,NULL,NULL,NULL);
		for(i=0;i<FD_SETSIZE;i++){
			if(FD_ISSET(i,&read_fds)){
				if(i==listenfd){
					if((newfd = accept(listenfd,(struct sockaddr *)&client, &length))<0){
						printf("err\n");
					}else {
						FD_SET(newfd,&master);
						if(newfd > fdmax)
							fdmax = newfd;
					}
				}else{
								int file;
								long relen;
								char readbuffer[MAXSIZE+1];
								char writebuffer[MAXSIZE+1];
				
    								relen = read(newfd,readbuffer,MAXSIZE); //return length 
								printf("%s\n",readbuffer);
								if (relen==0||relen==-1) 
									printf("error\n");	
								if((file=open("index.html",O_RDONLY))==-1)
									write(newfd, "Failed!", 7);
								while ((relen=read(file, writebuffer, MAXSIZE))>0) {
									write(newfd,writebuffer,relen);
								}
								close(i);
								FD_CLR(i,&read_fds);	
				}
			}
		}
	}
	
	return 0;
}
