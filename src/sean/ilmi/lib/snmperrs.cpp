#include "snmp.h"

static char * positiveErrors[] = {
  "Success Status",
  "Pdu too big,see returned error_index",
  "No such VB name, see returned error index",
  "VB is read only, see returned error index",
  "General VB error, see returned error index",
  "unknown error code"
};

static char * negativeErrors[] = {
  "Success Status",
  "Unable to start transport services",
  "Unable to end transport services ",
  "Unable to construct snmp object",
  "Unable to create variable bind list",
  "Unable to create am oid var",
  "Unable to set a vb to a vbl",
  "Unable to create an entity",
  "Unable to create a context",
  "Unable to create a pdu",
  "Unable to Send a message",
  "Unable to Receive a message",
  "Unable to decode pdu",
  "Unexpected Agent response",
  "Unexpected agent response id",
  "Unexpected vb count in response",
  "Timed out while waiting for response",
  "Snmp engine is blocked ",
  "failure registering pdu child proc ",
  "failure registering class",
  "failure instantiating pdu container",
  "pdu reception queue full ",
  "response pdu error status ! 0 ",
  "async or sync illegal mode ",
  "shut down message while pumping",
  "trap port is already in use",
  "trap registration fail",
  "shutting down only one session",
  "feature not yet supported ",
  "invalid target object was used",
  "error trying to set timeout value",
  "error trying to set retry",
  "invalid trap filter",
  "unknown error code"
};

#define MAX_POS_ERROR SNMP_CLASS_GENERAL_VB_ERR
#define MAX_NEG_ERROR SNMP_CLASS_INVALID_FILTER

char *SNMPGetError(int errCode){
  if (errCode < 0){
    if (errCode < MAX_NEG_ERROR)
      errCode = -MAX_NEG_ERROR;
    else
      errCode = -errCode;
    return(negativeErrors[errCode]);
  }
  else{
    if (errCode > MAX_POS_ERROR)
      errCode = MAX_POS_ERROR;
    return(positiveErrors[errCode]);
  }
}
