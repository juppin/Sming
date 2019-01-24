/**
 * August 2018 mikee47
 *
 * Revised to work with IFS.
 *
 */

#ifndef SPIFFS_SMING_H_
#define SPIFFS_SMING_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "spiffs.h"

#define LOG_PAGE_SIZE       256

bool spiffs_mount();
bool spiffs_mount_manual(u32_t phys_addr, u32_t phys_size);
void spiffs_unmount();	///< @deprecated
bool spiffs_format(); 	///< @deprecated
spiffs_config spiffs_get_storage_config();

#if defined(__cplusplus)
}

#endif
#endif /* SPIFFS_SMING_H_ */

