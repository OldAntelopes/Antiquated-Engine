//---------------------------------------------------------------
// SysGTK.c
//
//  This file is the GTK implementations of the (user interface related) functions defined in System.h
//
//  (Note : we could still be in windows but use the GTK toolkit... Sorry to break your nice organisation Mit...)
//---------------------------------------------------------------


#include <stdio.h>
#include <gtk/gtk.h>
#include <Compat.h>
#include "System.h"

#ifdef SERVER

#else
#include "Globals.h"
#endif


//-------------------------------------------------------
// Function : WidgetSetEnabled				GTK Implementation
//   Set a control enabled or disabled
//--------------------------------------------------------
void WidgetSetEnabled( void* pTargetTopParent, char* pcTarget, BOOL boNewState )
{
	gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(pTargetTopParent), pcTarget)), boNewState);
}

//-------------------------------------------------------
// Function : MenuEntrySetEnabled				GTK Implementation
//   Set a menu item enabled or disabled
//--------------------------------------------------------
// See macro in header file

//-------------------------------------------------------
// Function : SysMessageBox				GTK Implementation
//   Given an error msg, dialog box title and flags, this function 
//   should create a popup messagebox window.
//   Returns FALSE if user pressed "NO" or "Cancel", TRUE otherwise.
//--------------------------------------------------------
BOOL SysMessageBox( char* acError, char* acTitle, int nFlags )
{
BOOL boRes = FALSE;
GtkMessageType nMessType = (nFlags & SYSMESSBOX_EXCLAMATION) ? GTK_MESSAGE_WARNING : GTK_MESSAGE_QUESTION;
GtkButtonsType nButtons = (nFlags & SYSMESSBOX_OK) ? GTK_BUTTONS_OK_CANCEL : GTK_BUTTONS_CLOSE;
GtkWidget* pxDialog = gtk_message_dialog_new (	GTK_WINDOW(gpxMainGtkWindow),
												GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
												nMessType,
												nButtons,
												acError );
	
	gtk_window_set_title( GTK_WINDOW(pxDialog), acTitle );

	if ( (gtk_dialog_run( GTK_DIALOG(pxDialog) ) == GTK_RESPONSE_OK) && (nFlags & SYSMESSBOX_OK) )
		boRes = TRUE;
	
	gtk_widget_destroy(pxDialog);
	
	return boRes;
}

//-------------------------------------------------------
// Function : SysSetWindowTitle				GTK Implementation
//   This function change the title of a window
//--------------------------------------------------------
void SysSetWindowTitle( void* pxWindow, char* pcNewTitle )
{
	gtk_window_set_title( (GtkWindow*)pxWindow, pcNewTitle );
}

//-------------------------------------------------------
// Function : SysEmptyList				GTK Implementation
//   This function make a list control empty
//   pxParentWindow is the parent window, pListIdent the list widget name (string)
//--------------------------------------------------------
void SysEmptyList( void* pxParentWindow, void* pListIdent )
{
	gtk_clist_clear( (GtkCList*)lookup_widget(GTK_WIDGET(pxParentWindow), (char*)pListIdent) );
}

/*
//-------------------------------------------------------
// Function : SysFillListLine				GTK Implementation
//   This function fill up the line nLineNum of a list control with nValuesQtt strings
//   pxParentWindow is the parent window, pListIdent the list widget name (string)
//--------------------------------------------------------
extern void SysFillListLine( void* pxParentWindow, void* pListIdent, int nLineNum, int nValuesQtt, char** ppcValues )
{
int	nLoop;
	
	for (nLoop=0; nLoop<nValuesQtt; nLoop++)
	{
		gtk_clist_set_text( (GtkCList*)lookup_widget(GTK_WIDGET(pxParentWindow), (char*)pListIdent)), nLineNum, nLoop, ppcValues[nLoop] );
	}
}
*/

