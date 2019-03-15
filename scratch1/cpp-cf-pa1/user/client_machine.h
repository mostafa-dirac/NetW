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

#ifndef _CLI_M_H_
#define _CLI_M_H_

#include "simpleMachine.h"
#include "sm.h"

class ClientMachine: public SimpleMachine {
public:
	std::vector<ethernet_frame*> old_IPs;
	std::vector<ethernet_frame*> offers;

	ethernet_frame* current_ip;

	ClientMachine (SimulatedMachine*, Interface* iface);
	virtual ~ClientMachine ();

	virtual void initialize ();
	virtual void run ();
	virtual void processFrame (Frame frame, int ifaceIndex);

	void t_dhcp_discover (int requested_time);
	void r_dhcp_offer (Frame frame, int iface_number);
	void accept_dhcp_offer (uint32 IP, int requested_time);
	void t_dhcp_request (uint32 IP, int requested_time);
	void r_dhcp_ack (Frame frame, int iface_number);
	void release_ip (uint32);
	void t_dhcp_release (uint32);
	void r_dhcp_timeout (Frame frame, int iface_number);
	void extend_lease(uint32 IP, int requested_time);
	void t_dhcp_extend_request (uint32 IP, int requested_time);
	void r_dhcp_extend_response (Frame frame, int iface_number);
	void handle_ip_list ();
	void parse_input (input_part *input);
	void nazri (Frame frame, int src_iface);
	bool factor_by_IP(const ethernet_frame *f1, const ethernet_frame *h2);


};

#endif

