#include <stdlib.h>

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
};

typedef struct que_dns {
	unsigned short qtype;
	unsigned short qclass;
};

typedef struct ans_dns {
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short rdlength;
};

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