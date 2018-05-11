#ifndef _ASN1
#define _ASN1

#ifdef WIN32
#ifndef __unix
#include <winsock.h>
#endif
#endif

#include "snmp_pp.h"
#ifndef EIGHTBIT_SUBIDS
typedef unsigned long	oid;
#define MAX_SUBID   0xFFFFFFFF
#else
typedef unsigned char	oid;
#define MAX_SUBID   0xFF
#endif

#define MAX_OID_LEN	    64	/* max subid's in an oid */

// asn.1 values
#define ASN_BOOLEAN	     (0x01)
#define ASN_INTEGER	     (0x02)
#define ASN_BIT_STR	     (0x03)
#define ASN_OCTET_STR	 (0x04)
#define ASN_NULL	     (0x05)
#define ASN_OBJECT_ID	 (0x06)
#define ASN_SEQUENCE	 (0x10)
#define ASN_SET		     (0x11)
#define ASN_UNIVERSAL	 (0x00)
#define ASN_APPLICATION  (0x40)
#define ASN_CONTEXT	     (0x80)
#define ASN_PRIVATE	     (0xC0)
#define ASN_PRIMITIVE	 (0x00)
#define ASN_CONSTRUCTOR	 (0x20)
#define ASN_LONG_LEN	 (0x80)
#define ASN_EXTENSION_ID (0x1F)
#define ASN_BIT8	     (0x80)

#define IS_CONSTRUCTOR(byte)	((byte) & ASN_CONSTRUCTOR)
#define IS_EXTENSION_ID(byte)	(((byte) & ASN_EXTENSION_ID) == ASN_EXTENSION_ID)

#define ASNERROR( string)
#define MAX_NAME_LEN   64 
#define SNMP_VERSION_1 0
#define SNMP_VERSION_2C 1


// defined types (from the SMI, RFC 1065) 
#define SMI_IPADDRESS   (ASN_APPLICATION | 0)
#define SMI_COUNTER	    (ASN_APPLICATION | 1)
#define SMI_GAUGE	    (ASN_APPLICATION | 2)
#define SMI_TIMETICKS   (ASN_APPLICATION | 3)
#define SMI_OPAQUE	    (ASN_APPLICATION | 4)
#define SMI_NSAP        (ASN_APPLICATION | 5)
#define SMI_COUNTER64   (ASN_APPLICATION | 6)
#define SMI_UINTEGER    (ASN_APPLICATION | 7)

#define GET_REQ_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x0)
#define GETNEXT_REQ_MSG	(ASN_CONTEXT | ASN_CONSTRUCTOR | 0x1)
#define GET_RSP_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x2)
#define SET_REQ_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x3)
#define TRP_REQ_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x4)

#define GETBULK_REQ_MSG	(ASN_CONTEXT | ASN_CONSTRUCTOR | 0x5)
#define INFORM_REQ_MSG	(ASN_CONTEXT | ASN_CONSTRUCTOR | 0x6)
#define TRP2_REQ_MSG	(ASN_CONTEXT | ASN_CONSTRUCTOR | 0x7)
#define REPORT_MSG	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x8)

#define SNMP_NOSUCHOBJECT    (ASN_CONTEXT | ASN_PRIMITIVE | 0x0)
#define SNMP_NOSUCHINSTANCE  (ASN_CONTEXT | ASN_PRIMITIVE | 0x1)
#define SNMP_ENDOFMIBVIEW    (ASN_CONTEXT | ASN_PRIMITIVE | 0x2)

#define SNMP_MSG_LENGTH 1500

typedef struct sockaddr_in  ipaddr;

// pdu
struct snmp_pdu {
    int	    command;	     // pdu type
    unsigned long  reqid;    // Request id 
    unsigned long  errstat;	 // Error status 
    unsigned long  errindex; // Error index 

    // Trap information 
    oid	    *enterprise;     // System OID 
    int	    enterprise_length;
    ipaddr  agent_addr;	    // address of object generating trap 
    int	    trap_type;	    // trap type 
    int	    specific_type;  // specific type 
    unsigned long  time;	// Uptime 

