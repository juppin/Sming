/*
 * ConfigFile.cpp
 *
 *  Created on: 27 Jun 2018
 *      Author: mikee47
 */

#include "JsonConfigFile.h"
#include "FileSystem.h"
#include "Data/Stream/FileStream.h"

#if DEBUG_VERBOSE_LEVEL == DBG
#include "Services/Profiling/ElapseTimer.h"
#endif

bool JsonConfigFile::load(const String& filename)
{
#if DEBUG_VERBOSE_LEVEL == DBG
	ElapseTimer elapse;
#endif

	content = fileGetContent(filename);
#if DEBUG_VERBOSE_LEVEL == DBG
	uint32_t tload = elapse.elapsed();
	elapse.start();
#endif
	// Passing a non-const buffer reference means JSON will reference existing data, not duplicate it
	rootObject = &buffer.parseObject(content.begin());
	if(rootObject->success()) {
#if DEBUG_VERBOSE_LEVEL == DBG
		debug_d("Loaded '%s', %u items, %u bytes, load %u us, parse %u us", filename.c_str(), rootObject->size(),
				buffer.size(), tload, elapse.elapsed());
#endif
		return true;
	}

	debug_e("Failed to load '%s'", filename.c_str());
	rootObject = &buffer.createObject();
	return false;
}

bool JsonConfigFile::save(const String& filename)
{
	FileStream f;
	if(!f.open(filename, eFO_CreateNewAlways)) {
		return false;
	}

	rootObject->printTo(f);
	int res = f.getLastError();
	f.close();

	if(res < 0) {
		char buf[32];
		debug_e("JsonConfig::save failed, %s", fileGetErrorText(res, buf, sizeof(buf)));
	}

	return res >= 0;
}
