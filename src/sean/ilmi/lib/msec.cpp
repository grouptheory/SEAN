#include "msec.h"
#include "stdio.h"
/*
  Copyright (c) 1996
  Hewlett-Packard Company

  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  Permission to use, copy, modify, distribute and/or sell this software
  and/or its documentation is hereby granted without fee. User agrees
  to display the above copyright notice and this license notice in all
  copies of the software and any documentation of the software. User
  agrees to assume all liability for the use of the software; Hewlett-Packard
  makes no representations about the suitability of this software for any
  purpose. It is provided "AS-IS without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.
*/

// For debuggin cout's
#include <iostream.h>
#ifdef sun
#include <string.h>
#endif

msec::msec() {
  refresh();
}

msec::msec(const msec &in_msec) {
  *this = in_msec;
}

int operator==(const msec &t1, const msec &t2) {
  return((t1.m_time.tv_sec == t2.m_time.tv_sec) && 
	 (t1.m_time.tv_usec == t2.m_time.tv_usec));
}

int operator!=(const msec &t1, const msec &t2) {
  return((t1.m_time.tv_sec != t2.m_time.tv_sec) || 
	 (t1.m_time.tv_usec != t2.m_time.tv_usec));
}

int operator<=(const msec &t1, const msec &t2) {
  return((t1 < t2) || (t1 == t2));
}

int operator>=(const msec &t1, const msec &t2) {
  return((t1 > t2) || (t1 == t2));
}

int operator<(const msec &t1, const msec &t2) {
  if (t1.IsInfinite()) return 0;
  if (t2.IsInfinite()) return 1;
  if ((t1.m_time.tv_sec < t2.m_time.tv_sec) || 
      ((t1.m_time.tv_sec == t2.m_time.tv_sec) && 
       (t1.m_time.tv_usec < t2.m_time.tv_usec)))
    return 1;
  else
    return 0;
}

int operator>(const msec &t1, const msec &t2) {
  if (t2.IsInfinite()) return 0;
  if (t1.IsInfinite()) return 1;
  if ((t1.m_time.tv_sec > t2.m_time.tv_sec) || 
      ((t1.m_time.tv_sec == t2.m_time.tv_sec) && 
       (t1.m_time.tv_usec > t2.m_time.tv_usec)))
    return 1;
  else
    return 0;
}

msec &msec::operator-=(const long millisec) {
  timeval t1;

  // create a timeval
  t1.tv_sec = (time_t) (millisec / 1000);
  t1.tv_usec = (millisec % 1000) * 1000;
  // subtract it from this
  *this -= t1;
  return *this;
}

msec &msec::operator-=(const timeval &t1) {
  long tmp_usec = t1.tv_usec/1000;// convert usec to millisec
  if (!this->IsInfinite())
  {
    if (m_time.tv_usec < t1.tv_usec) {
      // borrow 
      m_time.tv_sec--;
      m_time.tv_usec += 1000;
    }
    m_time.tv_usec -= tmp_usec;
    m_time.tv_sec -= t1.tv_sec;
  }
  return *this;
}

msec &msec::operator+=(const long millisec) {
  timeval t1;

  // create a timeval
  t1.tv_sec = (time_t) (millisec / 1000);
  t1.tv_usec = (millisec % 1000) * 1000;
  // add it to this
  *this += t1;
  return *this;
}

msec &msec::operator+=(const timeval &t1) {
  long tmp_usec = t1.tv_usec/1000;// convert usec to millisec
  if (!this->IsInfinite())
  {
    m_time.tv_usec += tmp_usec;
    if (m_time.tv_usec > 1000) {
      // carry
      m_time.tv_sec +=  (time_t) (m_time.tv_usec / 1000);
      m_time.tv_usec = m_time.tv_usec % 1000;
    }
    m_time.tv_sec += t1.tv_sec;
  }
  return *this;
}

msec &msec::operator=(const msec &t1) {
  m_time  = t1.m_time;
  return *this;
}

msec &msec::operator=(const timeval &t1) {
  m_time.tv_sec = t1.tv_sec;
  m_time.tv_usec = t1.tv_usec/1000; // convert usec to millisec
  return *this;
}

msec::operator unsigned long() const {
  return ((m_time.tv_sec * 1000) + m_time.tv_usec);
}

void msec::refresh() {
  class timezone tzone;
  gettimeofday((timeval *)&m_time, &tzone);
  m_time.tv_usec /= 1000; // convert usec to millisec
}

void msec::SetInfinite() {
  // Set the object out into the future as far as possible.
  m_time.tv_sec = (time_t) -1;
  m_time.tv_usec = 0;
}

int msec::IsInfinite() const {
  return ((m_time.tv_sec == (time_t) -1) && 
	  (m_time.tv_usec == 0));
}

#ifdef sun
#define MAX_ALARM 1000
#endif

void msec::GetDelta(const msec &future, timeval &timeout) const {
  if (future.IsInfinite()) {
    timeout.tv_sec = (time_t)MAX_ALARM;	// max allowable select timeout
    timeout.tv_usec = 0;
  }
  else  if (future > *this) {
    msec then(future);

    if (then.m_time.tv_usec < m_time.tv_usec) {
      // borrow
      then.m_time.tv_sec--;
      then.m_time.tv_usec += 1000;
    }
    timeout.tv_sec = then.m_time.tv_sec - m_time.tv_sec;
    timeout.tv_usec = then.m_time.tv_usec - m_time.tv_usec;
    timeout.tv_usec *= 1000 ;// convert back to usec
  }
  // Never give back negative timeval's they make select() hurl
  else{
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
  }
}

void msec::GetDeltaFromNow(timeval &timeout) const {
  msec now;
  now.GetDelta(*this, timeout);
}

char *msec::get_printable() {
  struct tm *tmptr;
  char msec_buffer[5];

  tmptr = localtime((time_t *)&m_time.tv_sec);
  strftime(m_output_buffer, sizeof(m_output_buffer), "%.1H:%M:%S.", tmptr);
  sprintf(msec_buffer, "%.3d", m_time.tv_usec);
  strcat(m_output_buffer, msec_buffer);

  return(m_output_buffer);
}
