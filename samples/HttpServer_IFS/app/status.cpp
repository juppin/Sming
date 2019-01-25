/*
 * status.cpp
 *
 *  Created on: 28 May 2018
 *      Author: mikee47
 */

#include "status.h"

static DEFINE_FSTR(ATTR_STATUS, "status");
static DEFINE_FSTR(STATUS_SUCCESS, "success");
static DEFINE_FSTR(STATUS_PENDING, "pending");
static DEFINE_FSTR(STATUS_ERROR, "error");
static DEFINE_FSTR(STATUS_UNKNOWN, "unknown");

// Json error node
static DEFINE_FSTR(ATTR_ERROR, "error") DEFINE_FSTR(ATTR_CODE, "code");
DEFINE_FSTR(ATTR_TEXT, "text") static DEFINE_FSTR(ATTR_ARG, "arg");

String statusToStr(request_status_t status)
{
	switch(status) {
	case status_success:
		return STATUS_SUCCESS;
	case status_pending:
		return STATUS_PENDING;
	case status_error:
		return STATUS_ERROR;
	default:
		return STATUS_UNKNOWN;
	}
}

void setStatus(JsonObject& json, request_status_t status)
{
	json[String(ATTR_STATUS)] = statusToStr(status);
}

void setError(JsonObject& json, int code, const String& text, const String& arg)
{
	setStatus(json, status_error);
	JsonObject& err = json.createNestedObject(String(ATTR_ERROR));
	err[ATTR_CODE] = code;
	if(text) {
		err[ATTR_TEXT] = text;
	}
	if(arg) {
		err[ATTR_ARG] = arg;
	}
}

#define XX(_tag, _value) static DEFINE_FSTR(statusstr_##_tag, #_tag);
IOERROR_MAP(XX)
#undef XX

static const FlashString* const ioerror_strings[] PROGMEM = {
#define XX(_tag, _value) FSTR_PTR(statusstr_##_tag),
	IOERROR_MAP(XX)
#undef XX
};

String ioerrorString(ioerror_t err)
{
	return (err < ioe_MAX) ? String(*ioerror_strings[err]) : String(err);
}

ioerror_t setError(JsonObject& json, ioerror_t err, const String& arg)
{
	setError(json, err, ioerrorString(err), arg);
	return err;
}
