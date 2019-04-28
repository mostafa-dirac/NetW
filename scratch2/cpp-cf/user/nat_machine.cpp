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

#include "nat_machine.h"

#include "interface.h"
#include "frame.h"

#include <netinet/in.h>
#include <netinet/ip.h> // for iphdr struct
using namespace std;

NatMachine::NatMachine (SimulatedMachine *simulatedMachine, Interface* iface) :
	SimpleMachine(simulatedMachine, iface) {
	// The machine instantiated.
	// Interfaces are not valid at this point.
}

NatMachine::~NatMachine () {
	// destructor...
}

void NatMachine::initialize () {
	base_port = 2000;
	counter = 0;
}

/**
 * This method is called from the main thread.
 * Also ownership of the data of the frame is not with you.
 * If you need it, make a copy for yourself.
 *
 * You can also send frames using:
 * <code>
 *     bool synchronized sendFrame (Frame frame, int ifaceIndex) const;
 * </code>
 * which accepts one frame and the interface index (counting from 0) and
 * sends the frame on that interface.
 * The Frame class used here, encapsulates any kind of network frame.
 * <code>
 *     class Frame {
 *     public:
 *       uint32 length;
 *       byte *data;
 *
 *       Frame (uint32 _length, byte *_data);
 *       virtual ~Frame ();
 *     };
 * </code>
 */
void NatMachine::processFrame (Frame frame, int ifaceIndex) {
	auto ethz = (header *) frame.data;
	int client_idx, ifidx;
	if (ifaceIndex == 0){
		client_idx = find_in_outer_table(ntohl(ethz->ipHeader.dst_ip), ntohs(ethz->udpHeader.dst_port));

		if (client_idx == -1){
			return;
		}

		int family_idx = find_in_session(ntohl(ethz->ipHeader.dst_ip), ntohs(ethz->udpHeader.dst_port), ntohl(ethz->ipHeader.src_ip), ntohs(ethz->udpHeader.src_port));

		if (family_idx == -1){
			cout << "outer packet dropped" << endl;
		}

		ifidx = find_sending_interface(table[client_idx]->local_ip);
		ethz->ipHeader.dst_ip = htonl(table[client_idx]->local_ip);
		ethz->udpHeader.dst_port = htons(table[client_idx]->local_port);
		sendFrame(frame, ifidx);
	}
	else{
		if (!valid_in_range(ntohs(ethz->udpHeader.src_port))){
			return;
		}
		client_idx = find_in_local_table(ntohl(ethz->ipHeader.src_ip), ntohs(ethz->udpHeader.src_port));
		if (client_idx == -1){
			address new_address = calculate_new_address();
			auto new_meta = new metadata;
			new_meta->public_ip = new_address.ip;
			new_meta->public_port = new_address.port;
			new_meta->local_port = ntohs(ethz->udpHeader.src_port);
			new_meta->local_ip = ntohl(ethz->ipHeader.src_ip);
			table.push_back(new_meta);
			client_idx = table.size() - 1;
		}
		auto ns = new Session;
		ns->local_ip = table[client_idx]->public_ip;
		ns->local_port = table[client_idx]->public_port;
		ns->outer_ip = ntohl(ethz->ipHeader.dst_ip);
		ns->outer_port = ntohs(ethz->udpHeader.dst_port);
		sessions.push_back(ns);

		ethz->ipHeader.src_ip = htonl(table[client_idx]->public_ip);
		ethz->udpHeader.src_port =htons(table[client_idx]->public_port);

		ifidx = find_sending_interface(ntohl(ethz->ipHeader.dst_ip));
		sendFrame(frame, ifidx);
	}
}


/**
 * This method will be run from an independent thread. Use it if needed or simply return.
 * Returning from this method will not finish the execution of the program.
 */
void NatMachine::run () {
	while (true) {
		auto input_info = new nat_input;
		parse_input(input_info);

		switch (input_info->i_type) {
			case BLOCK_RANGE:
				block_range(input_info->min, input_info->max);
				break;
			case RESET:
				reset_setting();
				break;
			default:
				cout << "invalid command" << endl;
		}
	}
}

void NatMachine::parse_input(nat_input *input_info){
	memset(input_info, 0, sizeof(nat_input));

	string command;
	getline(cin, command);
	vector<string> command_info = split(command, ' ');

	regex block_port_in_range("block port range .*");
	regex reset_setting("reset_setting network settings");

	if (regex_match(command, block_port_in_range)){
		input_info->i_type = BLOCK_RANGE;
		input_info->min = static_cast<uint16>(stoi(command_info[3]));
		input_info->max = static_cast<uint16>(stoi(command_info[4]));
	} else if (regex_match(command, reset_setting)){
		input_info->i_type = RESET;
	}
}

address NatMachine::calculate_new_address(){
	uint32_t new_ip = iface[0].getIp() + (unsigned int)(counter / 3) + 1;
	uint16_t new_port = base_port + (counter % 3) * 100;
	counter++;
	struct address new_address = {new_ip, new_port};
	return new_address;
};

bool NatMachine::valid_in_range(uint16_t port){
	for (auto & itr : blocked_range){ // blocked_range.begin() ; itr < blocked_range.end() ; itr++
		if (port <= itr->end && port >= itr->begin){
			return false;
		}
	}
	return true;
}

int NatMachine::find_in_local_table(uint32_t local_ip, uint16_t local_port){
	for (int i = 0 ; i < table.size() ; i++){
		if ((table[i]->local_ip == local_ip) && (table[i]->local_port == local_port)){
			return i;                                             //TODO: iterator?????????????????????????
		}
	}
	return -1;
}

int NatMachine::find_in_outer_table(uint32_t public_ip, uint16_t public_port){
	for (int i = 0 ; i < table.size() ; i++){
		if ((table[i]->public_ip == public_ip) && (table[i]->public_port == public_port)){
			return i;
		}
	}
	return -1;
}

int NatMachine::find_in_session(uint32_t local_ip, uint16_t local_port, uint32_t outer_ip, uint16_t outer_port){
	for (int i = 0; i < sessions.size(); ++i) {
		if (sessions[i]->local_ip == local_ip && sessions[i]->local_port == local_port &&
			sessions[i]->outer_ip == outer_ip && sessions[i]->outer_port == outer_port){
			return i;
		}
	}
	return -1;
}

void NatMachine::block_range(uint16_t min, uint16_t max){
	auto blk_range = new Range;
	blk_range->begin = min;
	blk_range->end = max;
	blocked_range.push_back(blk_range);
}

void NatMachine::reset_setting(){
	for (int i = 0; i < sessions.size() ; ++i) {

	}
}

int NatMachine::find_sending_interface(uint32 dst_ip_hdr) {
	int count = getCountOfInterfaces();
	for (int i = 0; i < count; ++i) {
		uint32 left = this->iface[i].getIp() & this->iface[i].getMask();
		uint32 right = dst_ip_hdr & this->iface[i].getMask();
		if (left == right){
			return i;
		}
	}
	return 0;
}