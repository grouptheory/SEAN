/*
 * Abstract Syntax Notation One, ASN.1
 * As defined in ISO/IS 8824 and ISO/IS 8825
 * This implements a subset of the above International Standards that
 * is sufficient to implement SNMP.
 *
 * Encodes abstract data types into a machine independent stream of bytes.
 *
 */
/**********************************************************************
	Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#ifdef unix 
#include /**/ <sys/types.h>
#include /**/ <netinet/in.h>
#include /**/ <stdlib.h>
#endif

#include "asn1.h"
#include "snmp_pp.h"

#ifdef WIN32
#include <winsock.h>
#endif

#ifndef NULL
#define NULL	0
#endif

#define LENMASK 0x0ff

/*
 * asn_parse_int - pulls a long out of an ASN int type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char * asn_parse_int( unsigned char *data, 
			                   int *datalength, 
			                   unsigned char *type, 
			                   long int *intp, 
			                   int intsize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */
    unsigned char *bufp = data;
    unsigned long	    asn_length;
    long   value = 0;

    if (intsize != sizeof (long)){
	ASNERROR("not long");
	return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL){
	ASNERROR("bad length");
	return NULL;
    }
    if ((int)(asn_length + (bufp - data)) > *datalength){
	ASNERROR("overflow of message");
	return NULL;
    }
    if ((int)asn_length > intsize){
	ASNERROR("I don't support such large integers");
	return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);
    if (*bufp & 0x80)
	value = -1; /* integer is negative */
    while(asn_length--)
	value = (value << 8) | *bufp++;
    *intp = value;
    return bufp;
};


/*
 * asn_parse_unsigned_int - pulls an unsigned long out of an ASN int type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char * asn_parse_unsigned_int( unsigned char *data,	
                                        int *datalength,
                                        unsigned char *type,
                                        unsigned long *intp,
                                        int	intsize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */
    unsigned char *bufp = data;
    unsigned long	    asn_length;
    unsigned long value = 0;

	// check the size of the object being requested
    if (intsize != sizeof (long)){
	   ASNERROR("not long");
	   return NULL;
    }

	// get the type
    *type = *bufp++;

	// pick up the len
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL){
	   ASNERROR("bad length");
	   return NULL;
    }

	// check the len for message overflow
    if ((int)(asn_length + (bufp - data)) > *datalength){
	   ASNERROR("overflow of message");
	   return NULL;
    }

	// check for legal uint size
	if (( (int)asn_length > 5) || (((int)asn_length > 4) && (*bufp != 0x00))) {
	   ASNERROR("I don't support such large integers");
	   return NULL;
    }

	// check for leading  0 octet
	if (*bufp == 0x00) {
		bufp++;
		asn_length--;
	}

	// fix the returned data length value
    *datalength -= (int)asn_length + (bufp - data);

	// calculate the value
    for (long i=0;i<(long)asn_length;i++)
	   value = (value << 8) + (unsigned long) *bufp++;

	// assign return value
    *intp = value;

	// return the bumped pointer
    return bufp;
};


/*
 * asn_build_int - builds an ASN object containing an integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char * asn_build_int( unsigned char *data,
                               int *datalength,
                               unsigned char type,
                               long *intp,
                               int intsize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */

    long integer;
    unsigned long mask;

    if (intsize != sizeof (long))
	return NULL;
    integer = *intp;
    /*
     * Truncate "unnecessary" bytes off of the most significant end of this
     * 2's complement integer.  There should be no sequence of 9
     * consecutive 1's or 0's at the most significant end of the
     * integer.
     */
    mask = 0x1FF << ((8 * (sizeof(long) - 1)) - 1);
    /* mask is 0xFF800000 on a big-endian machine */
    while((((integer & mask) == 0) || ((integer & mask) == mask))
	  && intsize > 1){
	intsize--;
	integer <<= 8;
    }
    data = asn_build_header(data, datalength, type, intsize);
    if (data == NULL)
	return NULL;
    if (*datalength < intsize)
	return NULL;
    *datalength -= intsize;
    mask = 0xFF << (8 * (sizeof(long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */
    while(intsize--){
	*data++ = (unsigned char)((integer & mask) >> (8 * (sizeof(long) - 1)));
	integer <<= 8;
    }
    return data;
};


/*
 * asn_build_unsigned_int - builds an ASN object containing an integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char * asn_build_unsigned_int( unsigned char *data,   // modified data
                                        int *datalength,       // returned buffer length 
                                        unsigned char type,	   // SMI type
                                        unsigned long *intp,   // Uint to encode
                                        int intsize)           // size of uint to encode
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */

    unsigned long u_integer;
    int add_null_byte = 0;
	long u_integer_len;
	long x;

	// check uint size
    if (intsize != sizeof (long))
	   return NULL;


	// local var point to var passed in
    u_integer = *intp;

	// figure out the len 
	if ((( u_integer >> 24) & LENMASK) != 0)
	    u_integer_len = 4;
	else if ((( u_integer >> 16) & LENMASK) !=0)
		    u_integer_len = 3;
	     else if ((( u_integer >> 8) & LENMASK) !=0)
			     u_integer_len = 2;
		      else
				 u_integer_len =1;

	// check for 5 byte len where first byte will be
    // a null
	if ((( u_integer >> (8 * (u_integer_len -1))) & 0x080) !=0)	{
		u_integer_len++;
		intsize++;
	}

	// build up the header
	data = asn_build_header( data,                 // data buffer to be modified
							 datalength,           // length of data buffer
							 type,                 // SMI type to enode
							 (int)u_integer_len);  // length of BER encoded item

	// special case, add a null byte for len of 5
	if ( u_integer_len ==5) {
	  *data++ = (unsigned char) 0;
	  for (x=1;x<u_integer_len;x++)
		  *data++= (unsigned char) ( u_integer >> (8 * ((u_integer_len-1)-x)& LENMASK));
	}
	else
    {
	   for (x=0;x<u_integer_len;x++)
		  *data++= (unsigned char) ( u_integer >> (8 * ((u_integer_len-1)-x)& LENMASK));
    }
    
	return data;
};


/*
 * asn_parse_string - pulls an octet string out of an ASN octet string type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  "string" is filled with the octet string.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char * asn_parse_string( unsigned char	*data,
                                  int *datalength,
                                  unsigned char *type,
                                  unsigned char *string,
                                  int *strlength)
{
/*
 * ASN.1 octet string ::= primstring | cmpdstring
 * primstring ::= 0x04 asnlength byte {byte}*
 * cmpdstring ::= 0x24 asnlength string {string}*
 */
    unsigned char *bufp = data;
    unsigned long	    asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
	return NULL;
    if ((int)(asn_length + (bufp - data)) > *datalength){
	ASNERROR("overflow of message");
	return NULL;
    }
    if ((int)asn_length > *strlength){
	ASNERROR("I don't support such long strings");
	return NULL;
    }
	// fixed
    memcpy((char *)string, (char *)bufp,  (int)asn_length);
    *strlength = (int)asn_length;
    *datalength -= (int)asn_length + (bufp - data);
    return bufp + asn_length;
};


