/*
 * status.h
 *
 *  Created on: 28 May 2018
 *      Author: mikee47
 *
 *
 * In many places JSON request handlers need to report status back.
 * We provide a simple interface to deal with this.
 *
 */

#ifndef __STATUS_H
#define __STATUS_H

#include "WString.h"
#include "../Libraries/ArduinoJson/ArduinoJson.h"

DECLARE_FSTR(ATTR_CODE)
DECLARE_FSTR(ATTR_TEXT)

/*
 * Control status
 */
enum __attribute__((packed)) request_status_t {
	// Request queued
	status_pending,
	// Request completed
	status_success,
	// Request failed
	status_error
};

String statusToStr(request_status_t status);

void setStatus(JsonObject& json, request_status_t status);

static inline void setSuccess(JsonObject& json)
{
	setStatus(json, status_success);
}

static inline void setPending(JsonObject& json)
{
	setStatus(json, status_pending);
}

static inline void setError(JsonObject& json)
{
	setStatus(json, status_error);
}

/*
 *
 * @param json
 * @param code    Numeric error code
 * @param text    Text for error
 * @param arg     Argument or additional description
 *
 */
void setError(JsonObject& json, int code, const String& text = nullptr, const String& arg = nullptr);

#define IOERROR_MAP(XX)                                                                                                \
	XX(success, Success)                                                                                               \
	XX(bad_config, Configuration data invalid)                                                                         \
	XX(access_denied, Access Denied)                                                                                   \
	XX(file, File Error)                                                                                               \
	XX(timeout, Timeout)                                                                                               \
	XX(cancelled, Cancelled)                                                                                           \
	XX(not_impl, Not Implemented)                                                                                      \
  XX(bad_controller_class,  Wrong controller class specified for device)                                               \
	XX(bad_controller, Controller not registered)                                                                      \
	XX(bad_device_class, Device class not registered)                                                                  \
	XX(bad_device, Device not registered)                                                                              \
	XX(bad_node, Node ID not valid)                                                                                    \
	XX(bad_command, Invalid Command)                                                                                   \
	XX(bad_param, Invalid Parameter)                                                                                   \
	XX(busy, Device or controller is busy)                                                                             \
	XX(queue_full, Request queue is full)                                                                              \
	XX(nomem, Out of memory)                                                                                           \
	XX(no_control_id, Control ID not specified)                                                                        \
	XX(no_device_id, Device ID not specified)                                                                          \
	XX(no_command, Command not specified)                                                                              \
	XX(no_address, Device address not specified)                                                                       \
	XX(no_baudrate, Device baud rate not specified)                                                                    \
	XX(no_code, RF code not specified)

enum __attribute__((packed)) ioerror_t {
#define XX(_tag, _comment) ioe_##_tag,
	IOERROR_MAP(XX)
#undef XX
		ioe_MAX
};

String ioerrorString(ioerror_t err);

#define debug_err(_err, _arg) debug_w("%s: %s", ioerrorString(_err).c_str(), (_arg).c_str())

ioerror_t setError(JsonObject& json, ioerror_t err, const String& arg = nullptr);

#endif // __STATUS_H
