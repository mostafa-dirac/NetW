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
};

struct header{
	ethernet_header ethernetHeader;
	ip_header ipHeader;
	udp_header udpHeader;
	data_id dataId;
};

struct data{
	char data[50];
};

struct metadata{
	uint32_t local_ip;
	uint16_t local_port;
	uint32_t public_ip;
	uint16_t public_port;
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

	std::vector<std::string> split(std::string str, char delimiter);
};

#endif /* sm.h */

