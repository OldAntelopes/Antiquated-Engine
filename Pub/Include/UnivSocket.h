#ifndef _UNIVERSAL_SOCKET_H
#define _UNIVERSAL_SOCKET_H

#ifdef WIN32

#include <winsock.h>

#else

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>

#ifndef TYPE_SOCKET_DEF
typedef   unsigned int SOCKET;
# define TYPE_SOCKET_DEF
#endif /* def type SOCKET */

#define SOCKET_ERROR	(-1)

#endif

#ifdef WIN32
#define		UnivSocketLastError		WSAGetLastError()
#else
#define		UnivSocketLastError		errno
#endif


#endif	//_UNIVERSAL_SOCKET_H