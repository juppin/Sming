/*
 * ESP8266 LLMNR responder
 * Copyright (C) 2017 Stephen Warren <swarren@wwwdotorg.org>
 *
 * Based on:
 * ESP8266 Multicast DNS (port of CC3000 Multicast DNS library)
 * Version 1.1
 * Copyright (c) 2013 Tony DiCola (tony@tonydicola.com)
 * ESP8266 port (c) 2015 Ivan Grokhotkov (ivan@esp8266.com)
 * MDNS-SD Suport 2015 Hristo Gochkov
 * Extended MDNS-SD support 2016 Lars Englund (lars.englund@gmail.com)
 *
 * License (MIT license):
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Reference:
 * https://tools.ietf.org/html/rfc4795 (LLMNR)
 * https://tools.ietf.org/html/rfc1035 (DNS)
 */

// Define for extra debugging
#define LLMNR_DEBUG

#include "ESP8266LLMNR.h"
#include "MemoryStream.h"
#include <lwip/igmp.h>
#include "Platform/AccessPoint.h"
#include "Platform/Station.h"

//BIT(x) is defined in tools/sdk/c_types.h

#define FLAGS_QR BIT(15)
#define FLAGS_OP_SHIFT 11
#define FLAGS_OP_MASK 0x0f
// Conflict
#define FLAGS_C BIT(10)
// Truncation
#define FLAGS_TC BIT(9)
// Tentative
#define FLAGS_T BIT(8)
// Response code
#define FLAGS_RCODE_SHIFT 0
#define FLAGS_RCODE_MASK 0x0f

static const IPAddress LLMNR_MULTICAST_ADDR(224, 0, 0, 252);
static const int LLMNR_MULTICAST_TTL = 1;
static const uint16_t LLMNR_PORT = 5355;

bool LLMNRResponder::restart()
{
	UdpConnection::close();

	ip_addr_t multicast_addr = LLMNR_MULTICAST_ADDR;

	if(igmp_joingroup(IP_ADDR_ANY, &multicast_addr) != ERR_OK) {
		debug_w("LLMNR igmp_joingroup() failed");
		return false;
	}

	if(!UdpConnection::listen(LLMNR_PORT)) {
		debug_w("LLMNR listen() failed");
		return false;
	}

	debug_i("LLMNR restart() OK");
	return true;
}

static IPAddress _getRequestMulticastInterface(const IPAddress& remoteIP)
{
	if(WifiAccessPoint.isEnabled())
		if(WifiAccessPoint.getIP() == remoteIP)
			return remoteIP;

	return WifiStation.getIP();
}

void LLMNRResponder::onReceive(pbuf* buf, IPAddress remoteIP, uint16_t remotePort)
{
#ifdef LLMNR_DEBUG
	debug_i("LLMNR from %s:%u", remoteIP.toString().c_str(), remotePort);
#endif

	MemoryStream mem(buf->tot_len);
	if(mem.memptr() == NULL)
		return;
	pbuf_copy_partial(buf, mem.memptr(), buf->tot_len, 0);

#ifdef LLMNR_DEBUG
	debug_hex(DBG, "LLMNR", mem.memptr(), mem.size());
#endif

	uint16_t id = mem.read16();
	uint16_t flags = mem.read16();
	uint16_t QD_count = mem.read16();
	uint16_t AN_count = mem.read16();
	uint16_t NS_count = mem.read16();
	uint16_t AR_count = mem.read16();

#ifdef LLMNR_DEBUG
	debug_i("LLMNR ID: 0x%04x, Flags: 0x%04x, QD: %u, AN: %u, NS: %u, AR: %u", id, flags, QD_count, AN_count, NS_count,
			AR_count);
#endif

	const uint16_t badFlags = FLAGS_QR | (FLAGS_OP_MASK << FLAGS_OP_SHIFT) | FLAGS_C;
	if(flags & badFlags) {
#ifdef LLMNR_DEBUG
		debug_w("Bad flags");
#endif
		return;
	}

	if(QD_count != 1) {
#ifdef LLMNR_DEBUG
		debug_w("QD != 1");
#endif
		return;
	}

	if(AN_count || NS_count || AR_count) {
#ifdef LLMNR_DEBUG
		debug_w("AN/NS/AR count != 0");
#endif
		return;
	}

	uint8_t namelen = mem.read8();
#ifdef LLMNR_DEBUG
//  debug_i("QNAME len = %u", namelen);
#endif
	if(namelen != m_hostname.length()) {
#ifdef LLMNR_DEBUG
//    debug_w("QNAME len mismatch");
#endif
		return;
	}

	char qname[64];
	mem.readS(qname, namelen);
	mem.read8();
	qname[namelen] = '\0';
#ifdef LLMNR_DEBUG
	debug_i("QNAME = %s", qname);
#endif

	if(m_hostname != qname) {
#ifdef LLMNR_DEBUG
//    debug_w("QNAME mismatch");
#endif
		return;
	}

	uint16_t qtype = mem.read16();
	uint16_t qclass = mem.read16();

#ifdef LLMNR_DEBUG
	debug_i("QTYPE = %u, QCLASS = %u", qtype, qclass);
#endif

	bool have_rr = (qtype == 1)		 // A
				   && (qclass == 1); // IN

#ifdef LLMNR_DEBUG
	debug_i("Match: responding");
	if(!have_rr)
		debug_i("(no matching RRs)");
#endif

	uint32_t ip = _getRequestMulticastInterface(remoteIP);

	// Header
	mem.init(256);
	uint8_t header[] = {
		(uint8_t)(id >> 8),
		(uint8_t)(id & 0xff), // ID
		(uint8_t)(FLAGS_QR >> 8),
		0, // FLAGS
		0,
		1, // QDCOUNT
		0,
		!!have_rr, // ANCOUNT
		0,
		0, // NSCOUNT
		0,
		0, // ARCOUNT
	};
	mem.write(header, sizeof(header));
	// Question
	mem.writeS(qname);
	uint8_t q[] = {
		0,	// Name terminator
		0, 1, // TYPE (A)
		0, 1, // CLASS (IN)
	};
	mem.write(q, sizeof(q));
	// Answer, if we have one
	if(have_rr) {
		mem.writeS(qname);
		uint8_t rr[] = {
			0, // Name terminator
			0,
			1, // TYPE (A)
			0,
			1, // CLASS (IN)
			0,
			0,
			0,
			30, // TTL (30 seconds)
			0,
			4, // RDLENGTH
			uint8_t(ip),
			uint8_t(ip >> 8),
			uint8_t(ip >> 16),
			(uint8_t)(ip >> 24) // RDATA
		};
		mem.write(rr, sizeof(rr));
	}

	udp->multicast_ip = remoteIP;
	UdpConnection::sendTo(remoteIP, remotePort, (const char*)mem.memptr(), mem.tell());
}

bool LLMNRResponder::begin(const String& hostname)
{
	// Max length for a single label in DNS
	if(hostname.length() > 63) {
		debug_e("LLMNR hostname too long");
		return false;
	}

	m_hostname = hostname;
	m_hostname.toLowerCase();

	return restart();
}
