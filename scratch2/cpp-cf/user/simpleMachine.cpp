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


#include "simpleMachine.h"

SimpleMachine::SimpleMachine (const SimulatedMachine *simulatedMachine, Interface* iface){
		this->simulatedMachine = simulatedMachine;
		this->iface = iface;
}


SimpleMachine::~SimpleMachine(){
}

int SimpleMachine::getCountOfInterfaces () const {
	return simulatedMachine->getCountOfInterfaces();
}

void SimpleMachine::printInterfacesInformation () const {
	simulatedMachine->printInterfacesInformation();
}

const std::string SimpleMachine::getCustomInformation () {
	return simulatedMachine->getCustomInformation();
}

bool SimpleMachine::sendFrame (Frame frame, int ifaceIndex){
	return simulatedMachine->sendFrame(frame, ifaceIndex);
}

std::vector<std::string> SimpleMachine::split(std::string str, char delimiter)
{
	std::vector<std::string> internal;
	std::stringstream ss(str); // Turn the string into a stream.
	std::string tok;

	while(getline(ss, tok, delimiter)) {
		internal.push_back(tok);
	}

	return internal;
}

uint16_t SimpleMachine::get_checksum(const void *buf, size_t buf_len){
	uint32_t sum = 0;
	uint32_t carry = 0;
	uint16_t result = 0;

	auto ip_temp = (const uint16_t *)(buf);

	while (buf_len > 1){
		sum += *ip_temp;
		ip_temp++;
		buf_len-=2;
	}

	while (sum > 0x0FFFF) {
		carry = (unsigned)(sum & (unsigned)0x0F0000) / 65536;
		sum = (uint32_t)(sum & (unsigned)0xFFFF) + carry;
	}

	sum = (unsigned)0x0FFFF - sum;
	result = (uint16_t) sum;
	return result;
}

uint8_t SimpleMachine::get_data_type(data_type type)
{
	switch (type){
		case REQUEST_ASSIGNING_ID:
			return 0;
		case RESPONSE_ASSIGNING_ID:
			return 0;
		case DROP:
			return 0;
		case REQUEST_GETTING_IP:
			return 1;
		case RESPONSE_GETTING_IP:
			return 1;
		case REQUEST_LOCAL_SESSION:
			return 2;
		case RESPONSE_LOCAL_SESSION:
			return 2;
		case REQUEST_PUBLIC_SESSION:
			return 2;
		case RESPONSE_PUBLIC_SESSION:
			return 2;
		case MESSAGE:
			return 3;
		case NAT_UPDATED:
			return 4;
		case REQUEST_UPDATING_INFO:
			return 5;
		case STATUS:
			return 6;
		case STATUS_RESPONSE:
			return 7;
		default:
			return -1;  //TODO: 255?
	}
}
uint16_t SimpleMachine::get_data_length(data_type type)
{
	switch (type){
		case REQUEST_ASSIGNING_ID:
			return 6;
		case RESPONSE_ASSIGNING_ID:
			return 0;
		case DROP:
			return 4;
		case REQUEST_GETTING_IP:
			return 0;
		case RESPONSE_GETTING_IP:
			return 12;
		case REQUEST_LOCAL_SESSION:
			return 4;
		case RESPONSE_LOCAL_SESSION:
			return 4;
		case REQUEST_PUBLIC_SESSION:
			return 4;
		case RESPONSE_PUBLIC_SESSION:
			return 4;
		case NAT_UPDATED:
			return 0;
		case REQUEST_UPDATING_INFO:
			return 6;
		case STATUS:
			return 6;
		case STATUS_RESPONSE:
			return 6;
		default:
			return -1; //TODO: 255?
	}
}

int SimpleMachine::find_sending_interface(uint32 dst_ip_hdr) {
	int count = getCountOfInterfaces();
	for (int i = 0; i < count; ++i) {
		uint32 left = iface[i].getIp() & iface[i].getMask();
		uint32 right = dst_ip_hdr & iface[i].getMask();
		if (left == right){
			return i;
		}
	}
	return 0;
}