/*
 * asn_build_string - Builds an ASN octet string object containing the input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char * asn_build_string( unsigned char *data,
                                  int *datalength,
                                  unsigned char type,
                                  unsigned char *string,
                                  int strlength)
{
/*
 * ASN.1 octet string ::= primstring | cmpdstring
 * primstring ::= 0x04 asnlength byte {byte}*
 * cmpdstring ::= 0x24 asnlength string {string}*
 * This code will never send a compound string.
 */
    data = asn_build_header(data, datalength, type, strlength);
    if (data == NULL)
	return NULL;
    if (*datalength < strlength)
	return NULL;
	// fixed
    memcpy((unsigned char *)data,(unsigned char *)string, strlength);
    *datalength -= strlength;
    return data + strlength;
};


/*
 * asn_parse_header - interprets the ID and length of the current object.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.
 */
unsigned char *asn_parse_header( unsigned char *data,
								 int *datalength,
                                 unsigned char *type)
{
    unsigned char *bufp = data;
	register header_len;
    unsigned long	    asn_length;

    /* this only works on data types < 30, i.e. no extension octets */
    if (IS_EXTENSION_ID(*bufp)){
	ASNERROR("can't process ID >= 30");
	return NULL;
    }
    *type = *bufp;
    bufp = asn_parse_length(bufp + 1, &asn_length);
    if (bufp == NULL)
	return NULL;
    header_len = bufp - data;
    if ((int)(header_len + asn_length) > *datalength){
	ASNERROR("asn length too long");
	return NULL;
    }
    *datalength = (int)asn_length;
    return bufp;
};

/*
 * asn_build_header - builds an ASN header for an object with the ID and
 * length specified.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  This only works on data types < 30, i.e. no extension octets.
 *  The maximum length is 0xFFFF;
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.
 */
unsigned char * asn_build_header( unsigned char *data,
                                  int *datalength,
                                  unsigned char type,
                                  int length)
{
    if (*datalength < 1)
	return NULL;
    *data++ = type;
    (*datalength)--;
    return asn_build_length(data, datalength, length);
    
};

/*
 * asn_build_sequence - builds an ASN header for a sequence with the ID and
 * length specified.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  This only works on data types < 30, i.e. no extension octets.
 *  The maximum length is 0xFFFF;
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.
 */
unsigned char * asn_build_sequence( unsigned char *data,
                                    int *datalength,
                                    unsigned char type,
                                    int length)
{
    *datalength -= 4;
    if (*datalength < 0){
	*datalength += 4;	/* fix up before punting */
	return NULL;
    }
    *data++ = type;
    *data++ = (unsigned char)(0x02 | ASN_LONG_LEN);
    *data++ = (unsigned char)((length >> 8) & 0xFF);
    *data++ = (unsigned char)(length & 0xFF);
    return data;
};

/*
 * asn_parse_length - interprets the length of the current object.
 *  On exit, length contains the value of this length field.
 *
 *  Returns a pointer to the first byte after this length
 *  field (aka: the start of the data field).
 *  Returns NULL on any error.
 */
unsigned char * asn_parse_length( unsigned char *data,
                                  unsigned long  *length)
{
    unsigned char lengthbyte = *data;

    if (lengthbyte & ASN_LONG_LEN){
	lengthbyte &= ~ASN_LONG_LEN;	/* turn MSb off */
	if (lengthbyte == 0){
	    ASNERROR("We don't support indefinite lengths");
	    return NULL;
	}
	if (lengthbyte > sizeof(long)){
	    ASNERROR("we can't support data lengths that long");
	    return NULL;
	}
	// fixed
	memcpy((char *)length, (char *)data + 1, (int)lengthbyte);
	*length = ntohl(*length);
	*length >>= (8 * ((sizeof *length) - lengthbyte));
	return data + lengthbyte + 1;
    } else { /* short asnlength */
	*length = (long)lengthbyte;
	return data + 1;
    }
};

