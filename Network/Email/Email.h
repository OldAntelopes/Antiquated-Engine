#ifndef COMMS_EMAIL_H
#define COMMS_EMAIL_H

#ifdef __cplusplus
extern "C"
{
#endif


extern	void		SendEmail( const char* szFromName, const char* szFromAddress, const char* szTo, const char* szSubject, const char* szBody );


#ifdef __cplusplus
}
#endif

#endif