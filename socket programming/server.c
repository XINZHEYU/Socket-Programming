//
//  main.c
//  programming 1
//
//  Created by 俞歆哲 on 9/21/15.
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


#define maxusernumber 10
#define maxuserlen 20
#define maxpasslen 20
#define bufsize 200
#define TIME_OUT 200
#define BLOCK_TIME 60
#define LAST_HOUR 600

int user_pass_check(char s1[20] , char s2[20]);
int userpass(int sock);
void whoelse(int a);
void wholast(int a , char *b);
void logout(int a );
void message(int a , char *b);
void broadcast(int a , char *b);
void broaduser(int a , char *b);
char *delstr(char p1[] , char p2[]);
char *dellast(char p1[]);
void serverlogout(int a);


struct userinfo{
    char username[maxuserlen];
    char password[maxpasslen];
    int sock_client;
    int failtimes;
    time_t lstsd;
    int state;
    time_t btime;
    time_t lout;
}user_pass[10];

struct command{
    char name[bufsize];
}cmd[6];

char elsebuf[bufsize] = {"other connected users are: "};


int alloguser[maxusernumber] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

int l = 0;

unsigned int sock_server;


fd_set fdsr;
struct sockaddr_in s_addr , c_addr;
int c = sizeof(c_addr);
int cliqueue[maxusernumber] = {0};
int connnum = 0;

unsigned int newclient = 0;
char msg[bufsize] = {"message send success!"};


int main()