unsigned char *asn_build_length( unsigned char *data,
                                 int *datalength,
                                 int length)
{
    unsigned char    *start_data = data;

    /* no indefinite lengths sent */
    if (length < 0x80){
	if (*datalength < 1){
	    ASNERROR("build_length");
	    return NULL;
	}	    
	*data++ = (unsigned char)length;
    } else if (length <= 0xFF){
	if (*datalength < 2){
	    ASNERROR("build_length");
	    return NULL;
	}	    
	*data++ = (unsigned char)(0x01 | ASN_LONG_LEN);
	*data++ = (unsigned char)length;
    } else { /* 0xFF < length <= 0xFFFF */
	if (*datalength < 3){
	    ASNERROR("build_length");
	    return NULL;
	}	    
	*data++ = (unsigned char)(0x02 | ASN_LONG_LEN);
	*data++ = (unsigned char)((length >> 8) & 0xFF);
	*data++ = (unsigned char)(length & 0xFF);
    }
    *datalength -= (data - start_data);
    return data;

};

/*
 * asn_parse_objid - pulls an object indentifier out of an ASN object identifier type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  "objid" is filled with the object identifier.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *asn_parse_objid( unsigned char *data,
                                int *datalength,
                                unsigned char *type,
                                oid *objid,
                                int *objidlength)
{
/*
 * ASN.1 objid ::= 0x06 asnlength subidentifier {subidentifier}*
 * subidentifier ::= {leadingbyte}* lastbyte
 * leadingbyte ::= 1 7bitvalue
 * lastbyte ::= 0 7bitvalue
 */
    unsigned char *bufp = data;
    oid *oidp = objid + 1;
    unsigned long subidentifier;
    long   length;
    unsigned long	    asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
	return NULL;
    if ((int)asn_length + (bufp - data) > *datalength){
	ASNERROR("overflow of message");
	return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);

    /* Handle invalid object identifier encodings of the form 06 00 robustly */
    if (asn_length == 0)
	objid[0] = objid[1] = 0;

    length = asn_length;
    (*objidlength)--;	/* account for expansion of first byte */
    while (length > 0 && (*objidlength)-- > 0){
	subidentifier = 0;
	do {	/* shift and add in low order 7 bits */
	    subidentifier = (subidentifier << 7) + (*(unsigned char *)bufp & ~ASN_BIT8);
	    length--;
	} while (*(unsigned char *)bufp++ & ASN_BIT8);	/* last byte has high bit clear */
	if (subidentifier > (unsigned long)MAX_SUBID){
	    ASNERROR("subidentifier too long");
	    return NULL;
	}
	*oidp++ = (oid)subidentifier;
    }

    /*
     * The first two subidentifiers are encoded into the first component
     * with the value (X * 40) + Y, where:
     *	X is the value of the first subidentifier.
     *  Y is the value of the second subidentifier.
     */
    subidentifier = (unsigned long)objid[1];
    if (subidentifier == 0x2B){
	objid[0] = 1;
	objid[1] = 3;
    } else {
	objid[1] = (unsigned char)(subidentifier % 40);
	objid[0] = (unsigned char)((subidentifier - objid[1]) / 40);
    }

    *objidlength = (int)(oidp - objid);
    return bufp;
};

/*
 * asn_build_objid - Builds an ASN object identifier object containing the
 * input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *asn_build_objid( unsigned char *data,
                                int *datalength,
                                unsigned char type,
                                oid *objid,
                                int objidlength)
{
/*
 * ASN.1 objid ::= 0x06 asnlength subidentifier {subidentifier}*
 * subidentifier ::= {leadingbyte}* lastbyte
 * leadingbyte ::= 1 7bitvalue
 * lastbyte ::= 0 7bitvalue
 */
    unsigned char buf[MAX_OID_LEN];
    unsigned char *bp = buf;
    oid *op = objid;
    int    asnlength;
    unsigned long subid, mask, testmask;
    int bits, testbits;

    if (objidlength < 2){
	*bp++ = 0;
	objidlength = 0;
    } else {
	*bp++ = (unsigned char) (op[1] + (op[0] * 40));
	objidlength -= 2;
	op += 2;
    }

    while(objidlength-- > 0){
	subid = *op++;
	if (subid < 127){ /* off by one? */
	    *bp++ = (unsigned char )subid;
	} else {
	    mask = 0x7F; /* handle subid == 0 case */
	    bits = 0;
	    /* testmask *MUST* !!!! be of an unsigned type */
	    for(testmask = 0x7F, testbits = 0; testmask != 0;
		testmask <<= 7, testbits += 7){
		if (subid & testmask){	/* if any bits set */
		    mask = testmask;
		    bits = testbits;
		}
	    }
	    /* mask can't be zero here */
	    for(;mask != 0x7F; mask >>= 7, bits -= 7){
		/* fix a mask that got truncated above */
		if (mask == 0x1E00000)  
		    mask = 0xFE00000;
		*bp++ = (unsigned char)(((subid & mask) >> bits) | ASN_BIT8);
	    }
	    *bp++ = (unsigned char)(subid & mask);
	}
    }
    asnlength = bp - buf;
    data = asn_build_header(data, datalength, type, asnlength);
    if (data == NULL)
	return NULL;
    if (*datalength < asnlength)
	return NULL;
	// fixed
    memcpy((char *)data, (char *)buf,  asnlength);
    *datalength -= asnlength;
    return data + asnlength;
};

/*
 * asn_parse_null - Interprets an ASN null type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *asn_parse_null(unsigned char	*data,
                              int *datalength,
                              unsigned char *type)
{
/*
 * ASN.1 null ::= 0x05 0x00
 */
    unsigned char   *bufp = data;
    unsigned long	    asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
	return NULL;
    if (asn_length != 0){
	ASNERROR("Malformed NULL");
	return NULL;
    }
    *datalength -= (bufp - data);
    return bufp + asn_length;
};


