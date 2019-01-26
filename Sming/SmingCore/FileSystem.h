/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 *
 * August 2018
 * -----------
 *
 * Re-written to support pluggable file systems using the FileSystem class.
 *
 * Applications should continue to use the fileXXX functions definined in this
 * module. If no filing system is active they will fail gracefully with an
 * error code.
 *
 ****/

/**	@defgroup filesystem File system
 *	@brief	Access file system
 *  @{
 */

#ifndef _SMING_CORE_FILESYSTEM_H_
#define _SMING_CORE_FILESYSTEM_H_

#include "IFS/IFS.h"
#include "../Services/IFS/ifshelp.h"

// @deprecated
#include "../Services/SpifFS/spiffs_sming.h"

#include "WString.h"
#include "WVector.h" ///< @deprecated see fileList()

/** @brief Sets the currently active file system
 *  @param fileSystem
 *  @note Any existing filing system is freed first.
 *  Typically the filing system implementation has helper functions which
 *  create and initialise the file system to a valid state. The last step
 *  is to call this function to make it active.
 *	Call this function with nullptr to inactivate the filing system.
 */
void fileSetFileSystem(IFileSystem* fileSystem);

static inline void fileFreeFileSystem()
{
	fileSetFileSystem(nullptr);
}

/** @brief  Open file
 *  @param  name File name
 *  @param  flags Mode to open file
 *  @retval file File ID or negative error code
 */
file_t fileOpen(const char* name, FileOpenFlags flags = eFO_ReadOnly);

static inline file_t fileOpen(const String& name, FileOpenFlags flags = eFO_ReadOnly)
{
	return fileOpen(name.c_str(), flags);
}

file_t fileOpen(const FileStat& stat, FileOpenFlags flags = eFO_ReadOnly);

/** @brief  Clode file
 *  @param  file ID of file to open
 *  @note   File ID is returned from fileOpen function
 */
int fileClose(file_t file);

/** @brief  Write to file
 *  @param  file File ID
 *  @param  data Pointer to data to write to file
 *  @param  size Quantity of data elements to write to file
 *  @retval Quantity of data elements actually written to file or negative error code
 */
int fileWrite(file_t file, const void* data, size_t size);

/** @brief  Update file modification time
 *  @param  file File ID
 *  @retval error code
 */
static inline int fileTouch(file_t file)
{
	return fileWrite(file, nullptr, 0);
}

/** @brief  Read from file
 *  @param  file File ID
 *  @param  data Pointer to data buffer in to which to read data
 *  @param  size Quantity of data elements to read from file
 *  @retval Quantity of data elements actually read from file or negative error code
 */
int fileRead(file_t file, void* data, size_t size);

/** @brief  Position file cursor
 *  @param  file File ID
 *  @param  offset Quantity of bytes to move cursor
 *  @param  origin Position from where to move cursor
 *  @retval Offset within file or negative error code
 */
int fileSeek(file_t file, int offset, SeekOriginFlags origin);

/** @brief  Check if at end of file
 *  @param  file File ID
 *  @retval bool True if at end of file
 */
bool fileIsEOF(file_t file);

/** @brief  Get position in file
 *  @param  file File ID
 *  @retval int32_t Read / write cursor position
 */
int32_t fileTell(file_t file);

/** @brief  Flush pending writes
 *  @param  file File ID
 *  @retval int Size of last file written or negative error number
 */
int fileFlush(file_t file);

/** @brief get the text for a returned error code
 *  @param buffer to place text
 *  @param size bytes available in buffer
 *  @retval int length of text (excluding nul) or error code; indicates if error was recognised
 *  @note nul terminator will always be written to buffer, even on error unless
 *  buffer is null and/or size is 0.
 */
int fileGetErrorText(int err, char* buffer, size_t size);

String fileGetErrorString(int err);

/** @brief  Create or replace file with defined content
 *  @param  fileName Name of file to create or replace
 *  @param  content Pointer to c-string containing content to populate file with
 *  @retval int Number of bytes transferred or negative error code.
 *  @note   This function creates a new file or replaces an existing file and
            populates the file with the content of a c-string buffer.
            Remember to terminate your c-string buffer with a null (0).
 */
int fileSetContent(const char* fileName, const char* content, int length = -1);

static inline int fileSetContent(const String& fileName, const char* content)
{
	return fileSetContent(fileName.c_str(), content);
}

/** @brief  Create or replace file with defined content
 *  @param  fileName Name of file to create or replace
 *  @param  content String containing content to populate file with
 *  @retval int Number of bytes transferred or negative error code.
 *  @note   This function creates a new file or replaces an existing file and
            populates the file with the content of a string.
 */
static inline int fileSetContent(const String& fileName, const String& content)
{
	return fileSetContent(fileName.c_str(), content.c_str(), content.length());
}

/** @brief  Get size of file
 *  @param  fileName Name of file
 *  @retval uint32_t Size of file in bytes, 0 on error
 */
uint32_t fileGetSize(const char* fileName);

static inline uint32_t fileGetSize(const String& fileName)
{
	return fileGetSize(fileName.c_str());
}

