#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "208DNS.h"

#define T_A 1

//global var
char dns_servers[10][100];

//methods
void getans(unsigned char *hostname , int que_type);
void load_dns();
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
	unsigned char host[255];
	//char **dns_addr = malloc(10 * sizeof(char *));

	load_dns();

	printf("*****please enter the hostname to lookup*****");
	scanf("%s",host);

	getans(host, T_A);
	return 0;
}

void getans(unsigned char *hostname , int que_type)
{
	unsigned char buf[65536],*qname,*reader;
	
	int i , j , stop , s;

	srand(time(NULL));

	struct sockaddr_in a;

	//struct RES_RECORD answers[20],auth[20],addit[20];   //The replies from the DNS server
	struct sockaddr_in dest;

	struct DNS_HEADER *dns = NULL;
	struct QUESTION *qinfo = NULL;

	s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);     //UDP packet for DNS queries

	dest.sin_family = AF_INET;
	dest.sin_port = htons(53);
	dest.sin_addr.s_addr = inet_addr(dns_servers[0]);

	// Set the DNS_Header now 
	dns = (struct DNS_HEADER *)&buf;
	//printf("%s",dns_servers[0]);
	
	dns->id = (unsigned short) htons(getpid());
    dns->qr = 0; //This is a query
    dns->opcode = 0; //This is a standard query
    dns->aa = 0; //Not Authoritative
    dns->tc = 0; //This message is not truncated
    dns->rd = 1; //Recursion Desired
    dns->ra = 0; //Recursion not available! hey we dont have it (lol)
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->qdcount = htons(1); //we have only 1 question
    dns->ancount = 0;
    dns->nscount = 0;
    dns->arcount = 0;

    //point to the query portion
    qname =(unsigned char*)&buf[sizeof(struct DNS_HEADER)];
 
    ChangetoDnsNameFormat(qname , hostname);



}// end of main

void load_dns()
{
    FILE *fp;
    char line[200] , *p;

	int i = 0;
    if((fp = fopen("server_db.txt" , "r")) == NULL)
    {
        printf("Failed opening server_db.txt file \n");
    }
     
    while(fgets(line , 200 , fp))
    {
    	printf(" %s line\n", line );
        if(line[0] == '#')
        {

            continue;
        }
        if(strcmp(line , "server") > 0)
        {
        		
          strcpy(dns_servers[i], strtok(line, " "));
		  strcpy(dns_servers[i], strtok(NULL, "\n"));
			++i;
        }
    }
     
    //strcpy(dns_servers[0] , "207.206.209.222");
    //strcpy(dns_servers[1] , "208.206.208.208");
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