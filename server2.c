#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <stdint.h>

#include "dns1.c"



#define MAX 3 // number of client to connected
#define TRUE   1
#define FALSE  0
#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authoority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server


//global var
char dns_servers[10][100];
unsigned char host_cl_1[2000];
char dns_servers1[10][100];
int dns_server_count = 0;

//methods
//void getcon(char *argv[]);
void get_ip(char *host_cl);
void get_from_external(unsigned char* host_cl);
//void ChangetoDnsNameFormat(unsigned char*,unsigned char*);
void gettime();
void get_dns_servers();
void my_get_host_by_name(unsigned char *host , int query_type);
u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count);
void writetofile(unsigned char* host_cl,char *buffer2);

void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host_cl);

// connect to socket, Thread
//send recv ans
//get servers
//change to dot format
//change to dns format
//caching

// DNS server -> dns db
//dns ipv4 query
// must frw query which can not be ans no failure given
//various domain still can do it
//backend server should interact with DNS serve
//2 DNS server use same backend datbase
//manually edit backend data



int main(int argc , char *argv[])
{
    int opt = TRUE;
    int master_sock , new_sock , client_sock[MAX] , max_client = MAX , act, cl ,addrlen , valread , sd;
    int max_sd;
    fd_set sock_desc;
    struct sockaddr_in servaddr;
    
    char buffer1[2000];  //data buffer1
    
    //now set the socket descriptors
    
    
    //here initialise all client_sock[] to 0
    for (cl = 0; cl < max_client; cl++)
    {
        client_sock[cl] = 0;
    }
    
    //create a master socket
    if( (master_sock = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    //set master socket to allow multiple connections
    if( setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    //type of socket created
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(atoi("130.65.120.1"));
    servaddr.sin_port = htons(atoi(argv[1]) );
    
    //bind the socket to localhost port specified
    
    if (bind(master_sock, (struct sockaddr *)&servaddr, sizeof(servaddr))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    printf("here Listener on port %s \n", argv[1]);
    
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_sock, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    //accept the incoming connection
    addrlen = sizeof(servaddr);
    puts("Now Waiting for connections ...");
    
    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&sock_desc);
        
        //add master socket to set
        FD_SET(master_sock, &sock_desc);
        max_sd = master_sock;
        
        //add child sockets to set
        for ( cl = 0 ; cl < max_client ; cl++)
        {
            //socket descriptor
            sd = client_sock[cl];
            
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &sock_desc);
            
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }
        
        //wait for an act on one of the sockets , timeout is NULL , so wait indefinitely
        act = select( max_sd + 1 , &sock_desc , NULL , NULL , NULL);
        
        if ((act < 0) && (errno!=EINTR))
        {
            printf("select error here \n");
        }
        
        // an incoming connection
        if (FD_ISSET(master_sock, &sock_desc))
        {
            if ((new_sock = accept(master_sock, (struct sockaddr *)&servaddr, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            
            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is : %d , ip is : %s , Port : %d \n" , new_sock , inet_ntoa(servaddr.sin_addr) , ntohs(servaddr.sin_port));
            
            
            
            //add new socket to array of sockets
            for (cl = 0; cl < max_client; cl++)
            {
                //if position is empty
                if( client_sock[cl] == 0 )
                {
                    client_sock[cl] = new_sock;
                    printf("Adding the list of sockets here: %d\n" ,cl);
                    
                    break;
                }
            }
        }
        
        //else its some IO operation on some other socket
        for (cl = 0; cl < max_client; cl++)
        {
            sd = client_sock[cl];
            
            if (FD_ISSET( sd , &sock_desc))
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read(sd,buffer1, 2024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&servaddr , (socklen_t*)&addrlen);
                    printf("Host disconnected here sorry ip %s , port %d \n" , inet_ntoa(servaddr.sin_addr) , ntohs(servaddr.sin_port));
                    
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_sock[cl] = 0;
                }
                
                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end of the data read
                    buffer1[valread] = '\0';
                    //puts(buffer1); host name recived checked.
                    //printf("%s", );
                    
                    get_ip(buffer1);
                     send(sd ,dns_servers[0] , strlen(dns_servers[0]) , 0 );
                    
                    //send(sd , buffer1 , strlen(buffer1) , 0 );
                //   strcpy(dns_servers[0],(char *)'\0');
                    
                    int i, j;
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 100; j++) {
                            dns_servers[i][j] = '\0';
                        }
                    }// SETTING DNS server field to null
                    
                    memset(dns_servers[0], 0, sizeof(dns_servers[0][100]));
                   // void gettime();
                }
            }
        }
    }
    
    return 0;
}








void get_ip(char* host_cl)
{
    FILE *fp;
    char line[200] , *p;
    
    
       if((fp = fopen("server_db.txt" , "r")) == NULL)
    {
        printf("Failed opening server_db.txt file \n");
    }
    
    FILE *fp1=fopen("server_db.txt" , "r");
    int ch, number_of_lines = 0;
    
    do
    {
        ch = fgetc(fp1);
        if(ch == '\n')
            number_of_lines++;
    } while (ch != EOF);
    
  //  printf(" Number of lines here in this file %d",number_of_lines);
    int cnt_loop=0;
    while(fgets(line , 200 , fp))
    {
        //  printf("%s",line);
        
        cnt_loop++;
        
        if(line[0] == '#')
        {
            
            continue;
        }
        if (strncmp(host_cl,"",strlen(host_cl))==0) {
          //  printf("sorry no input recived");
            strcpy(dns_servers[0],"sorry no input recived");
          //  strcpy(,);
            strncpy((char*)host_cl_1, host_cl, 30);
           
            get_from_external(host_cl_1);
            //return;
            continue;
            
        }
        
        if(strncmp(line,host_cl,strlen(host_cl)) < 0 )
        {
            if (number_of_lines<cnt_loop)
            {
                //strcpy(dns_servers[0], "too small");
                strncpy((char*)host_cl_1, host_cl, 30);
                
                get_from_external(host_cl_1);
                return;

                
            }
            else
                continue;
        }
        if(strncmp(line,host_cl,strlen(host_cl)) > 0 )
        {
            if (number_of_lines<cnt_loop)
            {
                //strcpy(dns_servers[0], "too small");
              //  strcpy(host_cl_1,(unsigned char*)host_cl);
                strncpy((char*)host_cl_1, host_cl, 30);
                get_from_external(host_cl_1);
                return;
                
                
            }
            else
                continue;
        }
        
        if(strncmp(line,host_cl,strlen(host_cl)) == 0 )
        {
          //  printf("equal");
            
            strcpy(dns_servers[0], strtok(line, " "));
            strcpy(dns_servers[0], strtok(NULL, "\n"));
            //printf("%s \n",dns_servers[0]);
            return;
           // continue;
        }
    }

    return;
}
void gettime()
{
    time_t current_time;
    char* c_time_string;
    
    /* Obtain current time as seconds elapsed since the Epoch. */
    current_time = time(NULL);
    c_time_string = ctime(&current_time);

    printf("Current time is %s", c_time_string);
    return;
}


void get_from_external(unsigned char* host_cl)
{
    // strcpy(dns_servers[0], "Sorry not found");
    //now get DNS server
    get_dns_servers();
    printf("%s", "I am here ");
   // now get the ip of this hostname
    //my_get_host_by_name(host_cl,T_A);
    char *buffer2=ngethostbyname(host_cl,T_A);
    printf("%s", buffer2);
   // strcpy(dns_servers[0],buffer2);
    //strcat(dns_servers[0],"The IP Address");
    //ngethost_clbyname(host_cl , T_A);
   
    return;
}















