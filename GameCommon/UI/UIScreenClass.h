#ifndef ANTIQUATED_UI_SCREEN_CLASS_H
#define ANTIQUATED_UI_SCREEN_CLASS_H

class UIControl
{
public:
	// TODO 
	BOOL	IsActive( ) { return( FALSE ); }

	void	Update( float fDelta ) {};

	BOOL	OnLeftMouseUp() { return( FALSE ); }
	BOOL	OnLeftMouseDown() { return( FALSE ); }

	void			SetNext( UIControl* pNext ) { mpNext = pNext; }
	UIControl*		GetNext() { return( NULL ); } 

private:
	UIControl*	mpNext;
};

class UIScreen
{
public:
	UIScreen()
	{
		mbIsActive = false;
		mpControlsList = NULL;
		mbDrawWindowHold = false;
		mnWindowBarHover = NOTFOUND;
		mnDragWindowBasePosX = 0;
		mnDragWindowBasePosY = 0;
		mnManualPosX = 0;
		mnManualPosY = 0;
	}

	void		InitScreen( void );
	void		ActivateScreen( void );
	void		RenderScreen( int X, int Y, int W, int H, float fAlpha );
	void		UpdateScreen( float fDelta );
	void		CloseScreen( void );
	void		FreeScreen( void );

	bool		LeftMouseUp();
	bool		LeftMouseDown();

protected:
	virtual void	OnInitScreen( void ) {}
	virtual void	OnActivateScreen( void ) {}
	virtual void	OnRenderScreen( int X, int Y, int W, int H, float fAlpha ) {}
	virtual void	OnUpdateScreen( float fDelta ) {}
	virtual bool	OnLeftMouseUp( void ) { return( false ); }
	virtual bool	OnLeftMouseDown( void ) { return( false ); }
	virtual void	OnCloseScreen( void ) { }
	virtual void	OnFreeScreen( void ) { }
	
	bool		IsScreenActive() { return( mbIsActive); }
	
	void		AddControlToScreen( UIControl* pControl );


	int			mnScreenButtonHover;

private:
	UIControl*		mpControlsList;

	bool		mbDrawWindowHold;
	int			mnDragWindowGrabX;
	int			mnDragWindowGrabY;
	int			mnDragWindowBasePosX;
	int			mnDragWindowBasePosY;
	int			mnManualPosX;
	int			mnManualPosY;

	int			mnWindowBarHover; 
	bool		mbIsActive;
};



#endif