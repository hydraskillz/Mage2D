#include "MageLib.h"

// zlib and minizip
#include <zip.h>
#include <unzip.h>

#ifdef __cplusplus
using namespace mage;
extern "C"
{
#endif

static FileSystemDataPriority FilePrioryPreference;
static const int MAX_FILENAME = 256;
static const int MAX_ARCHIVES = 12;

char PathToData[ MAX_ARCHIVES ][ FILESYSTEM_MAX_PATH ] = { "../data" };
int CurrentNumerOfDataArchives = 1;
char ArchiveSuffix[ 8 ] = ".zip";
char DataPassword[ MAX_ARCHIVES ][ MAX_FILENAME  ];

//---------------------------------------
// Private helper functions
//---------------------------------------
const char* UNZ_ERROR_MAPPING( int UNZ_ERROR_CODE )
{
	switch ( UNZ_ERROR_CODE )
	{
	case UNZ_OK:				  return "OK";
	case UNZ_END_OF_LIST_OF_FILE: return "End of list of file";
	case UNZ_PARAMERROR:		  return "Parameter error";
	case UNZ_BADZIPFILE:		  return "Bad zip file";
	case UNZ_INTERNALERROR:		  return "Internal error";
	case UNZ_CRCERROR:			  return "CRC error";
	case Z_STREAM_END:			  return "File not found";
	case Z_ERRNO:				  return "Zlib error";
	case Z_STREAM_ERROR:		  return "Error reading file : [Stream]";
	case Z_DATA_ERROR:			  return "Error reading file : [Data]";
	case Z_MEM_ERROR:			  return "Error reading file : [Mem]";
	case Z_BUF_ERROR:			  return "Error reading file : [Buf]";
	case Z_VERSION_ERROR:		  return "Error reading file : [Version]";
	}
	return "Unknown Error (Probably File Not Found)";
}
//---------------------------------------
void Print64BitsSize( ZPOS64_T n, int size_char )
{
	char number[21];
	int offset = 19;
	int pos_string = 19;
	number[20] = 0;

	for ( ;; )
	{
		number[ offset ] = (char) ( ( n % 10 ) + '0' );
		if ( number[ offset ] != '0' )
		{
			pos_string = offset;
		}
		n /= 10;
		if ( offset == 0 )
		{
			break;
		}
		--offset;
	}
	int sizeDisplayString = 19 - pos_string;
	while ( size_char > sizeDisplayString )
	{
		--size_char;
		ConsolePrintf( " " );
	}
	ConsolePrintf( "%s", &number[ pos_string ] );
}
//---------------------------------------
char* StripPathFromFile( char* filepath )
{
	char* p = filepath;
	while ( *p )
	{
		if ( *p == '/' || *p == '\\' )
			filepath = p + 1;
		++p;
	}
	return filepath;
}
//---------------------------------------
// Search all archives for a file and load it
int _OpenFileArchive( const char* fname, char*& _out_file, unsigned int& _out_len )
{
	unzFile unzArchive;
	unz_file_info64 unzFileInfo;
	char filenameInArchive[ MAX_FILENAME ];
	char* filenameWithoutPath=0;
	char dataPath[ FILESYSTEM_MAX_PATH + 1 ];
	char* buffer;
	unsigned int bufferSize;
	int err;

	// Check each archive for file
	for ( int i = CurrentNumerOfDataArchives - 1; i >= 0; --i )
	{
		// Build archive name
		strncpy_s( dataPath, PathToData[ i ], FILESYSTEM_MAX_PATH - 1 );
		dataPath[ FILESYSTEM_MAX_PATH ] = '\0';
		strcat_s( dataPath, ArchiveSuffix );

		filenameWithoutPath = PathToData[ i ];
		filenameWithoutPath = StripPathFromFile( filenameWithoutPath );

		strcpy_s( filenameInArchive, filenameWithoutPath );
		strcat_s( filenameInArchive, "/" );
		strcat_s( filenameInArchive, fname );

		// Open archive
		unzArchive = unzOpen64( dataPath );
		if ( !unzArchive )
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning : Archive '%s' not found! Skipping...\n", dataPath );
			err = FSE_FILE_NOT_FOUND;
			continue;
		}

		// Try to find file in archive
		err = unzLocateFile( unzArchive, filenameInArchive, 0 );
		if ( err != UNZ_OK )
		{
			ConsolePrintf( CONSOLE_INFO, "File '%s' not found. Searching additional archives...\n", filenameInArchive );
			unzCloseCurrentFile( unzArchive );
			err =  FSE_FILE_NOT_FOUND;
			continue;
		}

		// Try to get the file info
		err = unzGetCurrentFileInfo64( unzArchive, &unzFileInfo, filenameInArchive, sizeof( filenameInArchive ), NULL, 0, NULL, 0 );
		if ( err != UNZ_OK )
		{
			ConsolePrintf( CONSOLE_INFO, "File '%s' not found. Searching additional archives...\n", filenameInArchive );
			unzCloseCurrentFile( unzArchive );
			err = FSE_CORRUPT_FILE;
			continue;
		}

		// Try to open the file
		// NOTE: password encryption doesn't seem to be enabled in the lib... why is that?
		err = *( DataPassword[ i ] ) ? unzOpenCurrentFilePassword( unzArchive , DataPassword[ i ]  )
							: unzOpenCurrentFile( unzArchive );
		if ( err != UNZ_OK )
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning: File '%s' found but password is invalid. Searching additional archives...\n", filenameInArchive );
			unzCloseCurrentFile( unzArchive );
			err = FSE_INVALID_PASSWORD;
			continue;
		}

		// Try to read the file
		bufferSize = (unsigned int) unzFileInfo.uncompressed_size;
		buffer = new char[ bufferSize ];
		err = unzReadCurrentFile( unzArchive, buffer, bufferSize );
		if ( err < 0 )
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning: File '%s' found but unable to read file. Searching additional archives...\n", filenameInArchive );
			unzCloseCurrentFile( unzArchive );
			delete[] buffer;
			err = FSE_CORRUPT_FILE;
			continue;
		}

		ConsolePrintf( CONSOLE_SUCCESS, "Loaded file '%s'\n", filenameInArchive );

		// Copy file data to out parameters
		_out_len = err == 0 ? bufferSize : err;
		_out_file = new char[ _out_len + 1 ];
		//strncpy( _out_file, (char*) buffer, _out_len );	// YO DAWG DONT DO THIS SHIT IT DONT WORK FOR BINARY DAWG USE MEMCPY DAWG
		memcpy( _out_file, buffer, _out_len );
		_out_file[ _out_len ] = 0;
		delete[] buffer;

		unzCloseCurrentFile( unzArchive );
		return FSE_NO_ERROR;
	}
	ConsolePrintf( CONSOLE_ERROR, "Error '%s' in OpenFile : %s\n", UNZ_ERROR_MAPPING( err ), filenameInArchive );
	return err;
}
//---------------------------------------
int _OpenFileFileSystem( const char* fname, char*& _out_file, unsigned int& _out_len )
{
	char dataPath[ FILESYSTEM_MAX_PATH + 1 ];
	char filenameInArchive[ MAX_FILENAME ];
	int err;
	FILE* fpFile=NULL;

	// Check each data path for file
	for ( int i = CurrentNumerOfDataArchives - 1; i >= 0; --i )
	{
		// Build archive name
		strncpy_s( dataPath, PathToData[ i ], FILESYSTEM_MAX_PATH - 1 );
		dataPath[ FILESYSTEM_MAX_PATH ] = '\0';
		strcat_s( dataPath, "/" );

		strcpy_s( filenameInArchive, dataPath );
		strcat_s( filenameInArchive, fname );

		// Try to open the file
		fopen_s( &fpFile, filenameInArchive, "rb" );
		if ( !fpFile )
		{
			ConsolePrintf( CONSOLE_INFO, "File '%s' not found. Searching additional files...\n", filenameInArchive );
			err = FSE_FILE_NOT_FOUND;
			continue;
		}

		// Get file size
		fseek( fpFile, 0, SEEK_END );
		_out_len = ftell( fpFile );
		rewind( fpFile );

		// Allocate memory for data
		_out_file = new char[ _out_len+1 ];
		_out_file[ _out_len ] = 0;

		// Copy file into memory
		err = fread( _out_file, 1, _out_len, fpFile );
		if ( (unsigned int) err != _out_len )
		{
			ConsolePrintf( CONSOLE_WARNING, "Warning : File '%s' failed to read.\n", filenameInArchive );
			err = FSE_CORRUPT_FILE;
		}
		else
		{
			err = FSE_NO_ERROR;
		}

		if ( err == FSE_NO_ERROR )
		{
			ConsolePrintf( CONSOLE_SUCCESS, "Loaded file '%s'\n", filenameInArchive );
		}
		else
		{
			ConsolePrintf( CONSOLE_ERROR, "Error: Failed to load file: '%s'\n", filenameInArchive );
		}

		fclose( fpFile );
		return err;
	}
	return err;
}
//---------------------------------------


