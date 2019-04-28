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

#include "server_machine.h"

#include "interface.h"
#include "frame.h"

#include <netinet/in.h>
#include <netinet/ip.h> // for iphdr struct
using namespace std;

ServerMachine::ServerMachine (SimulatedMachine *simulatedMachine, Interface* iface) :
	SimpleMachine(simulatedMachine, iface) {
	// The machine instantiated.
	// Interfaces are not valid at this point.
}

ServerMachine::~ServerMachine () {
	// destructor...
}

void ServerMachine::initialize () {
	current_free_id = 0;
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
void ServerMachine::processFrame (Frame frame, int ifaceIndex) {
	struct packet {
		header hdr;
		payload pl;
	} __attribute__ ((packed));

	auto ethz = (packet *) frame.data;

	if ((ethz->hdr.ipHeader.protocol == 17) && (get_checksum(&(ethz->hdr.ipHeader), 20) == 0)){
		if (ethz->hdr.ipHeader.dst_ip == 0x01010101){//htonl(iface[1].getIp())
			switch (detect_type(&(ethz->hdr))){
				case REQUEST_ASSIGNING_ID:
					receive_Request_assigning_ID(frame, ifaceIndex);
					break;
				case REQUEST_GETTING_IP:
					receive_Request_getting_IP(frame, ifaceIndex);
					break;
				case REQUEST_UPDATING_INFO:
					receive_Request_updating_info(frame);
					break;
				case STATUS:
					receive_status(frame, ifaceIndex);
					break;
				default:
					break;
			}
		} else {
			/* packet is not for this node. must pass on. */

			// find the route
			int i = find_sending_interface(ntohl(ethz->hdr.ipHeader.dst_ip));

			// decrease the time and calculate the checksum again
			ethz->hdr.ipHeader.TTL--;
			ethz->hdr.ipHeader.header_checksum = 0;
			ethz->hdr.ipHeader.header_checksum = get_checksum(&(ethz->hdr.ipHeader), 20);

			sendFrame(frame, i);
		}
	}
}


/**
 * This method will be run from an independent thread. Use it if needed or simply return.
 * Returning from this method will not finish the execution of the program.
 */
void ServerMachine::run () {

}

data_type ServerMachine::detect_type(header *packet_header) {
	switch (packet_header->dataId.data_type){
		case 0:
			return REQUEST_ASSIGNING_ID;
		case 1:
			return REQUEST_GETTING_IP;
		case 5:
			return REQUEST_UPDATING_INFO;
		case 6:
			return STATUS;
		default:
			return INVALID;
	}
}

void ServerMachine::receive_Request_assigning_ID(Frame frame, int ifaceIndex) {
	struct packet {
		header hdr;
		metadata md;
	} __attribute__ ((packed));
	auto ethz = (packet *)frame.data;

	if (find_client_from_local_ip(ntohs(ethz->md.local_ip)) != -1){
//		std::cout << "you already have an id, ignored" << std::endl;
		return;
	}
	if (current_free_id == 0)
		current_free_id++;
	if (current_free_id <= 31){
		int data_length = get_data_length(RESPONSE_ASSIGNING_ID);
		int packet_length = SIZE_OF_HEADER + data_length;

		byte *data = new byte[packet_length];
		auto epfl = (packet *)data;

		uint32 server_ip = 0x01010101;
		int which_interface = ifaceIndex;

		fill_header(&(epfl->hdr),
		            iface[ifaceIndex].mac,
		            RESPONSE_ASSIGNING_ID, data_length,
		            server_ip, ntohl(ethz->hdr.ipHeader.src_ip),
		            1234, ntohs(ethz->hdr.udpHeader.src_port),
		            static_cast<uint8_t>(current_free_id));

		// save this client's information
		auto ci = new client_info;
		ci->ID = current_free_id;
		ci->addresses.local_port = ntohs(ethz->md.local_port);
		ci->addresses.local_ip = ntohl(ethz->md.local_ip);
		ci->addresses.public_ip = ntohl(ethz->hdr.ipHeader.src_ip);
		ci->addresses.public_port = ntohs(ethz->hdr.udpHeader.src_port);
		information.push_back(ci);

		byte *temp_ip = new byte;
		memcpy(temp_ip, &ci->addresses.public_ip, 4);      //TODO: public is correct.
		auto buf = new char[100];
		sprintf(buf, "new id %d assigned to %d.%d.%d.%d:%d",
		        current_free_id,
		        temp_ip[3], temp_ip[2], temp_ip[1], temp_ip[0],
		        ci->addresses.public_port);
		auto output = string(buf);
		cout << output << endl;
		delete[] buf;

		current_free_id++;

		Frame reply_frame ((uint32) packet_length, data);
		sendFrame (reply_frame, which_interface);
		delete[] data;
	}
}

void ServerMachine::receive_Request_getting_IP(Frame frame, int ifaceIndex) {
	auto ethz = (header *)frame.data;

	int idx_a = find_client_from_public_ip(ntohl(ethz->ipHeader.src_ip), ntohs(ethz->udpHeader.src_port));
	int idx_b = find_client_from_ID(ethz->dataId.id);
	if ((idx_a == -1) || (idx_b == -1)){
		cout << "id not exist, dropped" << endl;
		return;
	}

	byte ID_A = information[idx_a]->ID;
	byte ID_B = information[idx_b]->ID;

	char *buf = new char[100];
	sprintf(buf, "%d wants info of node %d", ID_A, ID_B);
	auto output = string(buf);
	cout << output << endl;
	delete[] buf;

	int data_length = get_data_length(RESPONSE_GETTING_IP);
	int packet_length = SIZE_OF_HEADER + data_length;

	byte *data = new byte[packet_length];
	struct packet {
		header hdr;
		metadata md;
	} __attribute__ ((packed));
	auto whole_packet = (packet *)data;

	uint32 server_ip = 0x01010101;
	fill_header(&(whole_packet->hdr),
	            iface[ifaceIndex].mac,
	            RESPONSE_GETTING_IP, data_length,
	            server_ip, ntohl(ethz->ipHeader.src_ip),
	            1234, ntohs(ethz->udpHeader.src_port),
	            ID_B);

	whole_packet->md.public_ip = htonl(information[idx_b]->addresses.public_ip);
	whole_packet->md.public_port = htons(information[idx_b]->addresses.public_port);
	whole_packet->md.local_ip = htonl(information[idx_b]->addresses.local_ip);
	whole_packet->md.local_port = htons(information[idx_b]->addresses.local_port);

	Frame frame_reply ((uint32) packet_length, data);
	sendFrame (frame_reply, ifaceIndex);
	delete[] data;
}

void ServerMachine::receive_Request_updating_info(Frame frame)
{
	struct packet {
		header hdr;
		metadata md;
	} __attribute__ ((packed));
	auto whole_packet = (packet *)frame.data;

	int idx = find_client_from_ID(whole_packet->hdr.dataId.id);
	if (idx != -1){
		information[idx]->addresses.local_ip = ntohl(whole_packet->md.local_ip);
		information[idx]->addresses.local_port = ntohs(whole_packet->md.local_port);
		byte *temp_ip = new byte;
		memcpy(temp_ip, &(information[idx]->addresses.local_ip), 4);

		char *buf = new char[100];
		sprintf(buf, "id %d infos updated to %d.%d.%d.%d:%d",
		        information[idx]->ID,
		        temp_ip[3], temp_ip[2], temp_ip[1], temp_ip[0],
		        information[idx]->addresses.local_port);
		auto output = string(buf);
		cout << output << endl;
		delete[] buf;
	}
}

void ServerMachine::receive_status(Frame frame, int ifaceIndex) {
	struct packet {
		header hdr;
		metadata md;
	} __attribute__ ((packed));
	auto ethz = (packet *)frame.data;

	byte flag = (byte)((ethz->md.local_ip == ethz->hdr.ipHeader.src_ip) && (ethz->md.local_port == ethz->hdr.udpHeader.src_port));

	int data_length = get_data_length(STATUS_RESPONSE);
	int packet_length = SIZE_OF_HEADER + data_length;

	byte *data = new byte[packet_length];
	auto epfl = (packet *)data;
	epfl->md.local_ip = 0x00;
	epfl->md.local_port = 0x0000;

	uint32 server_ip = 0x01010101;
	fill_header(&(epfl->hdr),
	            iface[ifaceIndex].mac,
	            STATUS_RESPONSE, data_length,
	            server_ip, ntohl(ethz->md.local_ip),
	            1234, ntohs(ethz->md.local_port),
	            flag);

	Frame frame_reply ((uint32) packet_length, data);
	sendFrame (frame_reply, ifaceIndex);
	delete[] data;
}

int ServerMachine::find_client_from_public_ip(uint32 public_ip, uint16_t public_port)
{
	for (int i = 0; i < information.size(); ++i) {
		if (information[i]->addresses.public_ip == public_ip && information[i]->addresses.public_port == public_port){
			return i;
		}
	}
//	for (auto & itr : information){
//		if(itr->addresses.public_ip == public_ip){      //TODO: return what?
//			return std::distance(*information.begin(), itr);
//			return itr - *information.begin();
//		}
//	}
	return -1;
}

int ServerMachine::find_client_from_ID(byte ID) {
	for (int i = 0; i < information.size(); ++i) {
		if (information[i]->ID == ID){
			return i;
		}
	}
	return -1;
}

int ServerMachine::find_client_from_local_ip(uint32 local_ip) {
	for (int i = 0; i < information.size(); ++i) {
		if (information[i]->addresses.local_ip == local_ip){
			return i;
		}
	}
	return -1;
}

int ServerMachine::find_sending_interface(uint32 dst_ip_hdr) {
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