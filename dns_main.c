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




#define MAX 3 // number of client to connected
#define TRUE   1
#define FALSE  0


//global var
char dns_servers[10][100];
char host_cl_1[2000];

//methods
//void getcon(char *argv[]);
//void get_ip(char *host_cl);
void ChangetoDnsNameFormat(unsigned char*,unsigned char*);
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


int main(int args, char *argv[])
{
	
	

	

    int opt = TRUE;
    int master_sock , new_sock , client_sock[MAX] , max_client = MAX , act, cl ,addrlen , valread , sd;
    int max_sd;
    fd_set sock_desc;
    struct sockaddr_in servaddr;
    
    char host_cl[2000];  //data host_cl
    
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
    servaddr.sin_addr.s_addr = INADDR_ANY;
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
                if ((valread = read( sd , host_cl, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print               getpeername(sd , (struct sockaddr*)&servaddr , (socklen_t*)&addrlen);
                    printf("Host disconnected here sorry ip %s , port %d \n" , inet_ntoa(servaddr.sin_addr) , ntohs(servaddr.sin_port));
                    
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_sock[cl] = 0;
                }
                
                //Echo back the message that came in
                else
                { //ready the reply
                    //set the string terminating NULL byte on the end of the data read
                    host_cl[valread] = '\0';
                    //puts(host_cl); host name recived checked.
                    //printf("%s", );
                    
                    //this is reply ip address ,
                    //send(sd , host_cl , strlen(host_cl) , 0 );
                    FILE *fp;
                    char line[200] , *p;
                    
                   
                    int i,j = 0;
                     printf("%d",j);
                    j++;
                    if((fp = fopen("server_db.txt" , "r")) == NULL)
                    {
                        printf("Failed opening server_db.txt file \n");
                    }
                    
                    while(fgets(line , 200 , fp))
                    {
                        //  printf("%s",line);
                        
                        
                        
                        if(line[0] == '#')
                        {
                            
                            continue;
                        }
                        if (strncmp("", host_cl,strlen(host_cl))==0) {
                            printf("sorry no input recived");
                        }
                        
                        if(strncmp(line , host_cl,strlen(host_cl)) == 0 )
                        {
                            printf("equal");
                            
                            strcpy(dns_servers[i], strtok(line, " "));
                            
                            strcpy(dns_servers[i], strtok(NULL, "\n"));
                            printf("%s \n",dns_servers[0]);
                            
                           
                            send(sd ,dns_servers[0] , strlen(dns_servers[0]) , 0 );
                            break;
                        }
                    }
                    
                }// end of else reply ready
            }
        }
    }

	return 0;
}








void get_ip(char* host_cl)
{
    FILE *fp;
    char line[200] , *p;
    
    
    int i,j = 0;
    printf("%d",j);
    j++;
    if((fp = fopen("server_db.txt" , "r")) == NULL)
    {
        printf("Failed opening server_db.txt file \n");
    }
    
    while(fgets(line , 200 , fp))
    {
        //  printf("%s",line);
        
        
        
        if(line[0] == '#')
        {
            
            continue;
        }
        if (strncmp("", host_cl,strlen(host_cl))==0) {
            printf("sorry no input recived");
        }
        
        if(strncmp(line , host_cl,strlen(host_cl)) == 0 )
        {
            printf("equal");
            
            strcpy(dns_servers[i], strtok(line, " "));
            
            strcpy(dns_servers[i], strtok(NULL, "\n"));
            printf("%s \n",dns_servers[0]);
            
            
            send(sd ,dns_servers[0] , strlen(dns_servers[0]) , 0 );
            break;
        }
    }

}

void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host)
{
    int lock = 0 , i;
    strcat((char*)host,".");
    
    for(i = 0 ; i < strlen((char*)host) ; i++)
    {
        if(host[i]=='.')
        {
            *dns++ = i-lock;
            for(;lock<i;lock++)
            {
                *dns++=host[lock];
            }
            lock++; //or lock=i+1;
        }
    }
    *dns++='\0';
}