// Function : SysAddFilledListLine				GTK Implementation
//   This function fill up a new line of a list control with nValsQtt strings
//   pxParentWindow is the parent window, pListIdent the list widget name (string)
//--------------------------------------------------------
void SysAddFilledListLine( void* pxParentWindow, void* pListIdent, int nValsQtt, char** ppcValues, int nPos, int nParam )
{
	gtk_clist_append( (GtkCList*)lookup_widget(GTK_WIDGET(pxParentWindow), (char*)pListIdent), ppcValues );
}

//-------------------------------------------------------
// Function : SysGetSaveFilenameDialog				GTK Implementation
//   This function is used to allow the user to select a filename to save to from the folders on their puter.
//   szOutputFile where the filename will be stocked is expected to be an allocated 256 char string.
//   Returns True if all ok, False if error or user pressed cancel button.
//--------------------------------------------------------
BOOL SysGetSaveFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile )
{
GtkWidget* pxDialog = NULL;
GtkFileFilter* pxFilter = NULL;
char acString[128];
BOOL boRes = FALSE;
	
	SNPRINTF( acString, 127, "%s files", szFileFilter );

	pxFilter = gtk_file_filter_new();		
	gtk_file_filter_add_pattern( pxFilter, szFileFilter );
	gtk_file_filter_set_name( pxFilter, acString );
	
	pxDialog = gtk_file_chooser_dialog_new( szTitle,
											GTK_WINDOW(gpxMainWindow),
											GTK_FILE_CHOOSER_ACTION_SAVE,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
											NULL );
	//gtk_file_chooser_set_filename( GTK_FILE_CHOOSER(pxDialog), szOutputFile );	//Does nothing if file doesn't already exist - bad for "save as"
	//Todo : if (file exists) then select it for save else offer its name as default for creation
	gtk_file_chooser_set_current_name( GTK_FILE_CHOOSER(pxDialog), szOutputFile );
	gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER(pxDialog), TRUE );
	gtk_file_chooser_set_filter( GTK_FILE_CHOOSER(pxDialog), pxFilter);
	gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER(pxDialog), FALSE );

	if (gtk_dialog_run( GTK_DIALOG(pxDialog) ) == GTK_RESPONSE_ACCEPT)
	{
		SNPRINTF( szOutputFile, 255, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pxDialog)) );
		boRes = TRUE;
	}
	
	gtk_widget_destroy( pxDialog );
	return( boRes );
}

//-------------------------------------------------------
// Function : SysGetOpenFilenameDialog				GTK Implementation
//   This function is used to allow the user to select a file to open from the folders on their puter.
//   szOutputFile where the filename will be stocked is expected to be an allocated 256 char string.
//   Returns True if all ok, False if error or user pressed cancel button.
//--------------------------------------------------------
BOOL SysGetOpenFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile )
{
GtkWidget* pxDialog = NULL;
GtkFileFilter* pxFilter = NULL;
char acString[128];
BOOL boRes = FALSE;
	
	SNPRINTF( acString, 127, "%s files", szFileFilter );

	pxFilter = gtk_file_filter_new();		
	gtk_file_filter_add_pattern( pxFilter, szFileFilter );
	gtk_file_filter_set_name( pxFilter, acString );
	
	pxDialog = gtk_file_chooser_dialog_new( szTitle,
											GTK_WINDOW(gpxMainWindow),
											GTK_FILE_CHOOSER_ACTION_OPEN,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
											NULL );
	gtk_file_chooser_set_filename( GTK_FILE_CHOOSER(pxDialog), szOutputFile );
	gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER(pxDialog), TRUE );
	gtk_file_chooser_set_filter( GTK_FILE_CHOOSER(pxDialog), pxFilter);
	gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER(pxDialog), FALSE );

	if (gtk_dialog_run( GTK_DIALOG(pxDialog) ) == GTK_RESPONSE_ACCEPT)
	{
		SNPRINTF( szOutputFile, 255, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pxDialog)) );
		boRes = TRUE;
	}
	
	gtk_widget_destroy( pxDialog );
	return( boRes );
}
