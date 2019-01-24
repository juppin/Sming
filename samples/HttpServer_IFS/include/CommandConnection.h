/*
 * CommandConnection.h
 *
 *  Created on: 27 Jun 2018
 *      Author: mikee47
 *
 * Provides an abstraction for handling user commands, typically via websocket, but allows
 * use of other transports.
 *
 * Commands and responses use JSON.
 *
 * The command group specifies a minimum access level.
 */

#ifndef __COMMAND_CONNECTION_H
#define __COMMAND_CONNECTION_H

#include "Libraries/ArduinoJson/ArduinoJson.h"
#include "IPAddress.h"
#include "IFS/Access.h"

/*
 * Interface class for interacting either with user via websocket connection
 * or internal IOControls.
 */
class ICommandConnection
{
public:
	virtual ~ICommandConnection()
	{
	}

	// Check this connection is still active
	virtual bool active() = 0;

	//
	virtual void send(JsonObject& json)
	{
	}

	// Permitted access type
	virtual UserRole access()
	{
		return UserRole::None;
	}

	virtual void setAccess(UserRole access)
	{
	}

	virtual IPAddress getRemoteIp()
	{
		return IPAddress();
	}

	virtual uint32_t cid()
	{
		return 0;
	}
};

/*
 * We use a typedef to simplify implementing over a different type of connection.
 * Commands may be invoked without a connection so we must allow for connection = nullptr.
 */
typedef ICommandConnection* command_connection_t;

#endif // __CMDCONN_H
