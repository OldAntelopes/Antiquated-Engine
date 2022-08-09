
#ifndef BASIC_WEB_SERVER_INTERNAL_H
#define BASIC_WEB_SERVER_INTERNAL_H


#include "BasicWebServer.h"

	
extern BasicWebServerRequestHandler				BasicWebServerGetGETHandler( void );
extern BasicWebServerRequestHandler				BasicWebServerGetPOSTHandler( void );
extern BasicWebServerCloseConnectionHandler		BasicWebServerGetCloseConnectionHandler( void );





#endif