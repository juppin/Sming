/*
 * ifshelp.cpp
 *
 *  Created on: 27 Jan 2019
 *      Author: Mike
 */

#include "ifshelp.h"
#include "IFS/IFSFlashMedia.h"
#include "IFS/FWObjectStore.h"
#include "IFS/HybridFileSystem.h"
#include "../Services/SpifFS/spiffs_sming.h"

IFileSystem* CreateFirmwareFilesystem(const void* fwfsImageData)
{
	auto fwMedia = new IFSFlashMedia(fwfsImageData, eFMA_ReadOnly);
	if(fwMedia == nullptr) {
		return nullptr;
	}

	auto store = new FWObjectStore(fwMedia);
	if(store == nullptr) {
		delete fwMedia;
		return nullptr;
	}

	auto fs = new FirmwareFileSystem(store);
	if(fs == nullptr) {
		delete store;
	}

	return fs;
}

IFileSystem* CreateHybridFilesystem(const void* fwfsImageData)
{
	auto fwMedia = new IFSFlashMedia(fwfsImageData, eFMA_ReadOnly);
	if(fwMedia == nullptr) {
		return nullptr;
	}

	auto store = new FWObjectStore(fwMedia);
	if(store == nullptr) {
		delete fwMedia;
		return nullptr;
	}

	auto cfg = spiffs_get_storage_config();
	auto ffsMedia = new IFSFlashMedia(cfg.phys_addr, cfg.phys_size, eFMA_ReadWrite);

	auto fs = new HybridFileSystem(store, ffsMedia);

	if(fs == nullptr) {
		delete ffsMedia;
		delete store;
	}

	return fs;
}
