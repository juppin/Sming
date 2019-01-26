/*
 * ifshelp.h
 *
 *  Created on: 27 Jan 2019
 *      Author: Mike
 *
 * Helper functions to assist with standard filesystem creation
 */

#ifndef SERVICES_IFS_IFSHELP_H_
#define SERVICES_IFS_IFSHELP_H_

#include "IFS/IFS.h"

/** @brief Create a firmware filesystem
 *  @param fwfsImageData
 *  @retval IFileSystem* constructed filesystem object
 */
IFileSystem* CreateFirmwareFilesystem(const void* fwfsImageData);


/** @brief Create a hybrid filesystem
 *  @param fwfsImageData
 *  @retval IFileSystem* constructed filesystem object
 *  @note SPIFFS configuration is obtained via spiffs_get_storage_config()
 */
IFileSystem* CreateHybridFilesystem(const void* fwfsImageData);



#endif /* SERVICES_IFS_IFSHELP_H_ */
