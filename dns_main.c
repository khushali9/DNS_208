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
#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authoority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server


//global var
char dns_servers[10][100];
char host_cl_1[2000];
char dns_servers1[10][100];
int dns_server_count = 0;

//methods
//void getcon(char *argv[]);
void get_ip(char *host_cl);
void get_from_external(char* host_cl);
//void ChangetoDnsNameFormat(unsigned char*,unsigned char*);
void gettime();
void get_dns_servers();
void my_get_host_by_name(char *host , int query_type);
u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count);

void ChangetoDnsNameFormat(unsigned char* dns,char* host_cl);

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

typedef struct header_dns {
    unsigned short id; //16 bit ID
    unsigned char qr : 1; //1 bit Query/ Response
    unsigned char opcode : 4; //4 bit kind of Query
    unsigned char aa : 1; //
    unsigned char tc : 1;
    unsigned char rd : 1;
    unsigned char ra : 1;
    unsigned char z : 3;
    unsigned char rcode : 4;
    unsigned short qdcount;
    unsigned short ancount; //ans count
    unsigned short nscount; //auth count
    unsigned short arcount; //additional resource count
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authoenticated data
}dns1;

typedef struct que_dns {
    unsigned short qtype;
    unsigned short qclass;
}dns2;

typedef struct ans_dns {
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short rdlength;
}dns3;

//Pointers to resource record contents
struct RECORD_RES
{
    unsigned char *name;
    struct ans_dns *resource;
    unsigned char *rdata;
}dns4;

