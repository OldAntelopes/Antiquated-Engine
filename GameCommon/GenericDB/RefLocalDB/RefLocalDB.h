#ifndef REF_LOCAL_DB_H
#define REF_LOCAL_DB_H


extern void		RefLocalDBInit( const char* szDBName );

extern int		RefLocalDBRetrieve( const char* szDBName, u64 ulRecordUID, BYTE** ppbMemOut, int* pnMemSize );

extern int		RefLocalDBStore( const char* szDBName, u64 ulRecordUID, BYTE* pbMem, int nMemSize );

extern void		RefLocalDBShutdown( void );

extern void		RefLocalDBResetAll( const char* szDBName );

#endif