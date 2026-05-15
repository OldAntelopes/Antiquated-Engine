#ifndef MILKDROP_EXTENSION_ASYNCFILEMGR_H
#define MILKDROP_EXTENSION_ASYNCFILEMGR_H


#include <string>
#include <map>

// Added this into the milkdrop implementation to more easily handle async file loading
// for presets (and etc) within the milkdrop plugin
// 
//  General process is.. before telling the milkdropPlugin to start playing, 
// trigger  the asyncfilemanager to preload everything
//   wait for it to load everything into memory
//  .. then tell the milkdropPlugin to start playing.
class milkdropExtensionAsyncFile
{
public:
	char		GetC();
	
	BYTE*		GetData() { return mpbFileInMem; }

private:
	BYTE*		mpbFileReader;
	BYTE*		mpbFileInMem = NULL;
	int			mFileSize = 0;

	FILE*		mpFileHandler = NULL;
};


class milkdropExtensionAsyncFileManager
{
public:
	void PreloadFilesForPreset(const char* szPresetFilename);


private:
	std::map<std::string, milkdropExtensionAsyncFile*>	mLoadedFilesMap;	

};


#endif