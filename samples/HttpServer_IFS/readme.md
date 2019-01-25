# HttpServer_IFS

Demonstration of IFS operation.

## Building

By default, builds using FWFS (Firmware Filesystem) which is read-only. To build for Hybrid Filesystem (SPIFFS layered over FWFS) open `app\FileManager.cpp` and uncomment the line `#define FWFS_HYBRID` then rebuild.

You may need to clean the SPIFFS using `make flashinit`.

## fsbuild

Every time make is run, the filesystem image is rebuilt. See `Makefile-user.mk`.
Python needs to be in path for the fsbuild script to work.

A filesystem image is created according to the `fsbuild.ini` file in the current directory. If you want to use a different .ini file, for example to build multiple images, specify it on the command line. For example:

	python "$(SMING_HOME)/third-party/IFS/fsbuild/fsbuild.py" fsbuild2.ini

The line `touch app/application.cpp` ensures that make recompiles the modified filesystem image.	

## WiFi
WiFi needs to be pre-configured for your network. If it's not already set on your device, edit `files/.network.json` and enter network details, then rename "station XX" to "station". The application will then configure using this information at startup.

## Using the application

Point your web browser to the device. On Windows you can enter `http://sming-ifs.local/` in the address bar (uses LLMNR). If you're on a system which supports MDNS then it may also work, not really tested though.

Enter `admin` for both `username` and `password` then hit connect. You should be rewarded with a file listing. Columns are, from left to right:

`Filename`
`Access`: Read/Write access. See  For example, `ga` means you only need guest access to read a file, but require admin to write it
`Flags`: See `third-party/IFS/src/IFS/FileAttributes.h` for a list
`Size`: Size of file in bytes
`Date`: last modified date

Available operations:

`Load`: Load the selected file into the edit window
`Check`: If the file is a .json file, it is parsed and reformatted. Otherwise does nothing.
`Save`: Writes edited file back to filesystem.
`Delete Files`: Delete selected files.
`Choose Files` and `Upload`: Upload new files to file system
`Info`: Show some filesystem information (volume size / free space)
`Check`: Perform a filesystem integrity check.
`Format`: Re-format the filesystem.

## Other pages

The application has three pages, selected from the top-left menu

`Files`: The above page
`Network`: Allows selection of a different WiFi network
`System`: Some system information


## Security / Authentication

The default configuration supports three logins:
`guest`: leave `username` and `password` blank, hit connect. You'll get an obscure error 'No handler for auth'. That's because the guest account is not permitted to load any of the above pages. The only 
`user`: enter `user` for both `username` and `password`. Whilst the page doesn't look any different to the admin login, attempting to load files such as `.auth.json` will fail.

All logins are stored in `.auth.json`.

Without a valid login, a minimal set of files may be accessed. These are the ones read access specified as '-' (any) in the file listing. For example, navigating to `http://sming-ifs.local/readme.md` will throw up a `forbidden` error page.

  
