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

#ifndef _SRV_M_H_
#define _SRV_M_H_

#include "simpleMachine.h"
#include "sm.h"

struct client_info{
	metadata addresses;
	byte ID:5;
};

class ServerMachine: public SimpleMachine {
public:
	std::vector<client_info*> information;
	byte current_free_id:5;

	ServerMachine (SimulatedMachine*, Interface* iface);
	virtual ~ServerMachine ();

	virtual void initialize ();
	virtual void run ();
	virtual void processFrame (Frame frame, int ifaceIndex);
	data_type detect_type(header *packet_header);
	void receive_Request_assigning_ID(Frame frame, int ifaceIndex);
	void receive_Request_getting_IP(Frame frame, int ifaceIndex);
	void receive_Request_updating_info(Frame frame, int ifaceIndex);
	void receive_status(Frame frame, int ifaceIndex);
	int find_client_from_public_ip(uint32 public_ip);
	int find_client_from_ID(byte ID);
	int find_client_from_local_ip(uint32 local_ip);
};

#endif

