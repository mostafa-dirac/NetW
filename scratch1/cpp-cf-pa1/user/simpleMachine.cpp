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

void SimpleMachine::make_up_uint32(std::string IP, input_part *temp){
	std::vector<std::string> ip_bytes = split(IP, '.');
	temp->IP = static_cast<uint32>((std::stoi(ip_bytes[0]) << 24) |
		(std::stoi(ip_bytes[1]) << 16) |
		(std::stoi(ip_bytes[2]) << 8) |
		(std::stoi(ip_bytes[3])));
}

void SimpleMachine::ip_ntop(uint32 IP){
	auto b3 = static_cast<uint8_t>(IP & (uint32) 0xFF);
	auto b2 = static_cast<uint8_t>((IP >> 8) & (uint32) 0xFF);
	auto b1 = static_cast<uint8_t>((IP >> 16) & (uint32) 0xFF);
	auto b0 = static_cast<uint8_t>((IP >> 24) & (uint32) 0xFF);

	std::cout << b0 << "." << b1 << "." << b2 << "." << b3;
}