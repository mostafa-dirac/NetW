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

#ifndef _NAT_M_H_
#define _NAT_M_H_

#include "simpleMachine.h"
#include "sm.h"

struct Session{
	uint32_t local_ip;
	uint16_t local_port;
	uint32_t outer_ip;
	uint16_t outer_port;
};

struct Range{
	uint16_t begin;
	uint16_t end;
};

struct address{
	uint32_t ip;
	uint16_t port;
};

class NatMachine: public SimpleMachine {
public:

	std::vector<Range> blocked_range;
	std::vector<Session> sessions;
	std::vector<metadata*> table;

	uint16_t base_port;
	unsigned int counter;

	NatMachine (SimulatedMachine*, Interface* iface);
	virtual ~NatMachine ();

	virtual void initialize ();
	virtual void run ();
	virtual void processFrame (Frame frame, int ifaceIndex);
	address calculate_new_address();
	bool valid_in_range(uint16_t port);
};

#endif

