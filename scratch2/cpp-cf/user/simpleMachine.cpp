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
	// Cast the data pointer to one that can be indexed.
	char* data=(char*)buf;

	// Initialise the accumulator.
	uint32_t acc=0xffff;

	// Handle complete 16-bit blocks.
	for (size_t i=0;i+1<buf_len;i+=2) {
		uint16_t word;
		memcpy(&word,data+i,2);
		acc+=ntohs(word);
		if (acc>0xffff) {
			acc-=0xffff;
		}
	}

	// Handle any partial block at the end of the data.
	if (buf_len&1) {
		uint16_t word=0;
		memcpy(&word,data+buf_len-1,1);
		acc+=ntohs(word);
		if (acc>0xffff) {
			acc-=0xffff;
		}
	}

	// Return the checksum in network byte order.
	return htons(~acc);
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

void SimpleMachine::fill_header(header *packet_header,
                 byte *mac,
                 data_type type, int data_length,
                 uint32 src_ip, uint32 dst_ip,
                 uint16_t src_port, uint16_t dst_port,
                 uint8_t ID){

	fill_ethernet(&(packet_header->ethernetHeader),
	              mac);
	fill_ip_header(&(packet_header->ipHeader),
	               data_length, src_ip, dst_ip);
	fill_udp_header(&(packet_header->udpHeader),
	                data_length, src_port, dst_port);
	fill_data_type_id(&(packet_header->dataId),
	                  type, ID);
}

void SimpleMachine::fill_ethernet(ethernet_header *packet_header,
                   byte *MAC){
	packet_header->type = htons (0x0800);
	memset(packet_header->dst, 255, 6);
	memcpy(packet_header->src, MAC, 6);
}

void SimpleMachine::fill_ip_header(ip_header *packet_ip_header, int data_length, uint32 src_ip, uint32 dst_ip){
	packet_ip_header->version_IHL = 0x45;
	packet_ip_header->DSCP_ECN = 0;
	packet_ip_header->total_length = htons(sizeof(ip_header) + sizeof(udp_header) + sizeof(data_id) + data_length);
	packet_ip_header->identification = 0;
	packet_ip_header->flags_fragmentation_offset = 0;
	packet_ip_header->TTL = 64;
	packet_ip_header->protocol = 17;
	packet_ip_header->header_checksum = 0x0;
	packet_ip_header->src_ip = htonl(src_ip);
	packet_ip_header->dst_ip = htonl(dst_ip);
	packet_ip_header->header_checksum = (get_checksum(packet_ip_header, 20));
//	packet_ip_header->src_ip = htonl(src_ip);
//	packet_ip_header->dst_ip = htonl(dst_ip);
//	packet_ip_header->total_length = htons(sizeof(ip_header) + sizeof(udp_header) + sizeof(data_id) + data_length);
}

void SimpleMachine::fill_udp_header(udp_header *udpHeader,
                     int data_length, uint16_t src_port, uint16_t dst_port){
	udpHeader->src_port = htons(src_port);
	udpHeader->dst_port = htons(dst_port);
	udpHeader->length = htons(sizeof(udp_header) + sizeof(data_id) + data_length);
	udpHeader->checksum = 0x00;
}

void SimpleMachine::fill_data_type_id(data_id *dataId,
                       data_type type, uint8_t ID){
	dataId->data_type = get_data_type(type);
	dataId->id = ID;
}

void SimpleMachine::fix_received_header_endianness(header *packet_header){
	packet_header->ethernetHeader.type = ntohs(packet_header->ethernetHeader.type);

	packet_header->ipHeader.src_ip = ntohl(packet_header->ipHeader.src_ip);
	packet_header->ipHeader.dst_ip = ntohl(packet_header->ipHeader.dst_ip);
	packet_header->ipHeader.header_checksum = ntohs(packet_header->ipHeader.header_checksum);
	packet_header->ipHeader.total_length = ntohs(packet_header->ipHeader.total_length);
	packet_header->ipHeader.flags_fragmentation_offset = ntohs(packet_header->ipHeader.flags_fragmentation_offset);
	packet_header->ipHeader.identification = ntohs(packet_header->ipHeader.identification);

	packet_header->udpHeader.checksum = ntohs(packet_header->udpHeader.checksum);
	packet_header->udpHeader.src_port = ntohs(packet_header->udpHeader.src_port);
	packet_header->udpHeader.dst_port = ntohs(packet_header->udpHeader.dst_port);
	packet_header->udpHeader.length = ntohs(packet_header->udpHeader.length);
}

void SimpleMachine::fix_received_data_not_msg_endianness(metadata *metaData){
	metaData->local_port = ntohs(metaData->local_port);
	metaData->local_ip = ntohs(metaData->local_ip);
	metaData->public_port = ntohs(metaData->public_port);
	metaData->public_ip = ntohs(metaData->public_ip);
}

void SimpleMachine::fix_sending_header_endianness(header *packet_header){
	packet_header->ethernetHeader.type = htons(packet_header->ethernetHeader.type);

	packet_header->ipHeader.src_ip = htonl(packet_header->ipHeader.src_ip);
	packet_header->ipHeader.dst_ip = htonl(packet_header->ipHeader.dst_ip);
	packet_header->ipHeader.header_checksum = htons(packet_header->ipHeader.header_checksum);
	packet_header->ipHeader.total_length = htons(packet_header->ipHeader.total_length);
	packet_header->ipHeader.flags_fragmentation_offset = htons(packet_header->ipHeader.flags_fragmentation_offset);
	packet_header->ipHeader.identification = htons(packet_header->ipHeader.identification);

	packet_header->udpHeader.checksum = htons(packet_header->udpHeader.checksum);
	packet_header->udpHeader.src_port = htons(packet_header->udpHeader.src_port);
	packet_header->udpHeader.dst_port = htons(packet_header->udpHeader.dst_port);
	packet_header->udpHeader.length = htons(packet_header->udpHeader.length);
}

void SimpleMachine::fix_sending_data_not_msg_endianness(metadata *metaData){
	metaData->public_ip = htons(metaData->public_ip);
	metaData->public_port = htons(metaData->public_port);
	metaData->local_ip = htons(metaData->local_ip);
	metaData->local_port = htons(metaData->local_port);
}