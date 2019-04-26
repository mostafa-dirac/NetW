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
	// TODO: Initialize your program here; interfaces are valid now.
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

	fix_received_header_endianness(&(ethz->hdr));

	if ((ethz->hdr.ipHeader.protocol == 17) && (get_checksum(&(ethz->hdr.ipHeader), 20) == 0)){
		//std::cout << "SHABLAND_SERVER: valid packet" << std::endl;
		if (ethz->hdr.ipHeader.dst_ip == iface[0].getIp()){
			//std::cout << "SHABLAND_SERVER: packet is mine" << std::endl;
			switch (detect_type(&(ethz->hdr))){
				case REQUEST_ASSIGNING_ID:
					receive_Request_assigning_ID(frame, ifaceIndex);
					break;
				case REQUEST_GETTING_IP:
					receive_Request_getting_IP(frame, ifaceIndex);
					break;
				case REQUEST_UPDATING_INFO:
					receive_Request_updating_info(frame, ifaceIndex);
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
			int i = find_sending_interface(ethz->hdr.ipHeader.dst_ip);

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

	fix_received_data_not_msg_endianness(ethz->md);

	if (find_client_from_local_ip(ethz->md.local_ip) != -1){
		std::cout << "you already have an id, ignored" << std::endl;
		return;
	}

	if (current_free_id <= 31){
		int data_length = get_data_length(RESPONSE_ASSIGNING_ID);
		int packet_length = SIZE_OF_HEADER + data_length;

		byte *data = new byte[packet_length];
		auto epfl = (packet *)data;

		uint32 server_ip;
		memset(&server_ip, 1, 4);
		// TODO: is the sending interface the same as the interface we received this packet from?
		int which_interface = find_sending_interface(ethz->md.local_ip);
		// TODO: should the destination be the local one in the data?
		fill_header(&(epfl->hdr),
		            iface[ifaceIndex].mac,
		            RESPONSE_ASSIGNING_ID, data_length,
		            server_ip, ethz->md.local_ip,
		            1234, ethz->md.local_port,
		            static_cast<uint8_t>(current_free_id));

		// save this client's information
		auto ci = new client_info;
		ci->ID = current_free_id;
		ci->addresses.local_port = ethz->md.local_port;
		ci->addresses.local_ip = ethz->md.local_ip;
		ci->addresses.public_ip = ethz->hdr.ipHeader.src_ip;
		ci->addresses.public_port = ethz->hdr.udpHeader.src_port;
		information.push_back(ci);

		byte *temp_ip = new byte;
		memcpy(temp_ip, &(ci->addresses.local_ip), 4);
		char *buf = new char[100];
		sprintf(buf, "new id %d assigned to %d.%d.%d.%d:%d",
		        current_free_id,
		        temp_ip[3], temp_ip[2], temp_ip[1], temp_ip[0],
		        ci->addresses.local_port);
		auto output = string(buf);
		cout << output << endl;
		delete[] buf;

		current_free_id++;

		fix_sending_header_endianness(&(epfl->hdr));

		Frame reply_frame ((uint32) packet_length, data);
		sendFrame (reply_frame, which_interface);
		delete[] data;
	}
}

void ServerMachine::receive_Request_getting_IP(Frame frame, int ifaceIndex) {
	auto ethz = (header *)frame.data;

	int idx_a = find_client_from_public_ip(ethz->ipHeader.src_ip);
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
//	int header_length = get_header_length();
	int packet_length = SIZE_OF_HEADER + data_length;

	byte *data = new byte[packet_length];
	struct packet {
		header hdr;
		metadata md;
	} __attribute__ ((packed));
	auto whole_packet = (packet *)data;

	uint32 server_ip;
	memset(&server_ip, 1, 4);
	fill_header(&(whole_packet->hdr),
	            iface[ifaceIndex].mac,
	            RESPONSE_GETTING_IP, data_length,
	            server_ip, ethz->ipHeader.src_ip,
	            1234, ethz->udpHeader.src_port,
	            ID_B);
	whole_packet->md.public_ip = information[ID_B]->addresses.public_ip;
	whole_packet->md.public_port = information[ID_B]->addresses.public_port;
	whole_packet->md.local_ip = information[ID_B]->addresses.local_ip;
	whole_packet->md.local_port = information[ID_A]->addresses.local_port;

	fix_sending_header_endianness(&(whole_packet->hdr));
	fix_sending_data_not_msg_endianness(whole_packet->md);

	Frame frame_reply ((uint32) packet_length, data);
	sendFrame (frame, ifaceIndex);
	delete[] data;
}