/*
 * asn_build_null - Builds an ASN null object.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *asn_build_null( unsigned char *data,
                               int *datalength,
                               unsigned char type)
{
/*
 * ASN.1 null ::= 0x05 0x00
 */
    return asn_build_header(data, datalength, type, 0);
};

/*
 * asn_parse_bitstring - pulls a bitstring out of an ASN bitstring type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  "string" is filled with the bit string.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *asn_parse_bitstring( unsigned char *data,
                                    int *datalength,
                                    unsigned char *type,
                                    unsigned char *string,
                                    int *strlength)
{
/*
 * bitstring ::= 0x03 asnlength unused {byte}*
 */
    unsigned char *bufp = data;
    unsigned long	    asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
	return NULL;
    if ((int)(asn_length + (bufp - data)) > *datalength){
	ASNERROR("overflow of message");
	return NULL;
    }
    if ((int) asn_length > *strlength){
	ASNERROR("I don't support such long bitstrings");
	return NULL;
    }
    if (asn_length < 1){
	ASNERROR("Invalid bitstring");
	return NULL;
    }
    if (*bufp < 0 || *bufp > 7){
	ASNERROR("Invalid bitstring");
	return NULL;
    }
	// fixed
    memcpy((char *)string,(char *)bufp,  (int)asn_length);
    *strlength = (int)asn_length;
    *datalength -= (int)asn_length + (bufp - data);
    return bufp + asn_length;
};


/*
 * asn_build_bitstring - Builds an ASN bit string object containing the
 * input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *asn_build_bitstring( unsigned char *data,
                                    int *datalength,
                                    unsigned char type,	
                                    unsigned char *string,
                                    int strlength)
{
/*
 * ASN.1 bit string ::= 0x03 asnlength unused {byte}*
 */
    if (strlength < 1 || *string < 0 || *string > 7){
	ASNERROR("Building invalid bitstring");
	return NULL;
    }
    data = asn_build_header(data, datalength, type, strlength);
    if (data == NULL)
	return NULL;
    if (*datalength < strlength)
	return NULL;
	// fixed
    memcpy((char *)data,(char *)string, strlength);
    *datalength -= strlength;
    return data + strlength;
};


/*
 * asn_parse_unsigned_int64 - pulls a 64 bit unsigned long out of an ASN int
 * type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char * asn_parse_unsigned_int64(  unsigned char *data,
                                           int *datalength,
                                           unsigned char *type,
                                           struct counter64 *cp,
                                           int countersize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */
    unsigned char *bufp = data;
    unsigned long	    asn_length;
    unsigned long low = 0, high = 0;
    int intsize = 4;
    
    if (countersize != sizeof(struct counter64)){
	ASNERROR("not right size");
	return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL){
	ASNERROR("bad length");
	return NULL;
    }
    if ((int)(asn_length + (bufp - data)) > *datalength){
	ASNERROR("overflow of message");
	return NULL;
    }
    if (((int)asn_length > (intsize * 2 + 1)) ||
	(((int)asn_length == (intsize * 2) + 1) && *bufp != 0x00)){
	ASNERROR("I don't support such large integers");
	return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);
    if (*bufp & 0x80){
	low = (unsigned long) -1; // integer is negative 
	high = (unsigned long) -1; 
    }
    while(asn_length--){
	high = (high << 8) | ((low & 0xFF000000) >> 24);
	low = (low << 8) | *bufp++;
    }
    cp->low = low;
    cp->high = high;
    return bufp;
};