//Structure of a Query
typedef struct
{
    unsigned char *name;
    struct que_dns *ques;
} QUERY;

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
                 //   strcpy(dns_servers[0], "");
                    
                    int i, j;
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 100; j++) {
                            dns_servers[i][j] = '\0';
                        }
                    }// SETTING DNS server field to null
                    
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
            get_from_external(host_cl);
            //return;
            continue;
            
        }
        
        if(strncmp(line,host_cl,strlen(host_cl)) < 0 )
        {
            if (number_of_lines<cnt_loop)
            {
                //strcpy(dns_servers[0], "too small");
                 get_from_external(host_cl);
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
                get_from_external(host_cl);
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


void get_from_external(char* host_cl)
{
    // strcpy(dns_servers[0], "Sorry not found");
    //now get DNS server
    get_dns_servers();
    
   // now get the ip of this hostname
    my_get_host_by_name(host_cl,T_A);
    return;
}

void get_dns_servers()
{
    FILE *fp1;
    char line1[200] , *p1;
    if((fp1 = fopen("/etc/resolv.conf" , "r")) == NULL)
    {
        printf("Failed opening /etc/resolv.conf file \n");
    }
    
    while(fgets(line1 , 200 , fp1))
    {
        if(line1[0] == '#')
        {
            continue;
        }
        if(strncmp(line1 , "nameserver" , 10) == 0)
        {
            strcpy(dns_servers1[0], strtok(line1, " "));
            strcpy(dns_servers1[0], strtok(NULL, "\n"));
            //p = strtok(line , " ");
            //p = strtok(NULL , " ");
            
        }
    }
    
    strcpy(dns_servers1[0] , "75.75.75.75");
    strcpy(dns_servers1[1] , "208.67.220.220");
}


void my_get_host_by_name(char *host_cl , int query_type)
{
    
    unsigned char buf[65536],*q_name,*reader;
    int i , j , stop , s;
    
    struct sockaddr_in a;
    
    struct RECORD_RES answer1[20],autho[20],addit[20]; //the replies from the DNS server
    struct sockaddr_in dest;
    
    struct header_dns *dns = NULL;
    struct que_dns *qinfo = NULL;
    
    printf("Resolving %s" , host_cl);
    
    s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP packet for DNS queries
    
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr(dns_servers1[0]); //dns servers
    
    //Set the DNS structure to standard queries
    dns = (struct header_dns *)&buf;
    
    dns->id = (unsigned short) htons(getpid());
    dns->qr = 0; //This is a query
    dns->opcode = 0; //This is a standard query
    dns->aa = 0; //Not authooritative
    dns->tc = 0; //This message is not truncated
    dns->rd = 1; //Recursion Desired
    dns->ra = 0; //Recursion not available! hey we dont have it (lol)
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->qdcount = htons(1); //we have only 1 que_dns
    dns->ancount = 0;
    dns->nscount = 0;
    dns->arcount = 0;
    
    //point to the query portion
    q_name =(unsigned char*)&buf[sizeof(struct header_dns)];
    
    ChangetoDnsNameFormat(q_name , host_cl);
    qinfo =(struct que_dns*)&buf[sizeof(struct header_dns) + (strlen((const char*)q_name) + 1)]; //fill it
    
    qinfo->qtype = htons( query_type ); //type of the query , A , MX , CNAME , NS etc
    qinfo->qclass = htons(1); //its internet
    
    printf("\nSending Packet...");
    if( sendto(s,(char*)buf,sizeof(struct header_dns) + (strlen((const char*)q_name)+1) + sizeof(struct que_dns),0,(struct sockaddr*)&dest,sizeof(dest)) < 0)
    {
        perror("sendto failed");
    }
    printf("Done");
    
    //Receive the answer
    i = sizeof(dest);
    //puts("\n Receiving answer...");
    if(recvfrom (s,(char*)buf , 65536 , 0 , (struct sockaddr*)&dest , (socklen_t*)&i ) < 0)
    {
        perror("recvfrom failed");
    }
   // puts("Done");
    
    dns = (struct header_dns*) buf;
    
    //move ahead of the dns header and the query field
    reader = &buf[sizeof(struct header_dns) + (strlen((const char*)q_name)+1) + sizeof(struct que_dns)];
    
    //printf("\nThe response contains : ");
    //printf("\n %d que_dnss.",ntohs(dns->qdcount));
    //printf("\n %d answer1.",ntohs(dns->ancount));
   // printf("\n %d authooritative Servers.",ntohs(dns->nscount));
    //printf("\n %d Additional records.\n\n",ntohs(dns->arcount));
    
    //Start reading answer1
    stop=0;
    
    for(i=0;i<ntohs(dns->ancount);i++)
    {
        answer1[i].name=ReadName(reader,buf,&stop);
        reader = reader + stop;
        
        answer1[i].resource = (struct ans_dns*)(reader);
        reader = reader + sizeof(struct ans_dns);
        
        if(ntohs(answer1[i].resource->type) == 1) //if its an ipv4 address
        {
            answer1[i].rdata = (unsigned char*)malloc(ntohs(answer1[i].resource->rdlength));
            
            for(j=0 ; j<ntohs(answer1[i].resource->rdlength) ; j++)
            {
                answer1[i].rdata[j]=reader[j];
            }
            
            answer1[i].rdata[ntohs(answer1[i].resource->rdlength)] = '\0';
            
            reader = reader + ntohs(answer1[i].resource->rdlength);
        }
        else
        {
            answer1[i].rdata = ReadName(reader,buf,&stop);
            reader = reader + stop;
        }
    }
    
    //read authoorities
    for(i=0;i<ntohs(dns->nscount);i++)
    {
        autho[i].name=ReadName(reader,buf,&stop);
        reader+=stop;
        
        autho[i].resource=(struct ans_dns*)(reader);
        reader+=sizeof(struct ans_dns);
        
        autho[i].rdata=ReadName(reader,buf,&stop);
        reader+=stop;
    }
    
    //read additional
    for(i=0;i<ntohs(dns->arcount);i++)
    {
        addit[i].name=ReadName(reader,buf,&stop);
        reader+=stop;
        
        addit[i].resource=(struct ans_dns*)(reader);
        reader+=sizeof(struct ans_dns);
        
        if(ntohs(addit[i].resource->type)==1)
        {
            addit[i].rdata = (unsigned char*)malloc(ntohs(addit[i].resource->rdlength));
            for(j=0;j<ntohs(addit[i].resource->rdlength);j++)
                addit[i].rdata[j]=reader[j];
            
            addit[i].rdata[ntohs(addit[i].resource->rdlength)]='\0';
            reader+=ntohs(addit[i].resource->rdlength);
        }
        else
        {
            addit[i].rdata=ReadName(reader,buf,&stop);
            reader+=stop;
        }
    }
    
    //print answer1
  
    
printf("\nAnswer Records : %d \n" , ntohs(dns->ancount) );
 //   puts((char)(ntohs(dns->ancount)));
    for(i=0 ; i < ntohs(dns->ancount) ; i++)
    {
        printf("Name : %s ",answer1[i].name);
        
        if(ntohs(answer1[i].resource->type) == T_A) //IPv4 address
        {
            long *p;
            p=(long*)answer1[i].rdata;
            a.sin_addr.s_addr=(*p); //working without ntohl
            printf("has IPv4 address : %s",inet_ntoa(a.sin_addr));
        }
        
        if(ntohs(answer1[i].resource->type)==5)
        {
            //Canonical name for an alias
            printf("has alias name : %s",answer1[i].rdata);
        }
        
        printf("\n");
    }
    
    //print authoorities
    printf("\nauthooritive Records : %d \n" , ntohs(dns->nscount) );
    for( i=0 ; i < ntohs(dns->nscount) ; i++)
    {
        
        printf("Name : %s ",autho[i].name);
        if(ntohs(autho[i].resource->type)==2)
        {
            printf("has nameserver : %s",autho[i].rdata);
        }
        printf("\n");
    }
    
    //print additional resource records
    printf("\nAdditional Records : %d \n" , ntohs(dns->arcount) );
    for(i=0; i < ntohs(dns->arcount) ; i++)
    {
        printf("Name : %s ",addit[i].name);
        if(ntohs(addit[i].resource->type)==1)
        {
            long *p;
            p=(long*)addit[i].rdata;
            a.sin_addr.s_addr=(*p);
            printf("has IPv4 address : %s",inet_ntoa(a.sin_addr));
        }
        printf("\n");
    }
    return;

    
}

u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;
    
    *count = 1;
    name = (unsigned char*)malloc(256);
    
    name[0]='\0';
    
    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++]=*reader;
        }
        
        reader = reader+1;
        
        if(jumped==0)
        {
            *count = *count + 1; //if we havent jumped to another location then we can count up
        }
    }
    
    name[p]='\0'; //string complete
    if(jumped==1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }
    
    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++)
    {
        p=name[i];
        for(j=0;j<(int)p;j++)
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //remove the last dot
    return name;
}


void ChangetoDnsNameFormat(unsigned char* dns,char* host_cl)
{
    int lock = 0 , i;
    strcat((char*)host_cl,".");
    
    for(i = 0 ; i < strlen((char*)host_cl) ; i++)
    {
        if(host_cl[i]=='.')
        {
            *dns++ = i-lock;
            for(;lock<i;lock++)
            {
                *dns++=host_cl[lock];
            }
            lock++; //or lock=i+1;
        }
    }
    *dns++='\0';
}