/** @brief  Rename file
 *  @param  oldName Original name of file to rename
 *  @param  newName New name for file
 *  @retval error code
 */
int fileRename(const char* oldName, const char* newName);

static inline int fileRename(const String& oldName, const String& newName)
{
	return fileRename(oldName.c_str(), newName.c_str());
}

/** @brief  Get list of files on file system
 *  @retval Vector<String> Vector of strings.
            Each string element contains the name of a file on the file system

    @deprecated use fileOpenDir / fileReadDir / fileCloseDir
 */
Vector<String> fileList();

/** @brief  Read content of a file
 *  @param  fileName Name of file to read from
 *  @retval String String variable in to which to read the file content
 *  @note   After calling this function the content of the file is placed in to a string
 */
String fileGetContent(const String& fileName);

/** @brief  Read content of a file
 *  @param  fileName Name of file to read from
 *  @param  buffer Pointer to a character buffer in to which to read the file content
 *  @param  bufSize Quantity of bytes to read from file, plus 1 byte for trailing nul
 *  @retval int Quantity of bytes read from file or error code
 *  @note   After calling this function the content of the file is placed in to a c-string
 *  		Ensure there is sufficient space in the buffer for file content
 *  		plus extra trailing null, i.e. at least bufSize + 1
 *  		Always check the return value!
 */
int fileGetContent(const char* fileName, char* buffer, size_t bufSize);

static inline int fileGetContent(const String& fileName, char* buffer, size_t bufSize)
{
	return fileGetContent(fileName.c_str(), buffer, bufSize);
}

/** brief   Get file statistics
 *  @param  name File name
 *  @param  stat Pointer to SPIFFS statistic structure to populate
 *  @retval error code
 *  @note   Pass a pointer to an instantiated fileStats structure
 */
int fileStats(const char* fileName, FileStat* stat);

static inline int fileStats(const String& fileName, FileStat* stat)
{
	return fileStats(fileName.c_str(), stat);
}

/** brief   Get file statistics
 *  @param  file File ID
 *  @param  stat Pointer to SPIFFS statistic structure to populate
 *  @retval error code
 *  @note   Pass a pointer to an instantiated fileStats structure
 */
int fileStats(file_t file, FileStat* stat);

/** @brief  Delete file
 *  @param  name Name of file to delete
 *  @retval error code.
 */
int fileDelete(const char* fileName);

static inline int fileDelete(const String& fileName)
{
	return fileDelete(fileName.c_str());
}

/** @brief  Delete file
 *  @param  file handle of file to delete
 *  @retval error code.
 */
int fileDelete(file_t file);

/** @brief  Check if a file exists on file system
 *  @param  name Name of file to check for
 *  @retval bool true if file exists
 */
static inline bool fileExist(const char* fileName)
{
	return fileStats(fileName, nullptr) >= 0;
}

static inline bool fileExist(const String& fileName)
{
	return fileExist(fileName.c_str());
}

/** @brief Open a named directory for reading
 *  @param name Name of directory to open, empty or "/" for root
 *  @param dir  Directory object
 *  @retrval 0 on success or negative error
 *
 */
int fileOpenDir(const char* dirName, filedir_t* dir);

static inline int fileOpenDir(const String& dirName, filedir_t* dir)
{
	return fileOpenDir(dirName.c_str(), dir);
}

static inline int fileOpenRootDir(filedir_t* dir)
{
	return fileOpenDir(nullptr, dir);
}

/** @brief close a directory object
 *  @param dir directory to close
 *  @retval error code
 */
int fileCloseDir(filedir_t dir);

/** @brief Read a directory entry
 *  @param dir The directory object returned by fileOpenDir()
 *  @param stat The returned information, owned by filedir_t object
 */
int fileReadDir(filedir_t dir, FileStat* stat);

/** @brief Get basic file system information
 *  @retval int error code
 */
int fileGetSystemInfo(FileSystemInfo& info);

/** @brief Get the type of file system currently mounted (if any)
 *  @retval FileSystemType the file system type
 */
FileSystemType fileSystemType();

String fileSystemTypeStr();

/** @brief Format the active file system
 *  @retval int error code
 */
int fileSystemFormat();

/** @brief Perform a consistency check/repair on the active file system
 *  @retval int 0 if OK, < 0 unrecoverable errors, > 0 repairs required
 */
int fileSystemCheck();

/** @brief Truncate the file at the current cursor position
 *  @param file handle to open file
 *  @retval int new file size, or error code
 *  @note Changes the file size
 */
int fileTruncate(file_t file);

/** @brief  Set access control information
 *  @param  file File handle
 *  @retval int error code
 */
int fileSetACL(file_t file, FileACL* acl);

/** @brief Set file attributes
 *  @param file handle to open file, must have write access
 *  @retval int error code
 */
int fileSetAttr(file_t file, FileAttr attr);

/** @brief Set access control information for file
 *  @param file handle to open file, must have write access
 *  @retval int error code
 *  @note any writes to file will reset this to current time
 */
int fileSetTime(file_t file, time_t mtime);

/** @} */
#endif /* _SMING_CORE_FILESYSTEM_H_ */
