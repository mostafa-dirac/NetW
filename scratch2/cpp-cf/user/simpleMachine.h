//                   In the name of GOD
/**
 * Partov is a simulation engine, supporting emulation as well,
 * making it possible to create virtual networks.
 *  
 * Copyright © 2009-2015 Behnam Momeni.
 * 
 * This file is part of the Partov.
 * 
 * Partov is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Partov is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Partov.  If not, see <http://www.gnu.org/licenses/>.
 *  
 */

#ifndef _SI_M_H_
#define _SI_M_H_

#include "sm.h"
#include "../base/frame.h"
#include <vector>
#include <sstream>
#include <regex>
#include <netinet/in.h>
#include <netinet/ip.h> // for iphdr struct

#define SIZE_OF_HEADER sizeof(header)

struct ethernet_header {
	byte  dst[6];
	byte  src[6];
	uint16 type;
} __attribute__ ((packed));

struct ip_header {
	uint8_t version:4;
	uint8_t IHL:4;
	uint8_t DSCP:6;
	uint8_t ECN:2;
	uint16_t total_length;
	uint16_t identification;
	uint16_t flags_fragmentation_offset; //TODO: BIT Feild?
	uint8_t TTL;
	uint8_t protocol;
	uint16_t header_checksum;
	uint32_t src_ip;
	uint32_t dst_ip;
} __attribute__ ((packed));

struct udp_header{
	uint16_t src_port;
	uint16_t dst_port;
	uint16_t length;
	uint16_t checksum;
} __attribute__ ((packed));

struct data_id{
	uint8_t data_type:3;
	uint8_t id:5;
} __attribute__ ((packed));

struct header{
	ethernet_header ethernetHeader;
	ip_header ipHeader;
	udp_header udpHeader;
	data_id dataId;
} __attribute__ ((packed));

struct payload{
	char message[50];
} __attribute__ ((packed));

struct metadata{
	uint32_t local_ip;
	uint16_t local_port;
	uint32_t public_ip;
	uint16_t public_port;
} __attribute__ ((packed));

enum data_type {
	REQUEST_ASSIGNING_ID,
	RESPONSE_ASSIGNING_ID,
	DROP,
	REQUEST_GETTING_IP,
	RESPONSE_GETTING_IP,
	REQUEST_LOCAL_SESSION,
	RESPONSE_LOCAL_SESSION,
	REQUEST_PUBLIC_SESSION,
	RESPONSE_PUBLIC_SESSION,
	MESSAGE,
	NAT_UPDATED,
	REQUEST_UPDATING_INFO,
	STATUS,
	STATUS_RESPONSE,
	INVALID
};

class SimpleMachine{
private:
	const SimulatedMachine *simulatedMachine;

protected:
	Interface *iface;

public:
	SimpleMachine (const SimulatedMachine *simulatedMachine, Interface* iface);
	virtual ~SimpleMachine ();
	virtual void initialize () =0 ;
	virtual void run () = 0;
	virtual void processFrame (Frame frame, int ifaceIndex) = 0;
	int getCountOfInterfaces () const;

	void printInterfacesInformation () const;

	const std::string getCustomInformation ();

	bool sendFrame (Frame frame, int ifaceIndex);

	static std::vector<std::string> split(std::string str, char delimiter);
	static uint16_t get_checksum(const void *buf, size_t buf_len);      //TODO: really static?
	static uint8_t get_data_type(data_type type);
	static uint16_t get_data_length(data_type type);
	static void fill_header(header *packet_header,
	                 byte *mac,
	                 data_type type,
	                 int data_length,
	                 uint32 src_ip,
	                 uint32 dst_ip,
	                 uint16_t src_port,
	                 uint16_t dst_port,
	                 uint8_t ID);
	static void fill_ethernet(ethernet_header *packet_header, byte *MAC);
	static void fill_ip_header(ip_header *packet_ip_header, int data_length, uint32 src_ip, uint32 dst_ip);
	static void fill_udp_header(udp_header *udpHeader, int data_length, uint16_t src_port, uint16_t dst_port);
	static void fill_data_type_id(data_id *dataId, data_type type, uint8_t ID);
	static void fix_received_header_endianness(header *packet_header);
	static void fix_received_data_not_msg_endianness(metadata *metaData);
	static void fix_sending_header_endianness(header *packet_header);
	static void fix_sending_data_not_msg_endianness(metadata *metaData);
};

#endif /* sm.h */

