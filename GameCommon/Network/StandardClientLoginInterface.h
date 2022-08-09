
#ifndef STANDARD_CLIENT_LOGIN_INTERFACE_H
#define STANDARD_CLIENT_LOGIN_INTERFACE_H

typedef	void(*UserDetailsModifiedCallback)( const char* szUserLoginEmail, const char* szUserLoginPassword );


// To include this system..

// 1) On init, set login destination URL by calling :

extern void	StandardClientLoginInterfaceSetLoginDetails( const char* szLoginServiceURL, const char* szLoginVersionString );
//		e.g Game might do save data load and set previous login details from save data

// 2) Set UserDetailsModifiedCallback by calling

extern void	StandardClientLoginInterfaceSetUserDetailsCallback( UserDetailsModifiedCallback fnUserDetailsModifiedCallback );
//    (Called whenever the user has entered a new email,pass combo - usually the game will want to save that )
//     so will probably want something like..
//		void		GameUserDetailsModifiedCallback( const char* szLoginEmail, const char* szLoginPassword )
//		{
//    		SaveDataSetField( SAVEDATA_LOGIN_EMAIL, szLoginEmail );
//			SaveDataSetField( SAVEDATA_LOGIN_PASSWORD, szLoginPassword );
//		}

//	3)	Activate the login process using :

extern void	StandardClientLoginBegin( const char* szLoginEmail, const char* szLoginPassword );

//	4)	Call the render function repeatedly:

extern void		StandardClientLoginDisplay( int X, int Y, int W, int H );

//   5)	Check for completion using :

extern BOOL		StandardClientLoginIsComplete( void );
// (And then UserLoginClientGetLoginState() will tell you the result)


//	6) Reset the system
extern void		StandardClientLoginReset( void );



//-----------------------------------------------------------------
// Status info

extern const char*		StandardLoginClientGetShortStatusString( void );





#endif