	// vb list
    struct variable_list *variables;
};

// vb list
struct variable_list {
    struct variable_list *next_variable;    // NULL for last variable 
    oid	    *name;                          // Object identifier of variable 
    int	    name_length;                    // number of subid's in name 
    unsigned char   type;                   // ASN type of variable 
    union {                                 // value of variable 
	long	*integer;
	unsigned char 	*string;
	oid	*objid;
	unsigned char   *bitstring;
	struct counter64 *counter64;
    } val;
    int	    val_len;
};



// prototypes for encoding routines
unsigned char * asn_parse_int( unsigned char *data, 
			                   int *datalength, 
			                   unsigned char *type, 
			                   long int *intp, 
			                   int intsize);


unsigned char * asn_parse_unsigned_int( unsigned char *data,	
                                        int *datalength,
                                        unsigned char *type,
                                        unsigned long *intp,
                                        int	intsize);

unsigned char * asn_build_int( unsigned char *data,
                               int *datalength,
                               unsigned char type,
                               long *intp,
                               int intsize);

unsigned char * asn_build_unsigned_int( unsigned char *data,
                                        int *datalength,
                                        unsigned char type,
                                        unsigned long *intp,
                                        int intsize);

unsigned char * asn_parse_string( unsigned char	*data,
                                  int *datalength,
                                  unsigned char *type,
                                  unsigned char *string,
                                  int *strlength);

unsigned char * asn_build_string( unsigned char *data,
                                  int *datalength,
                                  unsigned char type,
                                  unsigned char *string,
                                  int strlength);

unsigned char *asn_parse_header( unsigned char *data,
								 int *datalength,
                                 unsigned char *type);

unsigned char * asn_build_header( unsigned char *data,
                                  int *datalength,
                                  unsigned char type,
                                  int length);

unsigned char * asn_build_sequence( unsigned char *data,
                                    int *datalength,
                                    unsigned char type,
                                    int length);

unsigned char * asn_parse_length( unsigned char *data,
                                  unsigned long  *length);

unsigned char *asn_build_length( unsigned char *data,
                                 int *datalength,
                                 int length);

unsigned char *asn_parse_objid( unsigned char *data,
                                int *datalength,
                                unsigned char *type,
                                oid *objid,
                                int *objidlength);

unsigned char *asn_build_objid( unsigned char *data,
                                int *datalength,
                                unsigned char type,
                                oid *objid,
                                int objidlength);

unsigned char *asn_parse_null(unsigned char	*data,
                              int *datalength,
                              unsigned char *type);

unsigned char *asn_build_null( unsigned char *data,
                               int *datalength,
                               unsigned char type);

unsigned char *asn_parse_bitstring( unsigned char *data,
                                    int *datalength,
                                    unsigned char *type,
                                    unsigned char *string,
                                    int *strlength);

unsigned char *asn_build_bitstring( unsigned char *data,
                                    int *datalength,
                                    unsigned char type,	
                                    unsigned char *string,
                                    int strlength);

unsigned char * asn_parse_unsigned_int64(  unsigned char *data,
                                           int *datalength,
                                           unsigned char *type,
                                           struct counter64 *cp,
                                           int countersize);

unsigned char * asn_build_unsigned_int64( unsigned char *data,
                                          int *datalength,
                                          unsigned char	type,
                                          struct counter64 *cp,
                                          int countersize);

struct counter64 {
    unsigned long high;
    unsigned long low;
};

struct snmp_pdu * snmp_pdu_create( int command);

void snmp_free_pdu( struct snmp_pdu *pdu);

int snmp_build( struct snmp_pdu	*pdu, 
			    unsigned char *packet, 
				int *out_length, 
				long version,
				unsigned char* community,
				int community_len);

void snmp_add_var(struct snmp_pdu *pdu, 
			      oid *name, 
			      int name_length,
			      SmiVALUE *smival);

int snmp_parse( struct snmp_pdu *pdu,
                unsigned char  *data,
				unsigned char *community_name,
				unsigned long &community_len,
				snmp_version &version,
                int length);


#endif  // _ASN1

