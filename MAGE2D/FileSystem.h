/*
 * Author      : Matthew Johnson
 * Date        : 4/Jun/2013
 * Description :
 *   Common file system functions.
 */
 
#pragma once

#ifdef __cplusplus
extern "C" {
namespace mage
{
#endif

	static const int FILESYSTEM_MAX_FILES = 100;
	static const int FILESYSTEM_MAX_PATH = 256;

	enum FileSystemDataPriority
	{
		DATA_FILESYSTEM_FIRST,
		DATA_ARCHIVE_FIRST,
		DATA_ARCHIVE_ONLY,
		DATA_FILESYSTEM_ONLY
	};

	enum FileSystemError
	{
		FSE_NO_ERROR,
		FSE_FILE_NOT_FOUND,
		FSE_CORRUPT_FILE,
		FSE_INVALID_PASSWORD
	};

	void SetFileSystemPriority( FileSystemDataPriority priority );
	void SetPathToData( const char* dataPath, int archiveIndex );
	void AddPathToData( const char* dataPath );
	void SetArchiveSuffix( const char* suffix );
	void SetDataArchivePassword( const char* password, int archiveIndex );

	bool FileExists( const char* filename );
	int GetAllFilesByExtension( const char* directoryPath, const char* ext,
		std::vector< std::string >& filenames );

	void ListArchive( const char* archiveName );

	// fname is file name relative to root... example:
	// data/
	//   GameInfo.xml
	//   sprites/
	//     Player.entity
	//
	// OpenDataFile( sprites/Player.entity, ... )
	// will open the Player.entity file.
	int OpenDataFile( const char* fname, char*& _out_file, unsigned int& _out_len );


	// Generates some random ass files.
	void GenerateRandomAssFiles( const char* directoryPath, int fileCount, int fileSize );

#ifdef __cplusplus
}}
#endif