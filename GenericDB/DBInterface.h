#ifndef GENERIC_DB_INTERFACE_H
#define GENERIC_DB_INTERFACE_H


typedef	void(*DBResponse)( int nResponseCode, int nRequestID, u64 ullRecordUID, char* pcJSON, void* vpCBParam );


extern void		DBCreateInstance( int nDBID, const char* szDBName );

extern int		DBRetrieve( int nDBID, u64 ullRecordUID, DBResponse fnCallback, void* vpCBParam );

extern int		DBStore( int nDBID, u64 ullRecordUID, const char* pcJSON, DBResponse fnCallback, void* vpCBParam );

extern int		DBQueryKeyMatch( int nDBID, const char* szKey, const char* szValue, DBResponse fnCallback, void* vpCBParam );

extern void		DBShutdownInstance( int nDBID );

extern void		DBUpdateAll( void );

extern void		DBShutdownAll( void );

extern void		DBReset( int nDBID );

#endif