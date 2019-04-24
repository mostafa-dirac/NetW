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
	cerr << "Frame received at iface " << ifaceIndex <<
		" with length " << frame.length << endl;
  struct ethernet_header {
    byte  dst[6];
    byte  src[6];
    uint16 type;
  } __attribute__ ((packed));

  ethernet_header *eth = (ethernet_header *) frame.data;
  cerr << "Ethernet type field is 0x" << std::hex << ntohs (eth->type) << endl;
}


/**
 * This method will be run from an independent thread. Use it if needed or simply return.
 * Returning from this method will not finish the execution of the program.
 */
void ClientMachine::run () {
	// TODO: write your business logic here...
  struct ethernet_header {
    byte  dst[6];
    byte  src[6];
    uint16 type;
  } __attribute__ ((packed));

  const int frameLength = sizeof (ethernet_header) + 100;
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
  cerr << "now ./free.sh and check the pcap log file to see the sent packet" << endl;
}

