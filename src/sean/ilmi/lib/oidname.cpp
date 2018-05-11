/*--------------------------------------------------------------------
 * filename: oidname.C
 *
 * Description: 
 *  This file contains the routines necessary to translate OID's to
 *  user readable names and vice versa.
 *
 * Procedures:
 *    int  *MyTranslateOid(char *, long *, int)
 *    int   MyReadMIBFromFile(char *)
 *
 * History:
 *  05/22/95 jdm    Initial design.
 *  07/20/95 jdm    Modifications for use w/ C++ library
 *-------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include "oidname.h"

MibTable::MibTable(const char *filename){
  char            in_buffer[1024];
  char            name[255], number[255], type[32];
  FILE           *fp;
  int             j = 0;
  int             line_no = 0;

  m_table_entries = 0;
  m_status = MT_UNITIALIZED;
  m_fname = NULL;

  if (!filename) 
    return;

  m_fname = new char [strlen(filename) + 1];
  strcpy(m_fname, filename);

  if ((fp = fopen(filename, "r")) == NULL) {
    m_status = MT_FILE_ACCESS_PROBLEM;
    return;
  }

  while (!feof(fp)) {
    line_no++;
    fgets(in_buffer, sizeof(in_buffer), fp);

      /*-----------------*/
      /* handle comments */
      /*-----------------*/
    j = 0;
    while (in_buffer[j] == ' ' || in_buffer[j] == '\t')
      j++;
    if ((in_buffer[0] == '#') || (in_buffer[j] == '\n')) {
      continue;
    }
    if (in_buffer[j] == '/' && in_buffer[j + 1] == '*') {
      while (!(in_buffer[j] == '*' && in_buffer[j + 1] == '/')) {
        if (in_buffer[j] == '\n' || in_buffer[j] == '\0') {
          line_no++;
          fgets(in_buffer, sizeof(in_buffer), fp);
	  j = 0;
	}
	else
	  j++;
      }
      continue;
    }

      /*----------------------*/
      /* handle real lines... */
      /*----------------------*/
    if ((sscanf(in_buffer, "%s %s %s", name, number, type)) == 3) {

        /*------------------------------*/
        /* sanity check name characters */
        /*------------------------------*/
      for (j = strlen(name) - 1; j >= 0; --j) {
        if ((!isprint((int) name[j])) || (name[j] == '.')) {
	  (void) fclose(fp);
	  fprintf(stderr, " Bad name in [%s]: Line %d: %s\n", 
	     filename, line_no, in_buffer);
	  m_status = MT_FILE_CONTENTS_BAD;
	  return;
	}
	if (name[j] == '-') {
	  name[j] = '_';	// fixup oid names such as "mib-2"
	}
      }
	/*--------------------------------*/
	/* sanity check number characters */
	/*--------------------------------*/
      for (j = strlen(number) - 1; j >= 0; --j)
        if (!(isdigit((int) number[j]) || (number[j] == '.'))){
	  (void) fclose(fp);
	  fprintf(stderr, " Bad objectid in [%s]: Line %d: %s\n", 
	     filename, line_no, in_buffer);
	  m_status = MT_FILE_CONTENTS_BAD;
	  return;
	}

      m_mib_table[m_table_entries].oid = new Oid(number);
      m_mib_table[m_table_entries].name = new char[strlen(name) + 1];
      strcpy(m_mib_table[m_table_entries].name, name);
      oid_hash_put(&m_mib_table[m_table_entries]);
      m_table_entries++;
    }
  }
  (void) fclose(fp);

  m_status = MT_GOOD;
  return;
}

MibTable::~MibTable(){

  delete [] m_fname;

  for (int i=0; i<m_table_entries; i++){
    delete m_mib_table[i].oid;
    delete [] m_mib_table[i].name;
  }

  for (i=0; i<HASH_SIZE; i++){
    Oid_link *prev, *next;

    next = m_oid_hash_tbl[i].next;
    while(next){
      prev = next;
      next = prev->next;
      delete(prev);
    }
  }
}

oid_compare(Oid & oid1, Oid & oid2)
{
  /*----------------------------------------------------*/
  /* Compare two oids returning:                        */
  /*    0 if they are equal                             */
  /*   -1 * number of elements comapred if oid1 < oid2  */
  /*   +1 * number of elements comapred if oid1 > oid2  */
  /*----------------------------------------------------*/
  unsigned long compare_len = (oid1.len() < oid2.len()?oid1.len():oid2.len());
  int i;
  for (i=0; i < compare_len; i++) {
    if (oid1[i] < oid2[i])
      return (-1 * (++i));
    if (oid1[i] > oid2[i])
      return (1 * (++i));
  }
  return 0;
}

