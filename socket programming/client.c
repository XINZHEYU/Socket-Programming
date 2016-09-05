//
//  main.c
//  programming 1 client
//
//  Created by 俞歆哲 on 9/27/15.
//  Copyright (c) 2015 XINZHE YU. All rights reserved.
//
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <fcntl.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>


#define bufsize 300


int receive();
void logout(int signal);
int inactivetime(long a);

int sock_client;


char recvbuf[bufsize];

int main() {
    
    struct sockaddr_in ser_addr;
    char sendbuf[bufsize];
    
    char server_ip[20];
    int PORT;
    
    while (1) {
        printf("please imput the server's IP and PORT in this format: IP PORT\n");
        scanf("%s %d" , server_ip , &PORT);
        
        //create the socket
        
        sock_client = socket(PF_INET, SOCK_STREAM, 0);
        if (sock_client < 0) {
            perror("socket error");
            return 1;
        }
        
        //create the connect address
        
        memset(&ser_addr , 0 , sizeof(ser_addr));
        ser_addr.sin_family = AF_INET;
        ser_addr.sin_port = htons(PORT);
        ser_addr.sin_addr.s_addr = inet_addr(server_ip);
        
        int connreq = connect(sock_client , (struct sockaddr*)&ser_addr , sizeof(struct sockaddr));
        if(connreq < 0){
            printf("connect fail\n");
        }
        if (connreq == 0) {
            printf("connected\n");
            break;
        }
    }
    
    
    pthread_t re;
    pthread_create(&re, NULL, (void*)receive, NULL);
    //send the connect request
    
    
    
    while(1){
        signal(SIGINT, logout);
        gets(sendbuf);
        long int s = send(sock_client, sendbuf, strlen(sendbuf), 0);
        if (s == -1) {
            printf("send fail\n");
        }
        
    }
    
    return 0;
    
}





int receive(){
    while (1) {
        recv(sock_client, recvbuf, bufsize, 0);
        if (recvbuf[0] != '\0') {
            printf("%s\n" , recvbuf);
            if (!strcmp(recvbuf, log)) {
                return 0;
            }
            memset(recvbuf, 0, sizeof(recvbuf));
        }
        
    }
    
}


void logout(int signal){
    printf("request for logout\n");
    send(sock_client, "logout", 6, 0);
    exit(0);
}


