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

#include <vector>
#include <sstream>
#include <regex>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sm.h"
#include "../base/frame.h"
#define SIZE_OF_FRAME sizeof(ethernet_frame)

enum dhcp_command_type{
	GET_IP_client,
	ACCEPT_OFFER_client,
	RELEASE_client,
	EXTEND_LEASE_client,
	PRINT_IP_client,
	ADD_POOL_server,
	ADD_TIME_server,
	PRINT_POOL_server
};

enum dhcp_data_type{
	DHCP_DISCOVER,
	DHCP_OFFER,
	DHCP_REQUEST,
	DHCP_ACK,
	DHCP_RELEASE,
	DHCP_TIMEOUT,
	DHCP_REQUEST_EXTEND,
	DHCP_RESPONSE_EXTEND
};

struct ethernet_header {
	byte  dst[6];
	byte  src[6];
	uint16 type;
} __attribute__ ((packed));

struct ethernet_data {
	byte  data_type;
	byte  MAC[6];
	uint32 IP;
	int time;
} __attribute__ ((packed));

struct ethernet_frame {
	ethernet_header header;
	ethernet_data data;
} __attribute__ ((packed));

struct input_part {
	dhcp_command_type c_type;
	int time;
	uint32 IP;
	int mask;
	int MAC;
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
	void make_up_uint32(std::string IP, input_part *temp);
	void ip_ntop(uint32 IP);

};

#endif /* sm.h */