Oid *MibTable::FindClosest(const Oid &ckey, char **name){
  int cur_index;
  int compare;
  int best_compare = 0;
  int best_match_index = -1;
  unsigned long best_match_len   = 0;
  char num_buf[20];
  Oid key(ckey);  // to get around const

  // check parms
  if (!name)
    return NULL;

  for (cur_index = 0; cur_index < m_table_entries; cur_index++){

      /*--------------------------------------------------------------*/
      /* ignore entries which are shorter than our current best match */
      /* or longer than the object id in question...                  */
      /*--------------------------------------------------------------*/
    if ((m_mib_table[cur_index].oid->len() < best_match_len) ||
	(m_mib_table[cur_index].oid->len() > key.len()))
      continue;

    compare = oid_compare(*m_mib_table[cur_index].oid, key);
    if (compare == 0) {
      /*----------------------------------------------*/
      /* Exact Match of oids up to the shorter length */  
      /*----------------------------------------------*/
      best_match_len = m_mib_table[cur_index].oid->len();
      best_match_index = cur_index;
      
      if (m_mib_table[cur_index].oid->len() == key.len()) {
	/*----------------------*/
	/* oid matched the key  */
	/*----------------------*/
	break;
      }
    }
    else if (compare < 0) {
      if (compare == -1) {
	/*-------------------------------------*/
	/* even the first elements don't match */
	/*-------------------------------------*/
	break;
      }
      /*-------------------------------------------------------------*/
      /* Oid is less than the key.  It could be a substring so check */
      /* to see if it is a better fit than our last best match if so,*/
      /* store it but keep looking for a better match...             */
      /*-------------------------------------------------------------*/
      if (compare < best_compare) {
	best_match_len = m_mib_table[cur_index].oid->len();
        best_match_index = cur_index;
	best_compare = compare;
      }
    }
    else {
      /*---------------------------------------------------------*/
      /* we have passed the point where we would expect a match  */
      /* lexicographically...                                    */
      /*---------------------------------------------------------*/
      break;
    }

  } // for

  if (best_match_index >= 0){
    *name = m_mib_table[best_match_index].name;
    return(m_mib_table[best_match_index].oid);
  }
  else {
    *name = '\0';
    return(NULL);
  }
}

Oid *MibTable::FindObject(const char *name){
  Mib_table_entry *lookup_value;

  // check parms
  if (!name)
    return NULL;

  lookup_value = oid_hash_lookup(name);
  if (lookup_value == NULL){
    return(NULL);
  }
  else
    return(lookup_value->oid);
}

hash_func(const char *name)
{
  int index = *name;
  int pos = 1;

  while (*name++ > 0) index += *name * pos++;
  index = index % HASH_SIZE;

  return(index);
}

void MibTable::oid_hash_put(Mib_table_entry *oid_ptr)
{
  int index;
  Oid_link *new_link;

  index = hash_func(oid_ptr->name);
  if (m_oid_hash_tbl[index].oid_ptr == NULL)
    m_oid_hash_tbl[index].oid_ptr = oid_ptr;
  else{
    if ((new_link = new Oid_link(oid_ptr)) != NULL){
      new_link->next = m_oid_hash_tbl[index].next;
      m_oid_hash_tbl[index].next = new_link;
    }
  }
}

Mib_table_entry *MibTable::oid_hash_lookup(const char *name)
{
  int index;
  Oid_link *new_link;

  index = hash_func(name);
  if (m_oid_hash_tbl[index].oid_ptr == NULL)
    return(NULL);
  else{
    if (strcmp(name, m_oid_hash_tbl[index].oid_ptr->name) == 0)
      return(m_oid_hash_tbl[index].oid_ptr);
    new_link = m_oid_hash_tbl[index].next;
    while(new_link != NULL){
      if (strcmp(name, new_link->oid_ptr->name) == 0)
	return(new_link->oid_ptr);
      else
	new_link = new_link->next;
    }
    return(NULL);
  }
}

char * MibTable::GetMIBName(int index){
  if ((index < 0) || (index >= m_table_entries))
    return(NULL);
  else
    return(m_mib_table[index].name);
}
