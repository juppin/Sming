/*
 * JsonConfigFile.h
 *
 *  Created on: 26 May 2018
 *      Author: mikee47
 *
 * Simple wrapper around a JSON object with dynamic content buffer.
 */

#ifndef __JSON_CONFIG_FILE_H
#define __JSON_CONFIG_FILE_H

#include "WString.h"
#include "Libraries/ArduinoJson/include/ArduinoJson.h"

class JsonConfigFile
{
public:
	bool load(const String& filename);
	bool save(const String& filename);

	operator JsonObject&()
	{
		return *rootObject;
	}

	JsonObject& root()
	{
		return *rootObject;
	}

	__forceinline JsonObjectSubscript<const String&> operator[](const String& key)
	{
		return rootObject->operator[](key);
	}

protected:
	JsonObject& childObject(JsonObject& parent, String key)
	{
		return parent.containsKey(key) ? parent[key] : parent.createNestedObject(key);
	}

	String getString(JsonObject& obj, const String& key, const String& defaultValue)
	{
		return obj.containsKey(key) ? obj[key] : defaultValue;
	}

private:
	String content;		   ///< File content
	DynamicJsonBuffer buffer; ///< References m_content
	JsonObject* rootObject = nullptr;
};

// Read a value from CConfigFile or another JsonObject, but only if it exists
#define CONFIG_READ(json, key, value)                                                                                  \
	(__extension__({                                                                                                   \
		auto& obj = static_cast<const JsonObject&>(json);                                                              \
		bool exists = obj.containsKey(key);                                                                            \
		if(exists)                                                                                                     \
			value = obj[key];                                                                                          \
		exists;                                                                                                        \
	}))

#endif // __JSON_CONFIG_FILE_H
