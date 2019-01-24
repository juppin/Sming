
#include "spiffs_sming.h"
#include "IFS/SPIFlashFileSystem.h"
#include "IFS/IFSFlashMedia.h"
#include "FileSystem.h"


/*******************
 The W25Q32BV array is organized into 16,384 programmable pages of 256-bytes each. Up to 256 bytes can be programmed at a time.
 Pages can be erased in groups of 16 (4KB sector erase), groups of 128 (32KB block erase), groups of 256 (64KB block erase) or
 the entire chip (chip erase). The W25Q32BV has 1,024 erasable sectors and 64 erasable blocks respectively.
 The small 4KB sectors allow for greater flexibility in applications that require data and parameter storage.
 ********************/

/*
 * This function is weakly defined so applications may override parameters
 * as required. This is done for rboot builds, for example.
 */
spiffs_config __attribute__((weak)) spiffs_get_storage_config()
{
	spiffs_config cfg = { 0 };
	cfg.phys_addr = (u32_t)flashmem_get_first_free_block_address();
	if (cfg.phys_addr == 0)
		return cfg;
	cfg.phys_addr &= 0xFFFFF000;  // get the start address of the sector
	cfg.phys_size = INTERNAL_FLASH_SIZE - ((u32_t)cfg.phys_addr);
	cfg.phys_erase_block = INTERNAL_FLASH_SECTOR_SIZE;  // according to datasheet
	cfg.log_block_size = INTERNAL_FLASH_SECTOR_SIZE * 2;  // Important to make large
	cfg.log_page_size = LOG_PAGE_SIZE;  // as we said

	return cfg;
}


bool spiffs_mount()
{
	auto cfg = spiffs_get_storage_config();
	return spiffs_mount_manual(cfg.phys_addr, cfg.phys_size);
}


bool spiffs_mount_manual(u32_t phys_addr, u32_t phys_size)
{
	if (phys_addr == 0 || phys_size == 0)
		return false;

	auto media = new IFSFlashMedia(phys_addr, phys_size, eFMA_ReadWrite);
	auto fs = new SPIFlashFileSystem(media);
	if (fs->mount() != FS_OK) {
		delete fs;
		return false;
	}

	fileSetFileSystem(fs);
	return true;
}


/** @brief unmount SPIFFS filesystem
 *  @deprecated use fileFreeFileSystem() instead
 *  @note this will do nothing if the active filesystem is not SPIFFS
 */
void spiffs_unmount()
{
	if (fileSystemType() == FileSystemType::SPIFFS)
		fileFreeFileSystem();
}


/** @brief format SPIFFS filesystem
 *  @deprecated use fileSystemFormat() instead
 *  @note this will fail if the active filesystem is not SPIFFS
 */
bool spiffs_format()
{
	if (fileSystemType() != FileSystemType::SPIFFS)
		return false;
	return fileSystemFormat() == FS_OK;
}