{
    int myport;
    
    printf("please imput the port\n");
    scanf("%d" , &myport);
    
    sock_server = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_server == -1) {
        perror("socket");
        exit(errno);
    }
    else
        printf("socket create success\n");
    
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(myport);
    s_addr.sin_addr.s_addr = INADDR_ANY;
    
    int on=1;
    if((setsockopt(sock_server,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    unsigned int serverbind = bind(sock_server, (struct sockaddr *)&s_addr, sizeof(s_addr));
    if (serverbind == -1) {
        perror("bind");
        exit(errno);
    }
    else{
        printf("bind success\n");
    }
    
    
    
    char recvbuf[bufsize];
    
    unsigned a = listen(sock_server , 8);
    if (a == -1) {
        printf("listening fail\n");
        exit(errno);
    }
    
    
    struct timeval tv;
    
    
    time_t currt;
    
    
    
    while(1){
        FD_ZERO(&fdsr);
        FD_SET(sock_server , &fdsr);
        
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        
        unsigned a = listen(sock_server , 8);
        if (a == -1) {
            printf("listening fail\n");
            exit(errno);
        }
        
        for (int i = 0; i < maxusernumber; i++) {
            if (cliqueue[i] != 0) {
                FD_SET(cliqueue[i], &fdsr);
            }
        }
        
        select(FD_SETSIZE, &fdsr, NULL, NULL, &tv);
        
        pthread_t login;
        
        int f =FD_ISSET(sock_server , &fdsr);
        
        if (f) {
            pthread_create(&login, NULL, (void*)userpass, (void*)(long)sock_server);
            //    pthread_join(login, NULL);
        }
        
        char cmda[] = "whoelse";
        char cmdb[] = "wholast";
        char cmdc[] = "broadcast";
        char cmdd[] = "broaduser";
        char cmde[] = "message";
        char cmdf[] = "logout";
        
        
        strcpy(cmd[0].name , cmda);
        strcpy(cmd[1].name, cmdb);
        strcpy(cmd[2].name, cmdc);
        strcpy(cmd[3].name, cmdd);
        strcpy(cmd[4].name, cmde);
        strcpy(cmd[5].name, cmdf);
        

        for (int i = 0; i < maxusernumber; i++) {
            time(&currt);
            if ((user_pass[i].sock_client != 0) && (difftime(currt, user_pass[i].lstsd) > TIME_OUT)) {
                send(user_pass[i].sock_client, "time out", 8, 0);
                logout(i);
            }
            if (user_pass[i].state == 1 && difftime(currt, user_pass[i].btime) > BLOCK_TIME) {
                user_pass[i].state = 0;
            }
        }
       
        
        for (int i = 0; i < connnum; i++) {
            if (FD_ISSET(cliqueue[i] , &fdsr)) {
                memset(recvbuf, 0, bufsize);
                long int r = recv(cliqueue[i] , recvbuf , bufsize , 0);
                int j;
                for (j = 0; j < maxusernumber; j++) {
                    if (user_pass[j].sock_client == cliqueue[i]) {
                        if (r != -1) {
                            printf("%s: %s\n" , user_pass[j].username , recvbuf);
                            time(&user_pass[j].lstsd);
                            break;
                        }
                    }
                }
                for (int k = 0; k < 6; k++) {
                    if (strstr(recvbuf, cmd[k].name) != NULL) {
                        switch (k) {
                            case 0:{
                                whoelse(j);
                                break;
                            }
                            case 1:
                                wholast(j, recvbuf);
                                break;
                            case 2:{
                                broadcast(j , recvbuf);
                                send(cliqueue[i], msg, bufsize, 0);
                                break;
                            }
                            case 3:{
                                broaduser(j , recvbuf);
                                send(cliqueue[i], msg, bufsize, 0);
                                break;
                            }
                            case 4:{
                                message(j , recvbuf);
                                send(cliqueue[i], msg, bufsize, 0);
                                break;
                            }
                            case 5:{
                                logout(j);
                                break;
                            }
                            default:
                                send(cliqueue[i], "invalid command!", 16, 0);
                                break;
                        }
                    }
                }
            }
        }
        signal(SIGINT, serverlogout);
    }
}





int userpass(int sock){                                                   //login authentication
    
    int newclient = accept(sock, (struct sockaddr *)&c_addr, &c);
    
    char user[] = {"please imput your username: "};
    char pass[] = {"please imput your password: "};
    char logged[] = {"the username you imput has already logged in."};
    char success[] = {"login success! welcome to chatting room!"};
    char invalid[] = {"the username or password is invalid."};
    char block[] = {"third failure, your username will be blocked."};
    char state[] = {"this username has been blocked."};
    

    
    FILE *fp = fopen("user_pass.txt", "r");                                //read the file
    if (!fp) {
        printf("cannot read the file!\n");
    }
    int i = 0;
    while(!feof(fp)){
        fscanf(fp, "%s %s", user_pass[i].username, user_pass[i].password);
        i++;
    }
    fclose(fp);
    
    char userimput[maxuserlen] = {0};
    char passimput[maxpasslen] = {0};
    char preuserimput[maxuserlen] = {0};
    
    while (1) {
        
        
        long s = send(newclient, user, strlen(user), 0);
        long r = recv(newclient, userimput, maxuserlen, 0);
        s = send(newclient, pass, strlen(pass), 0);
        r = recv(newclient, passimput, maxpasslen, 0);
        
        
        for (int i = 0; i < maxusernumber; i++) {                         //check the block state
            if (!strcmp(userimput, user_pass[i].username)) {
                if (user_pass[i].state == 1) {
                    send(newclient, state, sizeof(state), 0);
                    close(newclient);
                    return 0;
                }
            }
        }
        
        
        
        unsigned int checkResult = user_pass_check(userimput, passimput);           //check username and password
        
        if (checkResult == -1) {                                                    //condition 1: already login
            send(newclient, logged, strlen(logged), 0);
        }
        
        
        else if(checkResult == 0){                                                  //condition 2: correct
            send(newclient, success, strlen(success), 0);
            for (int i = 0; i < maxusernumber; i++) {
                user_pass[i].failtimes = 0;
            }
            for (int i = 0; i < maxusernumber; i++) {
                if (cliqueue[i] == 0) {
                    cliqueue[i] = newclient;
                    FD_SET(cliqueue[i] , &fdsr);
                    connnum++;
                    break;
                }
            }
            
            for (int i = 0; i < maxusernumber; i++) {
                if (!strcmp(userimput, user_pass[i].username)) {
                    user_pass[i].sock_client = newclient;
                    time(&user_pass[i].lstsd);
                    break;
                }
            }
            printf("%s has logged in.\n" , userimput);
            return 0;
        }
        
        
        
        else{                                                              //condition 3: incorrect
            if (!strcmp(preuserimput, userimput)) {
                for (int i = 0; i < maxusernumber; i++) {
                    if (!strcmp(userimput, user_pass[i].username)) {
                        user_pass[i].failtimes++;
                        if (user_pass[i].failtimes == 3) {
                            send(newclient, block, strlen(block), 0);      //block the username
                            user_pass[i].failtimes = 0;
                            user_pass[i].state = 1;
                            time(&user_pass[i].btime);
                            close(newclient);
                            return 0;
                        }
                        send(newclient, invalid, strlen(invalid), 0);
                        break;
                    }
                }
            }
            else{
                for (int i = 0; i < maxusernumber; i++) {
                    if (!strcmp(userimput, user_pass[i].username)) {
                        user_pass[i].failtimes = 1;
                    }
                    else
                        user_pass[i].failtimes = 0;
                }
                send(newclient, invalid, strlen(invalid), 0);
            }
        }
        strcpy(preuserimput, userimput);
        memset(userimput, 0, sizeof(userimput));
        memset(passimput, 0, sizeof(passimput));
        
    }
    
    
}






int user_pass_check(char s1[20] , char s2[20]){                     //check the username and password
    
    
    int j = 0 , k = 0;
    
    while(alloguser[k] >= 0) {
        int m = strcmp(s1, user_pass[alloguser[k]].username);      //check if username has already logged in
        k++;
        if (m == 0)
            return -1;
    }
    
    
    int cmpresult = 1;                                             //check the username and password
    while (cmpresult != 0) {
        cmpresult = strcmp(s1, user_pass[j].username) || strcmp(s2, user_pass[j].password);
        j++;
        if (j > maxusernumber) {
            break;
        }
    }
    if (cmpresult == 0){
        alloguser[l] = j - 1;
        l++;
        j = 0;
        
        return 0;
    }
    
    else{
        j = 0;
        return 1;
    }
    
}


void whoelse(int a){                                             //command: whoelse
    char b[] = {" "};
    char loginuser[bufsize] = {0};
    char snd[bufsize] = {0};
    char only[] = {"you are the only one in the chatting room"};
    for (int k = 0; k < maxusernumber; k++) {
        if (alloguser[k] != a && alloguser[k] != -1) {
            strcat(loginuser, user_pass[alloguser[k]].username);
            strcat(loginuser, b);
        }
    }
    if (connnum == 1) {
        send(user_pass[a].sock_client, only, sizeof(only), 0);
    }
    else{
        strcat(snd, elsebuf);
        strcat(snd, loginuser);
        send(user_pass[a].sock_client, snd, bufsize, 0);
        memset(loginuser, 0, sizeof(loginuser));
        memset(snd, 0, sizeof(snd));
    }
}


void wholast(int a , char *b){
    char *p;
    p = strstr(b, " ");
    p++;
    
    char last[] = {"user connected in last "};
    for (int i = 0; i < maxusernumber; i++) {
        time_t current;
        if (user_pass[i].sock_client != 0 || difftime(user_pass[i].lout , time(&current) < LAST_HOUR)) {
         
        }
    }
}



void logout(int a){                                                //command: logout
    for (int i = 0; i < maxusernumber; i++) {
        if (cliqueue[i] == user_pass[a].sock_client) {
            cliqueue[i] = 0;
        }
    }
    int k = 0;
    while(alloguser[k] >= 0){
        if (alloguser[k] == a) {
            alloguser[k] = -1;
        }
        k++;
    }
    char logout[] = {"you have successfully logout."};
    time(&user_pass[a].lout);
    send(user_pass[a].sock_client, logout, sizeof(logout), 0);
    close(user_pass[a].sock_client);
    user_pass[a].sock_client = 0;
    connnum--;
}


void message(int a , char *b){                                      //command: message
    char from[maxuserlen] = {0};
    char snd[bufsize] = {0};
    strcpy(from , user_pass[a].username);
    strcat(from, ": ");
    char *m = delstr(b, "message ");
    char e[] = {" "};
    char *p = strstr(m, e);
    char *c[2];
    p++;
    c[1] = p;
    c[0] = strtok(m, e);
    int i;
    for (i = 0; i < maxusernumber; i++) {
        if(!strcmp(c[0], user_pass[i].username)){
            break;
        }
    }
    strcat(snd, from);
    strcat(snd, c[1]);
    send(user_pass[i].sock_client, snd, strlen(snd), 0);
    memset(snd, 0, sizeof(snd));
    
}





void broadcast(int a , char *b){                                     //command: broadcast
    char from[maxuserlen] = {0};
    char snd[bufsize] = {0};
    strcpy(from , user_pass[a].username);
    strcat(from, ": ");
    char *m = delstr(b, "broadcast ");
    for (int i = 0; i < maxusernumber; i++) {
        if (alloguser[i] >= 0 && alloguser[i] != a) {
            strcat(snd, from);
            strcat(snd, m);
            send(user_pass[alloguser[i]].sock_client, snd, sizeof(snd), 0);
            memset(snd, 0, sizeof(snd));
        }
    }
}


void broaduser(int a , char *b){                                     //command: broadcast to some users
    char from[maxuserlen] = {0};
    char snd[bufsize] = {0};
    strcpy(from , user_pass[a].username);
    char *p;
    char *r;
    strcat(from, ": ");
    char e[] = {"<>"};
    int i = 0;
    
    char *c[15] = {0};
    p = strtok(b, e);
    while ((p = strtok(NULL, e))) {
        c[i] = p;
        i++;
    }
    i--;
    char t[maxuserlen] = {0};
    for (int j = 0; j < maxusernumber; j++) {
        r = user_pass[j].username;
        for (int k = 0; k < i; k++) {
            for (int l = 0; l < strlen(c[k]); l++) {
                t[l] = *(c[k] + l);
            }
            if (!strcmp(t, r)) {
                strcat(snd, from);
                strcat(snd, c[i]);
                send(user_pass[j].sock_client, snd, sizeof(snd), 0);
                memset(snd, 0, sizeof(snd));
                memset(t, 0, sizeof(t));
            }
            else
                memset(t, 0, sizeof(t));
        }
    }
}


char *delstr(char p1[] , char p2[]){
    for (int i = 0; i < strlen(p2); i++) {
        for (int j = 0; j < strlen(p1); j++) {
            p1[j] = p1[j+1];
        }
    }
    return p1;
}


void serverlogout(){
    char a[] = "server logout, the connection will cancel.";
    for (int i = 0; i < maxusernumber; i++) {
        if (user_pass[i].sock_client != 0) {
            send(user_pass[i].sock_client, a, sizeof(a), 0);
            logout(i);
        }
    }
    if(close(sock_server)){
        printf("server logout");
    }
}




