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

#include "client_machine.h"

#include "interface.h"
#include "frame.h"

#include <netinet/in.h>
#include <netinet/ip.h> // for iphdr struct

using namespace std;

ClientMachine::ClientMachine (SimulatedMachine *simulatedMachine, Interface* iface) :
	SimpleMachine(simulatedMachine, iface) {
	// The machine instantiated.
	// Interfaces are not valid at this point.
}

ClientMachine::~ClientMachine () {
	// destructor...
}

void ClientMachine::initialize () {
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
void ClientMachine::processFrame (Frame frame, int ifaceIndex) {
//	cerr << "Frame received at iface " << ifaceIndex <<
//		" with length " << frame.length << endl;
//  struct ethernet_header {
//    byte  dst[6];
//    byte  src[6];
//    uint16 type;
//  } __attribute__ ((packed));
//
//  ethernet_header *eth = (ethernet_header *) frame.data;
//  cerr << "Ethernet type field is 0x" << std::hex << ntohs (eth->type) << endl;
	struct packet {
		header hdr;
		payload pl;
	} __attribute__ ((packed));

	auto ethz = (packet *) frame.data;

	fix_received_header_endianness(&(ethz->hdr));
	if (ethz->hdr.ipHeader.TTL > 0){
		//std::cout << "SHABLAND_CLIENT: no timeout" << std::endl;
		if ((ethz->hdr.ipHeader.protocol == 17) && (get_checksum(&(ethz->hdr.ipHeader), 20) == 0)){
			//std::cout << "SHABLAND_CLIENT: is valid" << std::endl;
			if (ethz->hdr.ipHeader.dst_ip == iface[0].getIp()){
				//std::cout << "SHABLAND_CLIENT: is mine" << std::endl;
				switch (detect_data_type(&(ethz->hdr), ethz->pl.message)){
					case RESPONSE_ASSIGNING_ID:
						receive_Response_assigning_ID_packet(frame, ifaceIndex);
						break;
					case DROP:
						receive_drop_packet(frame, ifaceIndex);
						break;
					case RESPONSE_GETTING_IP:
						receive_Response_getting_IP_packet(frame, ifaceIndex);
						break;
					case REQUEST_LOCAL_SESSION:
						receive_Request_session_packet(frame, ifaceIndex, REQUEST_LOCAL_SESSION);
						break;
					case RESPONSE_LOCAL_SESSION:
						receive_Response_session_packet(frame, ifaceIndex, RESPONSE_LOCAL_SESSION);
						break;
					case REQUEST_PUBLIC_SESSION:
						receive_Request_session_packet(frame, ifaceIndex, REQUEST_PUBLIC_SESSION);
						break;
					case RESPONSE_PUBLIC_SESSION:
						receive_Response_session_packet(frame, ifaceIndex, RESPONSE_PUBLIC_SESSION);
						break;
					case MESSAGE:
						receive_Message_packet(frame, ifaceIndex);
						break;
					case NAT_UPDATED:
						receive_NAT_updated_packet();
						break;
					case STATUS_RESPONSE:
						receive_Status_Response_packet(frame, ifaceIndex);
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
		} else {
			cout << "invalid packet, dropped" << endl;
		}
	}
}


/**
 * This method will be run from an independent thread. Use it if needed or simply return.
 * Returning from this method will not finish the execution of the program.
 */
void ClientMachine::run () {
//  struct ethernet_header {
//    byte  dst[6];
//    byte  src[6];
//    uint16 type;
//  } __attribute__ ((packed));
//
//  const int frameLength = sizeof (ethernet_header) + 100;
//  byte *data = new byte[frameLength];
//
//  ethernet_header *eth = (ethernet_header *) data;
//  memset (eth->dst, 255, 6); // broadcast address
//  memcpy (eth->src, iface[0].mac, 6);
//  eth->type = htons (0x0800);
//
//  iphdr *packet = (iphdr *) (data + sizeof (ethernet_header));
//  packet->version = 4;
//  packet->ihl = 5;
//  packet->tot_len = htons (100);
//
//  Frame frame (frameLength, data);
//  sendFrame (frame, 0); // sends frame on interface 0
//
//  delete[] data;
//  cerr << "now ./free.sh and check the pcap log file to see the sent packet" << endl;
	while (true) {
		auto input_info = new client_input;
		parse_input(input_info);

		switch (input_info->i_type) {
			case make_connection_client:
				make_connection(input_info->source_port);
				break;
			case get_id_info_client:
				get_id_info(input_info->ID);
				break;
			case make_local_session_client:
				make_session(input_info->ID, REQUEST_LOCAL_SESSION);
				break;
			case make_public_session_client:
				make_session(input_info->ID, REQUEST_PUBLIC_SESSION);
				break;
			case send_msg_client:
				send_message(input_info->ID, input_info->msg, input_info->msg_length);
				break;
			case status_client:
				ask_status();
				break;
			default:
				cout << "invalid command" << endl;
		}
	}
}

void ClientMachine::parse_input(client_input *input_info){
	memset(input_info, 0, sizeof(client_input));

	string command;
	getline(cin, command);
	vector<string> command_info = split(command, ' ');

	regex make_a_connection("make a connection to server on port .*");
	regex get_info("get info of .*");
	regex make_a_local_session("make a local session to .*");
	regex make_a_public_session("make a public session to .*");
	regex send_msg("send msg to .*");
	regex status("status");

	if (regex_match(command, make_a_connection)){
		input_info->i_type = make_connection_client;
		input_info->source_port = static_cast<uint16>(stoi(command_info[7]));
	} else if (regex_match(command, get_info)){
		input_info->i_type = get_id_info_client;
		input_info->ID = static_cast<uint16>(stoi(command_info[3]));
	} else if (regex_match(command, make_a_local_session)){
		input_info->i_type = make_local_session_client;
		input_info->ID = static_cast<uint16>(stoi(command_info[5]));
	} else if(regex_match(command, make_a_public_session)){
		input_info->i_type = make_public_session_client;
		input_info->ID = static_cast<uint16>(stoi(command_info[5]));
	} else if (regex_match(command, (regex) send_msg)){
		input_info->i_type = send_msg_client;
		input_info->msg = new char;
		vector<string> ID_msg = split(command_info[3], ':');
		input_info->ID = static_cast<uint16>(stoi(ID_msg[0]));
		unsigned long len = ID_msg[1].size();
		ID_msg[1].copy(input_info->msg, len);
		input_info->msg_length = static_cast<int>(len);
	} else if (regex_match(command, status)){
		input_info->i_type = status_client;
	}
}

data_type ClientMachine::detect_data_type(header *packet_header, char *dm) {
	switch (packet_header->dataId.data_type){
		case 0: {
			if (packet_header->dataId.id == 0)
				return DROP;
			return RESPONSE_ASSIGNING_ID;
		}
		case 1:
			return RESPONSE_GETTING_IP;
		case 2: {
			char ping[4] = {'p','i','n','g'};
			char pong[4] = {'p','o','n','g'};
			if (memcmp(dm, ping, 4) == 0){
				return REQUEST_LOCAL_SESSION;
			} else if (memcmp(dm, pong, 4) == 0){
				return RESPONSE_LOCAL_SESSION;
			}
			return INVALID;     //TODO: SHARRI?
		}
		case 3:
			return MESSAGE;
		case 4:
			return NAT_UPDATED;
		case 7:
			return STATUS_RESPONSE;
		default:
			return INVALID;     //TODO: SHARRI?
	}
}

void ClientMachine::make_connection(uint16_t PORT) {
	int data_length = get_data_length(REQUEST_ASSIGNING_ID);
//	int header_length = get_header_length(); //TODO
	const int packet_length = SIZE_OF_HEADER + data_length;

	byte *data = new byte[packet_length];
	struct packet {
		header hdr;
		metadata msg;
	} __attribute__ ((packed));
	auto whole_packet = (packet *)data;

	uint32_t dest_ip;
	memset(&dest_ip, 1, 4);

	fill_header(&(whole_packet->hdr),
	            iface[0].mac,
	            REQUEST_ASSIGNING_ID, data_length,
	            iface[0].getIp(), dest_ip,
	            PORT, 1234,
	            0);
	whole_packet->msg.local_ip = iface[0].getIp();
	whole_packet->msg.local_port = PORT;

	my_port = PORT;

	fix_sending_header_endianness(&(whole_packet->hdr));
	fix_sending_data_not_msg_endianness(&(whole_packet->msg));

	Frame frame ((uint32) packet_length, data);
	sendFrame (frame, 0);
	delete[] data;
}

void ClientMachine::receive_drop_packet(Frame frame, int ifaceIndex){
	auto ethz = (header *) frame.data;
	if (ethz->udpHeader.src_port == 1234){
		my_port += 100;
		make_connection(my_port);
		cout << "connection to server failed, retrying on port " << my_port << endl;
	} else if (ethz->udpHeader.src_port == 4321){
		if (connected && peer_info != nullptr){
			cout << "connection lost, perhaps" << peer_ID << "'s info has changed, ask server for updates" << endl;
		}
	}
}

void ClientMachine::receive_Response_assigning_ID_packet(Frame frame, int ifaceIndex){

	auto ethz = (header *) frame.data;

	my_ID = ethz->dataId.id;

	char *buf = new char[100];
	sprintf(buf, "Now My ID is %d", my_ID);
	auto output = string(buf);
	cout << output << endl;
	delete[] buf;
}

void ClientMachine::get_id_info(byte ID) {
	// we want to find the ip of this ID, therefor we will send a Request getting IP packet to the server
	int data_length = get_data_length(REQUEST_GETTING_IP);
//	int header_length = get_header_length();
	const int packet_length = SIZE_OF_HEADER + data_length;

	// initialize the byte stream
	byte *data = new byte[packet_length];

	// this packet has no data
	auto ethz = (header *)data;

	// SERVER: 1.1.1.1
	uint32_t dest_ip;
	memset(&dest_ip, 1, 4);

	// we will send it from interface 0, and with the querying ID
	fill_header(ethz,
	            iface[0].mac,
	            REQUEST_GETTING_IP, data_length,
	            iface[0].getIp(), dest_ip,
	            my_port, 1234,
	            ID);

	fix_sending_header_endianness(ethz);

	// send frame on interface 0
	Frame frame ((uint32) packet_length, data);
	sendFrame (frame, 0);
	delete[] data;
}

void ClientMachine::receive_Response_getting_IP_packet(Frame frame, int ifaceIndex) {
	struct packet {
		header hdr;
		metadata msg;
	} __attribute__ ((packed));
	auto ethz = (packet *)frame.data;

	fix_received_data_not_msg_endianness(&(ethz->msg));

	char *buf = new char[200];
	byte *local_ip = new byte[4];
	memcpy(local_ip, &(ethz->msg.local_ip), 4);
	byte *public_ip = new byte[4];
	memcpy(public_ip, &(ethz->msg.public_ip), 4);
	sprintf(buf, "packet with (%d, %d.%d.%d.%d, %d, %d.%d.%d.%d, %d) received",
	        ethz->hdr.dataId.id,
	        local_ip[3], local_ip[2], local_ip[1], local_ip[0],
	        ethz->msg.local_port,
	        public_ip[3], public_ip[2], public_ip[1], public_ip[0],
	        ethz->msg.public_port);
	auto output = string(buf);
	cout << output << endl;
	delete[] buf;

	if (peer_info == nullptr){
		peer_info = new metadata;
	}
	memcpy(peer_info, &(ethz->msg), 12);
	peer_ID = ethz->hdr.dataId.id;
	connected = false;
}

void ClientMachine::make_session(byte ID, data_type session_kind) {
	// information was not received yet. we do not know the IP.
	if (peer_info == nullptr || peer_ID != ID){
		char *buf = new char[50];
		sprintf(buf, "info of node %d was not received", ID);
		auto output = string(buf);
		cout << output << endl;
		delete[] buf;
		return;
	}

	// specify the session kind
	peer_session_kind = session_kind;

	int data_length = get_data_length(session_kind);
//	int header_length = get_header_length();
	const int packet_length = SIZE_OF_HEADER + data_length;

	byte *data = new byte[packet_length];
	struct packet {
		header hdr;
		payload pl;
	} __attribute__ ((packed));
	auto ethz = (packet *)data;

	int which_interface = (session_kind == REQUEST_LOCAL_SESSION) ?
	                      find_sending_interface(peer_info->local_ip) :
	                      find_sending_interface(peer_info->public_ip);
	uint32_t destination_ip = (session_kind == REQUEST_LOCAL_SESSION) ?
	                          peer_info->local_ip : peer_info->public_ip;
	uint16_t destination_port = (session_kind == REQUEST_LOCAL_SESSION) ?
	                            peer_info->local_port : peer_info->public_port;
	fill_header(&(ethz->hdr),
	            iface[which_interface].mac,
	            session_kind, data_length,
	            iface[0].getIp(), destination_ip,
	            my_port, destination_port,
	            my_ID);

	char ping[4] = {'p','i','n','g'};
	memcpy(ethz->pl.message, ping, 4);

	fix_sending_header_endianness(&(ethz->hdr));

	Frame frame ((uint32) packet_length, data);
	sendFrame (frame, which_interface);
	delete[] data;
}

void ClientMachine::receive_Request_session_packet(Frame frame, int ifaceIndex,
                                                   data_type session_kind) {
	int data_length = get_data_length(session_kind);
//	int header_length = get_header_length();
	const int packet_length = SIZE_OF_HEADER + data_length;

	struct packet {
		header hdr;
		payload pl;
	} __attribute__ ((packed));

	auto ethz = (packet *)frame.data;
	fix_received_header_endianness(&(ethz->hdr));

	char ping[4] = {'p','i','n','g'};
	char pong[4] = {'p','o','n','g'};
	if (memcmp(ping, ethz->pl.message, 4) == 0){
		if (!check_connection(ethz->hdr.dataId.id)){
			peer_ID = ethz->hdr.dataId.id;
			connected = true;
			peer_session_kind = session_kind;

			byte *data = new byte[packet_length];
			auto epfl = (packet *)data;
			data_type reply_session = (session_kind == REQUEST_LOCAL_SESSION) ?
			                          RESPONSE_LOCAL_SESSION : RESPONSE_PUBLIC_SESSION;
			fill_header(&(epfl->hdr),
			            iface[ifaceIndex].mac,
			            reply_session, get_data_length(reply_session),
			            ethz->hdr.ipHeader.dst_ip, ethz->hdr.ipHeader.src_ip,
			            ethz->hdr.udpHeader.dst_port, ethz->hdr.udpHeader.src_port,
			            my_ID);
			fix_sending_header_endianness(&(epfl->hdr));

			memcpy(ethz->pl.message, pong, 4);

			Frame new_frame ((uint32) packet_length, data);
			sendFrame (new_frame, ifaceIndex);
			delete[] data;
		}
	}
}

bool ClientMachine::check_connection(byte ID) {
	return peer_ID == ID && connected && peer_info != nullptr;
}

void ClientMachine::receive_Response_session_packet(Frame frame, int ifaceIndex, data_type session_kind) {
	struct packet {
		header hdr;
		payload pl;
	} __attribute__ ((packed));

	auto ethz = (packet *)frame.data;
	fix_received_header_endianness(&(ethz->hdr));

	char pong[4] = {'p','o','n','g'};
	// the message contains pong
	if (memcmp(pong, ethz->pl.message, 4) == 0){
		// we have tried to establish connection before
		if (ethz->hdr.dataId.id == peer_ID){
			// we have not connected yet (received pong before)
			if (!connected){
				// the correct session kind
				if (session_kind == peer_session_kind){
					// we are now connected
					connected = true;
					char *buf = new char[50];
					sprintf(buf, "Connected to %d", peer_ID);
					auto output = string(buf);
					cout << output << endl;
					delete[] buf;
					return;
				}
			}
		}
	}
}

void ClientMachine::send_message(byte ID, char *msg, int msg_length) {
	// check that we can send message
	if (check_connection(ID)){
		int data_length = msg_length;
//		int header_length = get_header_length();
		const int packet_length = SIZE_OF_HEADER + data_length;

		byte *data = new byte[packet_length];

		struct packet {
			header hdr;
			payload pl;
		} __attribute__ ((packed));

		auto ethz = (packet *)data;

		int which_interface = (peer_session_kind == REQUEST_LOCAL_SESSION ) ?
		                      find_sending_interface(peer_info->local_ip) :
		                      find_sending_interface(peer_info->public_ip);
		uint32_t destination_ip = (peer_session_kind == REQUEST_LOCAL_SESSION) ?
		                          peer_info->local_ip : peer_info->public_ip;
		uint16_t destination_port = (peer_session_kind == REQUEST_LOCAL_SESSION) ?
		                            peer_info->local_port : peer_info->public_port;
		fill_header(&(ethz->hdr),
		            iface[which_interface].mac,
		            MESSAGE, data_length,
		            iface[0].getIp(), destination_ip,
		            my_port, destination_port,
		            my_ID);
		memcpy(ethz->pl.message, msg, static_cast<size_t>(msg_length));

		fix_sending_header_endianness(&(ethz->hdr));

		Frame frame ((uint32) packet_length, data);
		sendFrame (frame, which_interface);
		delete[] data;
	} else {
		cout << "please make a session first" << endl;
	}
}

void ClientMachine::receive_Message_packet(Frame frame, int ifaceIndex) {
	struct packet {
		header hdr;
		payload pl;
	} __attribute__ ((packed));

	auto ethz = (packet *)frame.data;
	fix_received_header_endianness(&(ethz->hdr));

	if (check_connection(ethz->hdr.dataId.id)){
		char *buf = new char[50];
		sprintf(buf, "received message from %d:", ethz->hdr.dataId.id);
		auto output = string(buf);
		cout << output;
		cout << ethz->pl.message << endl;
		delete[] buf;
	}
}


void ClientMachine::receive_NAT_updated_packet() {
	int data_length = get_data_length(REQUEST_UPDATING_INFO);
//	int header_length = get_header_length();
	const int packet_length = SIZE_OF_HEADER + data_length;

	byte *data = new byte[packet_length];

	struct packet {
		header hdr;
		metadata md;
	} __attribute__ ((packed));

	auto ethz = (packet *)data;

	uint32_t dest_ip;
	memset(&dest_ip, 1, 4);
	fill_header(&(ethz->hdr),
	            iface[0].mac,
	            REQUEST_UPDATING_INFO, data_length,
	            iface[0].getIp(), dest_ip,
	            my_port, 1234,
	            my_ID);
	ethz->md.local_port = my_port;
	ethz->md.local_ip = iface[0].getIp();

	fix_sending_header_endianness(&(ethz->hdr));
	fix_sending_data_not_msg_endianness(&(ethz->md));

	Frame frame ((uint32) packet_length, data);
	sendFrame (frame, 0);
	delete[] data;
}

void ClientMachine::ask_status() {
	int data_length = get_data_length(STATUS);
//	int header_length = get_header_length();
	const int packet_length = SIZE_OF_HEADER + data_length;

	byte *data = new byte[packet_length];

	struct packet{
		header hdr;
		metadata md;
	} __attribute__ ((packed));

	auto ethz = (packet *)data;

	uint32_t dest_ip;
	memset(&dest_ip, 1, 4);
	fill_header(&(ethz->hdr),
	            iface[0].mac,
	            STATUS, data_length,
	            iface[0].getIp(), dest_ip,
	            my_port, 1234,
	            0);
	ethz->md.local_ip = iface[0].getIp();
	ethz->md.local_port = my_port;

	fix_sending_header_endianness(&(ethz->hdr));
	fix_sending_data_not_msg_endianness(&(ethz->md));

	Frame frame ((uint32) packet_length, data);
	sendFrame(frame, 0);
	delete[] data;
}

void ClientMachine::receive_Status_Response_packet(Frame frame, int ifaceIndex) {
	struct packet {
		header hdr;
		payload pl;
	} __attribute__ ((packed));

	auto ethz = (packet *)frame.data;

	fix_received_header_endianness(&(ethz->hdr));

	if (ethz->hdr.dataId.id == 0){
		std::cout << "indirect" << std::endl;
	} else if (ethz->hdr.dataId.id == 1){
		std::cout << "direct" << std::endl;
	}
}

int ClientMachine::find_sending_interface(uint32 dst_ip_hdr) {
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