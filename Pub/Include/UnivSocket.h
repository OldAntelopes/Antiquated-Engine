#ifndef _UNIVERSAL_SOCKET_H
#define _UNIVERSAL_SOCKET_H

#ifdef I3D_PLATFORM_S3E
#undef MARMALADE
#define MARMALADE
#endif

#ifdef WIN32

#include <winsock.h>

#else

#ifdef MARMALADE
#include <s3eSocket.h>
#else
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


#define SOCKET_ERROR (-1)

#endif
#endif


#endif	//_UNIVERSAL_SOCKET_H