/*
 * asn_build_unsigned_int64 - builds an ASN object containing a 64 bit integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char * asn_build_unsigned_int64( unsigned char *data,
                                          int *datalength,
                                          unsigned char	type,
                                          struct counter64 *cp,
                                          int countersize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */

    unsigned long low, high;
    unsigned long mask, mask2;
    int add_null_byte = 0;
    int intsize;

    if (countersize != sizeof (struct counter64))
	return NULL;
    intsize = 8;
    low = cp->low;
    high = cp->high;
    mask = 0xFF << (8 * (sizeof(long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */
    if ((unsigned char)((high & mask) >> (8 * (sizeof(long) - 1))) & 0x80){
	/* if MSB is set */
	add_null_byte = 1;
	intsize++;
    }
    /*
     * Truncate "unnecessary" bytes off of the most significant end of this 2's
     * complement integer.
     * There should be no sequence of 9 consecutive 1's or 0's at the most
     * significant end of the integer.
     */
    mask2 = 0x1FF << ((8 * (sizeof(long) - 1)) - 1);
    /* mask2 is 0xFF800000 on a big-endian machine */
    while((((high & mask2) == 0) || ((high & mask2) == mask2))
	  && intsize > 1){
	intsize--;
	high = (high << 8)
	    | ((low & mask) >> (8 * (sizeof(long) - 1)));
	low <<= 8;
    }
    data = asn_build_header(data, datalength, type, intsize);
    if (data == NULL)
	return NULL;
    if (*datalength < intsize)
	return NULL;
    *datalength -= intsize;
    if (add_null_byte == 1){
	*data++ = '\0';
	intsize--;
    }
    while(intsize--){
	*data++ = (unsigned char)((high & mask) >> (8 * (sizeof(long) - 1)));
	high = (high << 8)
	    | ((low & mask) >> (8 * (sizeof(long) - 1)));
	low <<= 8;
	
    }
    return data;
};


// create a pdu
struct snmp_pdu * snmp_pdu_create( int command)
{
    struct snmp_pdu *pdu;

    pdu = (struct snmp_pdu *)malloc(sizeof(struct snmp_pdu));
    memset((char *)pdu, 0,sizeof(struct snmp_pdu));
    pdu->command = command;
    pdu->errstat = 0;
    pdu->errindex = 0;
    pdu->enterprise = NULL;
    pdu->enterprise_length = 0;
    pdu->variables = NULL;
    return pdu;
};

// free a pdu
void snmp_free_pdu( struct snmp_pdu *pdu)
{
    struct variable_list *vp, *ovp;

    vp = pdu->variables;
    while(vp){
	  // free the oid part
	  if (vp->name)
	    free((char *)vp->name);
	  // if deep data, then free as well
	  if (vp->val.string)
	    free((char *)vp->val.string);
	  ovp = vp;
	  // go to the next one
	  vp = vp->next_variable;
	  // free up vb itself
	  free((char *)ovp);
    }
	// if enterprise free it up
    if (pdu->enterprise)
	  free((char *)pdu->enterprise);
	// free up pdu itself
    free((char *)pdu);
};


// add a null var to a pdu
void snmp_add_var(struct snmp_pdu *pdu, 
			      oid *name, 
			      int name_length,
			      SmiVALUE *smival)
{
    struct variable_list *vars;

	// if we don't have a vb list ,create one
    if (pdu->variables == NULL)
	  pdu->variables = vars = (struct variable_list *)malloc(sizeof(struct variable_list));
    else 
	{ // we have one, find the end
	  for(vars = pdu->variables; vars->next_variable; vars = vars->next_variable);
	  // create a new one
	  vars->next_variable = (struct variable_list *)malloc(sizeof(struct variable_list));
	  // bump ptr
	  vars = vars->next_variable;
    }

	// add the oid with no data
    vars->next_variable = NULL;

	// hook in the Oid portion
    vars->name = (oid *)malloc(name_length * sizeof(oid));
	// fixed
    memcpy((char *)vars->name,(char *)name, name_length * sizeof(oid));
    vars->name_length = name_length;

	// hook in the SMI value
    switch( smival->syntax)
    {
       // null , do nothing
       case sNMP_SYNTAX_NULL:
       case sNMP_SYNTAX_NOSUCHOBJECT:
       case sNMP_SYNTAX_NOSUCHINSTANCE:
       case sNMP_SYNTAX_ENDOFMIBVIEW:
	   {
	      vars->type = (unsigned char) smival->syntax;
          vars->val.string = NULL;
          vars->val_len = 0;
	   }
       break;

       // octects
       case sNMP_SYNTAX_OCTETS:
       case sNMP_SYNTAX_OPAQUE:
	   case sNMP_SYNTAX_IPADDR:
       {
		  vars->type = (unsigned char) smival->syntax;
		  vars->val.string = (unsigned char *)malloc((unsigned)smival->value.string.len);
		  vars->val_len = (int) smival->value.string.len;
		  memcpy( (unsigned char *) vars->val.string,
			      (unsigned char *) smival->value.string.ptr,
				  (unsigned) smival->value.string.len);
       }
       break;

       // oid
       case sNMP_SYNTAX_OID:
	   {
		vars->type = (unsigned char) smival->syntax;
        vars->val_len = (int) smival->value.oid.len * sizeof(oid);
		vars->val.objid = (oid *)malloc((unsigned)vars->val_len);
		memcpy((unsigned long *)vars->val.objid,
			   (unsigned long *)smival->value.oid.ptr,
			   (unsigned) vars->val_len);
       }
       break;


	   
	   case sNMP_SYNTAX_TIMETICKS:
	   case sNMP_SYNTAX_CNTR32:
	   case sNMP_SYNTAX_GAUGE32:
	   case sNMP_SYNTAX_UINT32:
	   {
		 long templong;
		 vars->type = (unsigned char) smival->syntax;
		 vars->val.integer = (long *)malloc(sizeof(long));
		 vars->val_len = sizeof(long);
		 templong = (long) smival->value.uNumber;
		 memcpy( (long*) vars->val.integer,
			     (long*) &templong,
				 sizeof(long));
	   }
	   break;

	   case sNMP_SYNTAX_INT32:
	   {
	   	 long templong;
		 vars->type = (unsigned char) smival->syntax;
		 vars->val.integer = (long *)malloc(sizeof(long));
		 vars->val_len = sizeof(long);
		 templong = (long) smival->value.sNumber;
		 memcpy( (long*) vars->val.integer,
			     (long*) &templong,
				 sizeof(long));
	   }
	   break;

       // 64 bit counter
       case sNMP_SYNTAX_CNTR64:
	   {
		 vars->type = ( unsigned char) smival->syntax;
		 vars->val.counter64 = (struct counter64 *)malloc( sizeof(struct counter64) );
		 vars->val_len = sizeof(struct counter64);
		 memcpy( (struct counter64*) vars->val.counter64,
			     (SmiLPCNTR64) &(smival->value.hNumber),
				 sizeof( SmiCNTR64));
       }
       break;

    } // end switch

};

// build the authentication
// works for v1 or v2c
unsigned char *snmp_auth_build( unsigned char *data,
                         int *length,
                         long int version,
						 unsigned char *community,
						 int community_len,
                         int messagelen)
{
	unsigned char *params;
	int     plen;

	params = community;
	plen = community_len;

	data = asn_build_sequence(data, 
		                      length, 
							  (unsigned char)(ASN_SEQUENCE | ASN_CONSTRUCTOR), 
	                          messagelen + plen + 5);
	if (data == NULL){
		ASNERROR("buildheader");
		return NULL;
	}
	data = asn_build_int(data, 
		                 length,
		                 (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		                 (long *)&version, 
						 sizeof(version));
	if (data == NULL){
		ASNERROR("buildint");
		return NULL;
	}

	data = asn_build_string(data, 
		                   length,
	                       (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR), 
						   params, 
						   plen );
	if (data == NULL){
		ASNERROR("buildstring");
		return NULL;
	}

	return (unsigned char *)data;
};


// build a variable binding
unsigned char * snmp_build_var_op(unsigned char *data, 
								  oid * var_name, 
								  int *var_name_len, 
								  unsigned char var_val_type, 
								  int var_val_len,
		                          unsigned char *var_val, 
								  int *listlength)

{
    int dummyLen, headerLen;
    unsigned char *dataPtr;

    dummyLen = *listlength;
    dataPtr = data;


    data += 4;
    dummyLen -=4;
    if (dummyLen < 0)
	  return NULL;

    headerLen = data - dataPtr;
    *listlength -= headerLen;
    data = asn_build_objid( data, 
		                    listlength,
	                        (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OBJECT_ID),
	                        var_name, 
							*var_name_len);
    if (data == NULL){
	  ASNERROR("");
	  return NULL;
    }

	// based on the type...
    switch(var_val_type){
	case ASN_INTEGER:
	  data = asn_build_int( data, 
		                    listlength, 
							var_val_type,
		                    (long *)var_val, 
							var_val_len);
	break;

	case SMI_GAUGE:
	case SMI_COUNTER:
	case SMI_TIMETICKS:
	case SMI_UINTEGER:
      data = asn_build_unsigned_int( data, 
		                             listlength, 
									 var_val_type,
					                 (unsigned long *)var_val, 
									 var_val_len);
    break;

	case SMI_COUNTER64:
	    data = asn_build_unsigned_int64(data, 
			                            listlength, 
										var_val_type,
					                    (struct counter64 *)var_val,
					                    var_val_len);
    break;

	case ASN_OCTET_STR:
	case SMI_IPADDRESS:
	case SMI_OPAQUE:
    case SMI_NSAP:
	    data = asn_build_string(data, listlength, var_val_type,
		    var_val, var_val_len);
	    break;

	case ASN_OBJECT_ID:
	    data = asn_build_objid(data, listlength, var_val_type,
		    (oid *)var_val, var_val_len / sizeof(oid));
	    break;

	case ASN_NULL:
	    data = asn_build_null(data, listlength, var_val_type);
	    break;

	case ASN_BIT_STR:
	    data = asn_build_bitstring(data, listlength, var_val_type,
		    var_val, var_val_len);
	    break;

	case SNMP_NOSUCHOBJECT:
	case SNMP_NOSUCHINSTANCE:
	case SNMP_ENDOFMIBVIEW:
	    data = asn_build_null(data, listlength, var_val_type);
	    break;

	default:
	    ASNERROR("wrong type");
	    return NULL;
    }
    if (data == NULL){
	  ASNERROR("");
	  return NULL;
    }
    dummyLen = (data - dataPtr) - headerLen;

    asn_build_sequence(dataPtr, 
		               &dummyLen,
		               (unsigned char)(ASN_SEQUENCE | ASN_CONSTRUCTOR), 
					   dummyLen);
    return data;
};


// serialize the pdu
int snmp_build( struct snmp_pdu	*pdu, 
			    unsigned char *packet, 
				int *out_length, 
				long version,
				unsigned char* community,
				int community_len)
{
    unsigned char  buf[SNMP_MSG_LENGTH];
    unsigned char  *cp;
    struct   variable_list *vp;
    int	     length;
    long int zero = 0;
    int	 totallength;

    length = *out_length;
    cp = packet;
    for(vp = pdu->variables; vp; vp = vp->next_variable){
	   cp = snmp_build_var_op( cp, 
		                       vp->name, 
		                       &vp->name_length, 
							   vp->type, 
							   vp->val_len, 
							   (unsigned char *)vp->val.string, 
							   &length);
	  if (cp == NULL)
	    return -1;
    }
    totallength = cp - packet;

    length = SNMP_MSG_LENGTH;
	
	// encode the total len
    cp = asn_build_header( buf, 
		                   &length, 
						   (unsigned char)(ASN_SEQUENCE | ASN_CONSTRUCTOR), 
						   totallength);
    if (cp == NULL)
	  return -1;
	memcpy( (char *)cp, (char *)packet,totallength);
    totallength += cp - buf;

    length = *out_length;
    if (pdu->command != TRP_REQ_MSG) {

	   // request id 
	   cp = asn_build_int( packet, 
		                   &length,
	                       (unsigned char )(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
	                       (long *)&pdu->reqid, 
						   sizeof(pdu->reqid));
	   if (cp == NULL)
	      return -1;

	   // error status 
	   cp = asn_build_int(cp, 
		                  &length,
		                  (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		                  (long *)&pdu->errstat, sizeof(pdu->errstat));
	   if (cp == NULL)
	      return -1;

	   // error index 
	   cp = asn_build_int(cp, 
		                  &length,
		                  (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		                  (long *)&pdu->errindex, sizeof(pdu->errindex));
	   if (cp == NULL)
	     return -1;
    } 
	else {	// this is a trap message 

	   // enterprise 
	   cp = asn_build_objid( packet, 
		                     &length,
	                         (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OBJECT_ID),
	                         (oid *)pdu->enterprise, 
							 pdu->enterprise_length);
	   if (cp == NULL)
	      return -1;

	   // agent-addr 
	   cp = asn_build_string(cp, 
		                     &length,
		                     (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR),
		                     (unsigned char *)&pdu->agent_addr.sin_addr.s_addr, 
							 sizeof(pdu->agent_addr.sin_addr.s_addr));
	   if (cp == NULL)
	     return -1;

	  // generic trap 
	  cp = asn_build_int(cp, 
		                 &length,
		                 (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		                 (long *)&pdu->trap_type, 
						 sizeof(pdu->trap_type));
	  if (cp == NULL)
	     return -1;

	  // specific trap 
	  cp = asn_build_int( cp, 
		                  &length,
		                  (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		                  (long *)&pdu->specific_type, 
						  sizeof(pdu->specific_type));
	  if (cp == NULL)
	    return -1;

	  // timestamp  
	  cp = asn_build_int(cp, 
		                 &length,
		                 (unsigned char )(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		                 (long *)&pdu->time, 
						 sizeof(pdu->time));
	  if (cp == NULL)
	      return -1;
    }

    if (length < totallength)
	  return -1;
	// fixed
    memcpy((char *)cp, (char *)buf, totallength);
    totallength += cp - packet;

    length = SNMP_MSG_LENGTH;
    cp = asn_build_header(buf, 
		                  &length, 
						  (unsigned char)pdu->command, 
						  totallength);
    if (cp == NULL)
	  return -1;
    if (length < totallength)
	  return -1;
	// fixed
    memcpy((char *)cp, (char *)packet, totallength);
    totallength += cp - buf;

    length = *out_length;

    cp = snmp_auth_build( packet, 
		                  &length, 
						  version, 
						  community,
						  community_len,
						  totallength );
    if (cp == NULL)
	  return -1;
    if ((*out_length - (cp - packet)) < totallength)
	  return -1;
	// fixed
    memcpy((char *)cp, (char *)buf, totallength);
    totallength += cp - packet;
    *out_length = totallength;

    return 0;
};

// parse the authentication header
unsigned char *snmp_auth_parse(unsigned char *data,
                               int *length,
                               unsigned char *sid,
							   int *slen,
                               long	*version)
{
    unsigned char type;

	// get the type
    data = asn_parse_header( data, 
		                     length, 
							 &type);
    if (data == NULL){
	  ASNERROR("bad header");
	  return NULL;
    }

    if (type != (ASN_SEQUENCE | ASN_CONSTRUCTOR)){
	   ASNERROR("wrong auth header type");
	   return NULL;
    }

	// get the version
    data = asn_parse_int(data, 
		                 length, 
						 &type, 
						 version, 
						 sizeof(*version));
    if (data == NULL){
	   ASNERROR("bad parse of version");
	   return NULL;
    }

	// get the community name
    data = asn_parse_string(data, 
		                    length, 
							&type, 
							sid, 
							slen);
    if (data == NULL){
	  ASNERROR("bad parse of community");
	  return NULL;
    }

    return (unsigned char *)data;
};

unsigned char *
snmp_parse_var_op( unsigned char *data,  // IN - pointer to the start of object 
    oid	    *var_name,	                 // OUT - object id of variable 
    int	    *var_name_len,               // IN/OUT - length of variable name 
    unsigned char  *var_val_type,        // OUT - type of variable (int or octet string) (one byte) 
    int	    *var_val_len,                // OUT - length of variable
    unsigned char  **var_val,	         // OUT - pointer to ASN1 encoded value of variable
    int	    *listlength)                 // IN/OUT - number of valid bytes left in var_op_list
{
    unsigned char var_op_type;
    int	var_op_len = *listlength;
    unsigned char *var_op_start = data;

    data = asn_parse_header(data, &var_op_len, &var_op_type);
    if (data == NULL){
	ASNERROR("");
	return NULL;
    }
    if (var_op_type != (unsigned  char)(ASN_SEQUENCE | ASN_CONSTRUCTOR))
	return NULL;
    data = asn_parse_objid(data, &var_op_len, &var_op_type, var_name, var_name_len);
    if (data == NULL){
	ASNERROR("");
	return NULL;
    }
    if (var_op_type != (unsigned char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OBJECT_ID))
	return NULL;
    *var_val = data;	/* save pointer to this object */
    /* find out what type of object this is */
    data = asn_parse_header(data, &var_op_len, var_val_type);
    if (data == NULL){
	ASNERROR("");
	return NULL;
    }
    *var_val_len = var_op_len;
    data += var_op_len;
    *listlength -= (int)(data - var_op_start);
    return data;
};



// build a pdu from a data and length
int snmp_parse( struct snmp_pdu *pdu,
                unsigned char  *data,
				unsigned char *community_name,
				unsigned long &community_len,
				snmp_version &spp_version,
                int length)
{
    unsigned char  msg_type;
    unsigned char  type;
    unsigned char  *var_val;
    long    version;
    int	    len, four;
    unsigned char community[256];
    int community_length = 256;
    struct variable_list *vp;
    oid	    objid[MAX_NAME_LEN], *op;
    unsigned char  *origdata = data;
    int      origlength = length;
    unsigned char  *save_data;

    // authenticates message and returns length if valid 
    data = snmp_auth_parse(data, 
		                   &length, 
						   community, 
						   &community_length, 
						   &version);
    if (data == NULL)
	return -1;

	// copy the returned community name
	memcpy( (unsigned char *) community_name, 
		    (unsigned char *) community, 
			community_length);
	community_len = (long int) community_length;

    if( version != SNMP_VERSION_1 && version != SNMP_VERSION_2C ) {
	    ASNERROR("Wrong version");
	    return -1;
    }

	spp_version = (snmp_version) version;

    save_data = data;

    data = asn_parse_header(data, 
		                    &length, 
							&msg_type);
    if (data == NULL)
	return -1;
    pdu->command = msg_type;

    if (pdu->command != TRP_REQ_MSG){
	   // get the rid
	   data = asn_parse_int(data, 
		                    &length, &type, 
							(long *)&pdu->reqid, 
							sizeof(pdu->reqid));
	   if (data == NULL)
	     return -1;
	   // get the error status
	   data = asn_parse_int(data, 
		                    &length, 
							&type, 
							(long *)&pdu->errstat, 
							sizeof(pdu->errstat));
	   if (data == NULL)
	     return -1;
	   // get the error index
	   data = asn_parse_int(data, 
		                    &length, 
							&type, 
							(long *)&pdu->errindex, 
							sizeof(pdu->errindex));
	   if (data == NULL)
	      return -1;
    } 
	else {  // is a trap

	   // get the enterprise
	   pdu->enterprise_length = MAX_NAME_LEN;
	   data = asn_parse_objid(data, 
		                      &length, 
							  &type, 
							  objid, 
							  &pdu->enterprise_length);
	   if (data == NULL)
	     return -1;

	   pdu->enterprise = (oid *)malloc(pdu->enterprise_length * sizeof(oid));
	   // fixed
	   memcpy((char *)pdu->enterprise,(char *)objid,  pdu->enterprise_length * sizeof(oid));

	   // get source address
	   four = 4;
	   data = asn_parse_string(data, 
		                       &length, 
							   &type, (unsigned char *)&pdu->agent_addr.sin_addr.s_addr, 
							   &four);
	   if (data == NULL)
	     return -1;

	   // get trap type
	   data = asn_parse_int(data, 
		                    &length, 
							&type, 
							(long *)&pdu->trap_type, 
							sizeof(pdu->trap_type));
	   if (data == NULL)
	     return -1;

	   // trap type
	   data = asn_parse_int(data, 
		                    &length, 
							&type, 
							(long *)&pdu->specific_type, 
							sizeof(pdu->specific_type));
	   if (data == NULL)
	     return -1;

	   // timestamp
	   data = asn_parse_int(data, &length, &type, (long *)&pdu->time, sizeof(pdu->time));
	   if (data == NULL)
	      return -1;
    }

	// get the vb list
    data = asn_parse_header(data, &length, &type);
    if (data == NULL)
	   return -1;
    if (type != (unsigned char)(ASN_SEQUENCE | ASN_CONSTRUCTOR))
	   return -1;
    while((int)length > 0){
	if (pdu->variables == NULL){
	    pdu->variables = vp = (struct variable_list *)malloc(sizeof(struct variable_list));
	} else {
	    vp->next_variable = (struct variable_list *)malloc(sizeof(struct variable_list));
	    vp = vp->next_variable;
	}
	vp->next_variable = NULL;
	vp->val.string = NULL;
	vp->name = NULL;
	vp->name_length = MAX_NAME_LEN;
	data = snmp_parse_var_op( data, 
		                      objid, 
							  &vp->name_length, 
							  &vp->type, 
							  &vp->val_len, 
							  &var_val, 
							  (int *)&length);
	if (data == NULL)
	    return -1;
	op = (oid *)malloc((unsigned)vp->name_length * sizeof(oid));
	// fixed
	memcpy((char *)op, (char *)objid, vp->name_length * sizeof(oid));
	vp->name = op;

	len = SNMP_MSG_LENGTH;
	switch((short)vp->type){
	    case ASN_INTEGER:
		   vp->val.integer = (long *)malloc(sizeof(long));
		   vp->val_len = sizeof(long);
		   asn_parse_int(var_val, &len, &vp->type, (long *)vp->val.integer, sizeof(vp->val.integer));
		break;

	    case SMI_COUNTER:
	    case SMI_GAUGE:
	    case SMI_TIMETICKS:
	    case SMI_UINTEGER:
		   vp->val.integer = (long *)malloc(sizeof(long));
		   vp->val_len = sizeof(long);
		   asn_parse_unsigned_int(var_val, &len, &vp->type, (unsigned long *)vp->val.integer, sizeof(vp->val.integer));
		break;

	    case SMI_COUNTER64:
		   vp->val.counter64 = (struct counter64 *)malloc( sizeof(struct counter64) );
		   vp->val_len = sizeof(struct counter64);
		   asn_parse_unsigned_int64(var_val, &len, &vp->type,
					    (struct counter64 *)vp->val.counter64,
					    sizeof(*vp->val.counter64));
		break;
	    
		case ASN_OCTET_STR:
	    case SMI_IPADDRESS:
	    case SMI_OPAQUE:
	    case SMI_NSAP:
		   vp->val.string = (unsigned char *)malloc((unsigned)vp->val_len);
		   asn_parse_string(var_val, &len, &vp->type, vp->val.string, &vp->val_len);
		break;

	    case ASN_OBJECT_ID:
		   vp->val_len = MAX_NAME_LEN;
		   asn_parse_objid(var_val, &len, &vp->type, objid, &vp->val_len);
		   //vp->val_len *= sizeof(oid);
		   vp->val.objid = (oid *)malloc((unsigned)vp->val_len * sizeof(oid));
		   // fixed
		   memcpy((char *)vp->val.objid,
			      (char *)objid,
			      vp->val_len * sizeof(oid));
		break;

        case SNMP_NOSUCHOBJECT:
        case SNMP_NOSUCHINSTANCE:
        case SNMP_ENDOFMIBVIEW:
	    case ASN_NULL:
		break;

	    default:
		   ASNERROR("bad type returned ");
		break;
	}
    }
    return 0;
};



