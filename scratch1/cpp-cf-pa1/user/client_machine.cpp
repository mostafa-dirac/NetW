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
	// TODO: process the raw frame; frame.data points to the frame's byte stream
	/*cerr << "Frame received at iface " << ifaceIndex <<
	     " with length " << frame.length << endl;
	struct ethernet_header {
		byte  dst[6];
		byte  src[6];
		uint16 type;
	} __attribute__ ((packed));

	ethernet_header *eth = (ethernet_header *) frame.data;
	cerr << "Ethernet type field is 0x" << std::hex << ntohs (eth->type) << endl;*/
}


/**
 * This method will be run from an independent thread. Use it if needed or simply return.
 * Returning from this method will not finish the execution of the program.
 */
void ClientMachine::run () {
	// TODO: write your business logic here...
//	struct ethernet_header {
//		byte  dst[6];
//		byte  src[6];
//		uint16 type;
//	} __attribute__ ((packed));
//
//	struct ethernet_data {
//		byte  data_type;
//		byte  MAC[6];
//		uint32 IP;
//		int time;
//	} __attribute__ ((packed));
//
//	struct header {
//		ethernet_header header;
//		ethernet_data data;
//	} __attribute__ ((packed));

	/*const int frameLength = sizeof (ethernet_header) + 100;
	byte *data = new byte[frameLength];

	ethernet_header *eth = (ethernet_header *) data;
	memset (eth->dst, 255, 6); // broadcast address
	memcpy (eth->src, iface[0].mac, 6);
	eth->type = htons (0x0800);

	iphdr *packet = (iphdr *) (data + sizeof (ethernet_header));
	packet->version = 4;
	packet->ihl = 5;
	packet->tot_len = htons (100);

	Frame frame (frameLength, data);
	sendFrame (frame, 0); // sends frame on interface 0

	delete[] data;
	cerr << "now ./free.sh and check the pcap log file to see the sent packet" << endl; */
	for (;;) {
		auto input = new input_part;
		parse_input(input);

		switch (input->c_type){
			case GET_IP_client:
				t_dhcp_discover(input->time);
				break;
			case ACCEPT_OFFER_client:
				accept_dhcp_offer(input->IP, input->time);
				break;
			case RELEASE_client:
				release_ip(input->IP);
				break;
			case EXTEND_LEASE_client:
				extend_lease(input->IP, input->time);
				break;
			case PRINT_IP_client:
				handle_ip_list();
				break;
			default:
				cout << "invalid command" << endl;
		}
	}
}
void ClientMachine::parse_input(input_part *input)
{
	memset(input, 0, sizeof(input_part));

	std::string command;
	getline(std::cin, command);
	std::vector<std::string> command_tok = split(command, ' ');

	std::regex get_ip("get ip for time .*");
	std::regex accept_offer("accept offer: .*");
	std::regex release("release .*");
	std::regex extend_lease("extend lease .*");
	std::regex print_ip("print ip");

	if (std::regex_match(command, get_ip)){
		input->c_type = GET_IP_client;
		input->time = std::stoi(command_tok[4]);
	} else if (std::regex_match(command, accept_offer)){
		input->c_type = ACCEPT_OFFER_client;
		make_up_uint32(command_tok[2], input);
		input->time = std::stoi(command_tok[5]);
	} else if (std::regex_match(command, release)){
		input->c_type = RELEASE_client;
		make_up_uint32(command_tok[1], input);
	} else if (std::regex_match(command, extend_lease)){
		input->c_type = EXTEND_LEASE_client;
		make_up_uint32(command_tok[2], input);
		input->time = std::stoi(command_tok[5]);
	} else if (std::regex_match(command, print_ip)){
		input->c_type = PRINT_IP_client;
	}
}
void ClientMachine::t_dhcp_discover(int requested_time)
{
	int count = getCountOfInterfaces();
	for (int i = 0; i < count; i++) {
		auto frame_data = new byte[SIZE_OF_FRAME];
		auto ethz = (ethernet_frame *) frame_data;
		memset(ethz->header.dst, 255, 6);
		memcpy(ethz->header.src, iface[i].mac, 6);
		ethz->header.type = htons(0x0);
		ethz->data.data_type = DHCP_DISCOVER;
		memcpy(ethz->data.MAC, iface[0].mac, 6);
		ethz->data.IP = 0;
		ethz->data.time = requested_time;
		Frame frame ((uint32) SIZE_OF_FRAME, frame_data);
		sendFrame(frame, i);

		delete[] ethz;
	}
}
void ClientMachine::r_dhcp_offer(Frame frame, int iface_number)
{
	auto ethz = (ethernet_frame *) frame.data;

	if (memcmp(ethz->data.MAC, iface[0].mac, 6) == 0){
		offers.push_back(ethz);
		cout << "new offer: ";
		ip_ntop(ethz->data.IP);
		cout << " for time " << ethz->data.time << endl;
	}
}
void ClientMachine::accept_dhcp_offer(uint32 IP, int requested_time)
{
	for (auto itr = this->offers.begin(); itr < this->offers.end(); itr++){
		if ((*itr)->data.IP == IP){
			if ((*itr)->data.time >= requested_time)
				t_dhcp_request(IP, requested_time);
			else
				cout << "invalid offer" << endl;
			return;
		}
	}
}
void ClientMachine::t_dhcp_request(uint32 IP, int requested_time)
{
	int count = getCountOfInterfaces();
	for (int i = 0; i < count; i++){
		auto frame_data = new byte[SIZE_OF_FRAME];
		auto ethz = (ethernet_frame*)(frame_data);
		memset(ethz->header.dst, 255, 6);
		memcpy(ethz->header.src, iface[i].mac, 6);
		ethz->header.type = htons(0x0);
		ethz->data.data_type = DHCP_REQUEST;
		memcpy(ethz->data.MAC, iface[0].mac, 6); //TODO
		ethz->data.IP = IP;
		ethz->data.time = requested_time;
		Frame frame ((uint32) SIZE_OF_FRAME, frame_data);
		sendFrame(frame, i);
		delete[] frame_data;
	}
}
void ClientMachine::r_dhcp_ack(Frame frame, int iface_number)
{
	auto frame_data = (ethernet_frame*) frame.data;
	if (current_ip != nullptr)
		old_IPs.push_back(current_ip);
	current_ip = frame_data;

	cout << "now my ip is ";
	ip_ntop(frame_data->data.IP);
	cout << " for time " << frame_data->data.time << endl;

	int count = getCountOfInterfaces();
	for (int i = 0; i < count; i++){
		if (i != iface_number){
			sendFrame(frame, i);
		}
	}

}
void ClientMachine::r_dhcp_timeout(Frame frame, int iface_number)
{

}
void ClientMachine::release_ip(uint32)
{

}
void ClientMachine::t_dhcp_release(uint32)
{

}
void ClientMachine::extend_lease(uint32 IP, int requested_time)
{

}
void ClientMachine::t_dhcp_extend_request(uint32 IP, int requested_time)
{

}
void ClientMachine::handle_ip_list()
{
	std::sort(old_IPs.begin(), old_IPs.end(),& ClientMachine::factor_by_IP);

}
void ClientMachine::r_dhcp_extend_response(Frame frame, int iface_number)
{

}
void ClientMachine::nazri(Frame frame, int src_iface)
{
	int count = getCountOfInterfaces();
	for (int i = 0; i < count; i++){
		if (i != src_iface){
			sendFrame(frame, i);
		}
	}
}
bool ClientMachine::factor_by_IP(const ethernet_frame *f1, const ethernet_frame *f2)
{
	return f2->data.IP > f1->data.IP;
}

