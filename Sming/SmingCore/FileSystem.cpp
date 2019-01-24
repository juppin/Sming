/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "FileSystem.h"
#include "../Services/Profiling/ElapseTimer.h"

/** @brief Global file system instance
 *
 * This is declared static to prevent applications using it directly.
 *
 * Filing system implementations should use helper functions to setup and
 * initialise a filing system. If successful, call fileSetFileSystem() to make
 * it active. This ensures that any active filing system is dismounted destroyed
 * first.
 *
 */
static IFileSystem* _filesys;

void fileSetFileSystem(IFileSystem* fileSystem)
{
	if(_filesys)
		delete _filesys;
	_filesys = fileSystem;
}

/*
 * Boilerplate check for every file function to catch undefined filesystem.
 *
 * We don't use _method at the moment, but if we end up using a function table struct
 * for IFS then we'd also need to check for null values there.
 *
 * See IFS.h for a discussion of VMT vs. function tables.
 */
#define CHECK_FS(_method)                                                                                              \
	if(!_filesys)                                                                                                      \
		return (file_t)FSERR_NoFileSystem;

file_t fileOpen(const char* name, FileOpenFlags flags)
{
	CHECK_FS(open)
	return _filesys->open(name, flags);
}

file_t fileOpen(const FileStat& stat, FileOpenFlags flags)
{
	CHECK_FS(fopen)
	return _filesys->fopen(stat, flags);
}

int fileClose(file_t file)
{
	CHECK_FS(close)
	return _filesys->close(file);
}

int fileWrite(file_t file, const void* data, size_t size)
{
	CHECK_FS(write);
	ElapseTimer timer;
	auto result = _filesys->write(file, data, size);
	debug_i("fileWrite %u %u", size, timer.elapsed());
	return result;
}

int fileRead(file_t file, void* data, size_t size)
{
	CHECK_FS(read)
	ElapseTimer timer;
	auto result = _filesys->read(file, data, size);
	debug_i("fileRead %u %u", size, timer.elapsed());
	return result;
}

int fileSeek(file_t file, int offset, SeekOriginFlags origin)
{
	CHECK_FS(seek)
	return _filesys->lseek(file, offset, origin);
}

bool fileIsEOF(file_t file)
{
	return _filesys ? (_filesys->eof(file) != 0) : true;
}

int32_t fileTell(file_t file)
{
	CHECK_FS(tell)
	return _filesys->tell(file);
}

int fileFlush(file_t file)
{
	CHECK_FS(flush)
	return _filesys->flush(file);
}

int fileGetErrorText(int err, char* buffer, size_t size)
{
	CHECK_FS(geterrortext)
	return _filesys->geterrortext(err, buffer, size);
}

String fileGetErrorString(int err)
{
	String s;
	if(s.setLength(63)) {
		int len = fileGetErrorText(err, s.begin(), s.length() + 1);
		s.setLength(len > 0 ? len : 0);
	}
	return s;
}

int fileSetContent(const char* fileName, const char* content, int length)
{
	file_t file = fileOpen(fileName, eFO_CreateNewAlways | eFO_WriteOnly);
	if(file < 0)
		return file;

	fileWrite(file, content, length < 0 ? strlen(content) : length);
	fileClose(file);
	return FS_OK;
}

uint32_t fileGetSize(const char* fileName)
{
	file_t file = fileOpen(fileName);
	if(file < 0)
		return 0;
	// Get size
	fileSeek(file, 0, eSO_FileEnd);
	int size = fileTell(file);
	fileClose(file);
	return size > 0 ? size : 0;
}

int fileRename(const char* oldName, const char* newName)
{
	CHECK_FS(rename)
	return _filesys->rename(oldName, newName);
}

Vector<String> fileList()
{
	Vector<String> result;

	filedir_t dir;
	if(fileOpenDir(nullptr, &dir) == FS_OK) {
		FileNameStat stat;
		while(fileReadDir(dir, &stat) >= 0)
			result.add(stat.name.buffer);
		fileCloseDir(dir);
	}
	return result;
}

String fileGetContent(const String& fileName)
{
	file_t file = fileOpen(fileName.c_str());
	if(file < 0)
		return nullptr;

	// Get size
	fileSeek(file, 0, eSO_FileEnd);
	int size = fileTell(file);
	if(size <= 0) {
		fileClose(file);
		return nullptr;
	}
	fileSeek(file, 0, eSO_FileStart);
	String s;
	if(s.setLength(size))
		fileRead(file, s.begin(), size);
	fileClose(file);
	return s;
}

int fileGetContent(const char* fileName, char* buffer, size_t bufSize)
{
	if(!buffer || bufSize == 0)
		return FSERR_BadParam;
	*buffer = '\0';

	file_t file = fileOpen(fileName);
	if(file < 0)
		return file;

	// Get size
	fileSeek(file, 0, eSO_FileEnd);
	int res = fileTell(file);
	if(res > 0) {
		auto size = (size_t)res;
		if(size >= bufSize) {
			res = FSERR_BufferTooSmall;
			size = bufSize - 1;
		}
		buffer[size] = '\0';
		fileSeek(file, 0, eSO_FileStart);
		fileRead(file, buffer, size);
	}

	fileClose(file);

	return res;
}

int fileStats(const char* fileName, FileStat* stat)
{
	CHECK_FS(stat)
	return _filesys->stat(fileName, stat);
}

int fileStats(file_t file, FileStat* stat)
{
	CHECK_FS(fstat)
	return _filesys->fstat(file, stat);
}

int fileDelete(const char* fileName)
{
	CHECK_FS(remove)
	ElapseTimer timer;
	auto result = _filesys->remove(fileName);
	debug_i("fileRemove %u", timer.elapsed());
	return result;
}

int fileDelete(file_t file)
{
	CHECK_FS(fremove)
	return _filesys->fremove(file);
}

int fileOpenDir(const char* dirName, filedir_t* dir)
{
	CHECK_FS(opendir)
	return _filesys->opendir(dirName, dir);
}

int fileCloseDir(filedir_t dir)
{
	CHECK_FS(closedir)
	return _filesys->closedir(dir);
}

int fileReadDir(filedir_t dir, FileStat* stat)
{
	CHECK_FS(readdir)
	return _filesys->readdir(dir, stat);
}

int fileSetACL(file_t file, FileACL* acl)
{
	CHECK_FS(setacl)
	return _filesys->setacl(file, acl);
}

int fileSetAttr(file_t file, FileAttributes attr)
{
	CHECK_FS(setattr)
	return _filesys->setattr(file, attr);
}

int fileSetTime(file_t file, time_t mtime)
{
	CHECK_FS(settime)
	return _filesys->settime(file, mtime);
}

int fileGetSystemInfo(FileSystemInfo& info)
{
	CHECK_FS(getinfo)
	return _filesys->getinfo(info);
}

FileSystemType fileSystemType()
{
	if(!_filesys)
		return FileSystemType::Unknown;
	FileSystemInfo info;
	fileGetSystemInfo(info);
	return info.type;
}

String fileSystemTypeStr()
{
	String s;
	s.setLength(5);
	fileSystemTypeToStr(fileSystemType(), s.begin(), s.length());
	return s;
}

int fileSystemFormat()
{
	CHECK_FS(format)
	return _filesys->format();
}

int fileSystemCheck()
{
	CHECK_FS(check)
	return _filesys->check();
}