//---------------------------------------
// General file functions
//---------------------------------------
void GenerateRandomAssFiles( const char* directoryPath, int fileCount, int fileSize )
{
	char* randomAssData = new char[ fileSize * 1000 ];
	for ( int panda = 0; panda < fileCount; ++panda )
	{
		char fname[64];
		sprintf_s( fname, "%s/generated_file_%d.dat", directoryPath, panda );
		FILE* fp = NULL;
		fopen_s( &fp, fname, "wb" );
		
		if ( fp )
		{
			for ( int l = 0; l < fileSize * 1000; ++l )
			{
				randomAssData[l] = (char) rand();
			}
			fwrite( randomAssData, sizeof( char ), fileSize * 1000, fp );
			fclose( fp );
		}
	}
	delete[] randomAssData;
}
//---------------------------------------
// Archive functions
//---------------------------------------
void SetFileSystemPriority( FileSystemDataPriority priority )
{
	FilePrioryPreference = priority;
}
//---------------------------------------
void SetPathToData( const char* dataPath, int archiveIndex )
{
	strncpy_s( PathToData[ archiveIndex ], dataPath, FILESYSTEM_MAX_PATH );
}
//---------------------------------------
void AddPathToData( const char* dataPath )
{
	strncpy_s( PathToData[ CurrentNumerOfDataArchives++ ], dataPath, FILESYSTEM_MAX_PATH );
}
//---------------------------------------
void SetArchiveSuffix( const char* suffix )
{
	strncpy_s( ArchiveSuffix, suffix, 8 );
}
//---------------------------------------
void SetDataArchivePassword( const char* password, int archiveIndex )
{
	strncpy_s( DataPassword[ archiveIndex] , password, MAX_FILENAME );
}
//---------------------------------------
void ListArchive( const char* archiveName )
{
	char archiveNameClipped[ MAX_FILENAME + 16 ];
	unzFile unzArchive = NULL;

	if ( archiveName )
	{
		strncpy_s( archiveNameClipped, archiveName, MAX_FILENAME - 1 );
		archiveNameClipped[ MAX_FILENAME ] = '\0';	// Buffer overrun protection

		unzArchive = unzOpen64( archiveName );

		if ( !unzArchive )
		{
			strcat_s( archiveNameClipped, ".zip" );
			unzArchive = unzOpen64( archiveNameClipped );
		}
	}

	if ( unzArchive )
	{
		unz_global_info64 gi;
		int err;


		err = unzGetGlobalInfo64( unzArchive, &gi );
		if ( err != UNZ_OK )
		{
			ConsolePrintf( CONSOLE_ERROR, "Error '%s' in ListArchive : unzGetGlobalInfo()\n", UNZ_ERROR_MAPPING( err ) );
		}

		ConsolePrintf( "  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n" );
		ConsolePrintf( "  ------  ------     ---- -----   ----    ----   ------     ----\n" );

		for ( ZPOS64_T i = 0; i < gi.number_entry; ++i )
		{
			char filename[ MAX_FILENAME ];
			unz_file_info64 fileInfo;
			uLong ratio = 0;
			const char* string_method;
			char cryptChar = ' ';

			err = unzGetCurrentFileInfo64( unzArchive, &fileInfo, filename, sizeof( filename ), NULL, 0, NULL, 0 );
			if ( err != UNZ_OK )
			{
				ConsolePrintf( CONSOLE_ERROR, "Error '%s' in ListArchive : unzGetCurrentFileInfo64()\n", UNZ_ERROR_MAPPING( err ) );
				break;
			}

			if ( fileInfo.uncompressed_size > 0 )
			{
				ratio = (uLong) ( (fileInfo.compressed_size * 100 ) / fileInfo.uncompressed_size );
			}

			if ( ( fileInfo.flag & 1 ) )
			{
				cryptChar = '*';
			}

			if ( fileInfo.compression_method == 0 )
			{
				string_method = "Stored";
			}
			else if ( fileInfo.compression_method == Z_DEFLATED )
			{
				uInt iLevel = (uInt) ( ( fileInfo.flag & 0x6 ) / 2 );

				if ( iLevel == 0 ) string_method = "Defl :N";
				else if ( iLevel == 1 ) string_method = "Defl :X";
				else if ( iLevel == 2 || iLevel == 3 ) string_method = "Defl :F";
			}
			else if ( fileInfo.compression_method == Z_BZIP2ED )
			{
				string_method = "BZip2 ";
			}
			else
			{
				string_method = "Unkn. ";
			}

			Print64BitsSize( fileInfo.uncompressed_size, 7 );
			ConsolePrintf( "  %6s%c", string_method, cryptChar );
			Print64BitsSize( fileInfo.compressed_size, 7 );
			ConsolePrintf( " %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
				ratio,
				(uLong) fileInfo.tmu_date.tm_mon + 1,
				(uLong) fileInfo.tmu_date.tm_mday,
				(uLong) fileInfo.tmu_date.tm_year % 100,
				(uLong) fileInfo.tmu_date.tm_hour, (uLong) fileInfo.tmu_date.tm_min,
				(uLong) fileInfo.crc, filename );

			if ( ( i + 1 ) < gi.number_entry )
			{
				err = unzGoToNextFile( unzArchive );
				if ( err != UNZ_OK )
				{
					ConsolePrintf( CONSOLE_ERROR, "Error '%s' in ListArchive : unzGoToNextFile()\n", UNZ_ERROR_MAPPING( err ) );
					break;
				}
			}
		}

		unzClose( unzArchive );
	}
	else
	{
		ConsolePrintf( CONSOLE_ERROR, "Cannot open file %s or %s.zip\n", archiveName, archiveNameClipped );
	}
}
//---------------------------------------
int OpenDataFile( const char* fname, char*& _out_file, unsigned int& _out_len )
{
	int err = FSE_NO_ERROR;
	switch ( FilePrioryPreference )
	{
		case DATA_ARCHIVE_ONLY:
		{
			err = _OpenFileArchive( fname, _out_file, _out_len );
			break;
		}
		case DATA_FILESYSTEM_ONLY:
		{
			err = _OpenFileFileSystem( fname, _out_file, _out_len );
			break;
		}
		case DATA_ARCHIVE_FIRST:
		{
			err = _OpenFileArchive( fname, _out_file, _out_len );
			if ( err != FSE_NO_ERROR )
			{
				err = _OpenFileFileSystem( fname, _out_file, _out_len );
			}
			break;
		}
		case  DATA_FILESYSTEM_FIRST:
		{
			err = _OpenFileFileSystem( fname, _out_file, _out_len );
			if ( err != FSE_NO_ERROR )
			{
				err = _OpenFileArchive( fname, _out_file, _out_len );
			}
			break;
		}
	}

	return err;
}
//---------------------------------------

#ifdef __cplusplus
}
#endif