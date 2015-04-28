//DNS Query Program on Linux
//authoor : Silver Moon (m00n.silv3r@gmail.com)
//Dated : 29/4/2009
 
//Header Files
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<stdlib.h>    //malloc
#include<sys/socket.h>    //you know what this is for
#include<arpa/inet.h> //inet_addr , inet_ntoa , ntohs etc
#include<netinet/in.h>
#include<unistd.h>    //getpid
 
//List of DNS Servers registered on the system
char dns_servers1[10][100];
int dns_server_count = 0;
//Types of DNS resource records :)
 
#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authoority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server
 
//Function Prototypes
void ngethost_clbyname (unsigned char* , int);
void ChangetoDnsNameFormat (unsigned char*,unsigned char*);
unsigned char* ReadName (unsigned char*,unsigned char*,int*);
void get_dns_servers1();
 
//DNS header structure
struct header_dns
{
    unsigned short id; // identification number
 
    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authooritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
 
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authoenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available
 
    unsigned short qdcount; // number of que_dns entries
    unsigned short ancount; // number of answer entries
    unsigned short nscount; // number of authoority entries
    unsigned short arcount; // number of resource entries
};
 
//Constant sized fields of query structure
struct que_dns
{
    unsigned short qtype;
    unsigned short qclass;
};
 
//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct ans_dns
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short rdlength;
};
#pragma pack(pop)
 
//Pointers to resource record contents
struct RECORD_RES
{
    unsigned char *name;
    struct ans_dns *resource;
    unsigned char *rdata;
};
 
//Structure of a Query
typedef struct
{
    unsigned char *name;
    struct que_dns *ques;
} QUERY;
 
int main( int argc , char *argv[])
{
    unsigned char host_clname[100];
 
    //Get the DNS servers from the resolv.conf file
    get_dns_servers1();
     
    //Get the host_clname from the terminal
    printf("Enter host_clname to Lookup : ");
    scanf("%s" , host_clname);
     
    //Now get the ip of this host_clname , A record
    ngethost_clbyname(host_clname , T_A);
 
    return 0;
}
 
/*
 * Perform a DNS query by sending a packet
 * */
void ngethost_clbyname(unsigned char *host_cl , int query_type)
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
    qinfo->qclass = htons(1); //its internet (lol)
 
    printf("\nSending Packet...");
    if( sendto(s,(char*)buf,sizeof(struct header_dns) + (strlen((const char*)q_name)+1) + sizeof(struct que_dns),0,(struct sockaddr*)&dest,sizeof(dest)) < 0)
    {
        perror("sendto failed");
    }
    printf("Done");
     
    //Receive the answer
    i = sizeof dest;
    printf("\nReceiving answer...");
    if(recvfrom (s,(char*)buf , 65536 , 0 , (struct sockaddr*)&dest , (socklen_t*)&i ) < 0)
    {
        perror("recvfrom failed");
    }
    printf("Done");
 
    dns = (struct header_dns*) buf;
 
    //move ahead of the dns header and the query field
    reader = &buf[sizeof(struct header_dns) + (strlen((const char*)q_name)+1) + sizeof(struct que_dns)];
 
    printf("\nThe response contains : ");
    printf("\n %d que_dnss.",ntohs(dns->qdcount));
    printf("\n %d answer1.",ntohs(dns->ancount));
    printf("\n %d authooritative Servers.",ntohs(dns->nscount));
    printf("\n %d Additional records.\n\n",ntohs(dns->arcount));
 
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
    for(i=0 ; i < ntohs(dns->ancount) ; i++)
    {
        printf("Name : %s ",answer1[i].name);
 
        if( ntohs(answer1[i].resource->type) == T_A) //IPv4 address
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
 
/*
 * 
 * */
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
 
/*
 * Get the DNS servers from /etc/resolv.conf file on Linux
 * */
void get_dns_servers1()
{
    FILE *fp;
    char line[200] , *p;
    if((fp = fopen("/etc/resolv.conf" , "r")) == NULL)
    {
        printf("Failed opening /etc/resolv.conf file \n");
    }
     
    while(fgets(line , 200 , fp))
    {
        if(line[0] == '#')
        {
            continue;
        }
        if(strncmp(line , "nameserver" , 10) == 0)
        {
            p = strtok(line , " ");
            p = strtok(NULL , " ");
             
            //p now is the dns ip :)
            //????
        }
    }
     
    strcpy(dns_servers1[0] , "208.67.222.222");
    strcpy(dns_servers1[1] , "208.67.220.220");
}
 
/*
 * This will convert www.google.com to 3www6google3com 
 * got it :)
 * */
void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host_cl) 
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