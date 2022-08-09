
//#include <quickmail.h>

#include "CSmtp.h"
#include <StandardDef.h>

#include "Email.h"


void		SendEmail( const char* szFromName, const char* szFromAddress, const char* szTo, const char* szSubject, const char* szBody )
{
	printf( "Sending email to %s (%s)\n", szTo, szSubject );

    CSmtp mail;
    mail.SetSMTPServer("smtp.virginmedia.com",465);
    mail.SetSecurityType(USE_SSL);
    mail.SetLogin("galaxyserver@virginmedia.com");
    mail.SetPassword("spirit99");
    mail.SetSenderName(szFromName);
	mail.SetSenderMail(szFromAddress );
	mail.AddRecipient(szTo);
	mail.SetSubject(szSubject);
	mail.AddMsgLine( szBody );
    mail.Send();
}
