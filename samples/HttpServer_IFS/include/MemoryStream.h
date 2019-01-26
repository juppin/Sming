/*
 * memstream.h
 *
 *  Created on: 27 Apr 2018
 *      Author: mikee47
 */

#ifndef __MEMORY_STREAM_H
#define __MEMORY_STREAM_H

#include "c_types.h"

class MemoryStream
{
public:
	MemoryStream(size_t size) : m_data(nullptr), m_size(size), m_pos(0), m_owned(false)
	{
		init(size);
	}

	MemoryStream(uint8_t* data, size_t size) : m_data(data), m_size(size), m_pos(0), m_owned(false)
	{
	}

	~MemoryStream()
	{
		free();
	}

	void init(size_t size)
	{
		free();
		m_data = (uint8_t*)malloc(size);
		m_size = m_data ? size : 0;
		m_owned = true;
	}

	void free()
	{
		if(m_owned && m_data) {
			::free(m_data);
			m_owned = false;
			m_data = nullptr;
			m_size = 0;
			m_pos = 0;
		}
	}

	int read()
	{
		return (m_pos < m_size) ? m_data[m_pos++] : -1;
	}

	uint8_t read8()
	{
		return (uint8_t)read();
	}

	uint8_t readS(char* buf, uint8_t len)
	{
		if(m_pos + len > m_size)
			len = m_size - m_pos;
		memcpy(buf, &m_data[m_pos], len);
		m_pos += len;
		return len;
	}

	uint16_t read16()
	{
		return (read8() << 8) | read8();
	}

	uint32_t read32()
	{
		return (read16() << 16) | read16();
	}

	bool isValidOffset(size_t pos)
	{
		return pos < m_size;
	}

	size_t tell()
	{
		return m_pos;
	}

	size_t seek(size_t newPos)
	{
		if(newPos > m_size)
			m_pos = m_size;
		else
			m_pos = newPos;
		return m_pos;
	}

	uint8_t write(char c)
	{
		m_data[m_pos++] = c;
		return 1;
	}

	size_t write(const uint8_t* data, size_t len)
	{
		// @todo check for overflow, etc.
		memcpy(&m_data[m_pos], data, len);
		m_pos += len;
		return len;
	}

	size_t write_P(const uint8_t* data_P, size_t len)
	{
		memcpy_P(&m_data[m_pos], data_P, len);
		m_pos += len;
		return len;
	}

	size_t writeS(const char* str)
	{
		size_t len = strlen(str);
		m_data[m_pos++] = len;
		memcpy(&m_data[m_pos], str, len);
		m_pos += len;
		return 1 + len;
	}

	size_t writeS(const String& str)
	{
		return writeS(str.c_str());
	}

	size_t writeS_P(PGM_P str)
	{
		size_t len = strlen_P(str);
		m_data[m_pos++] = len;
		memcpy_P(&m_data[m_pos], str, len);
		return 1 + len;
	}

	uint8_t* memptr()
	{
		return m_data;
	}

	size_t size() const
	{
		return m_size;
	}

private:
	uint8_t* m_data;
	size_t m_size;
	size_t m_pos;
	bool m_owned;
};

#endif // __MEMORY_STREAM_H
