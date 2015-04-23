#include <stdlib.h>



typedef struct header {
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
	unsigned short ancount;
	unsigned short nscount;
	unsigned short arcount;
} DNS_HEADER;

typedef struct que_dns {
	unsigned short qtype;
	unsigned short qclass;
} QUE_DNS;

typedef struct ans_dns {
	unsigned short type;
	unsigned short class;
	unsigned int ttl;
	unsigned short rdlength;
} ANS_DNS;