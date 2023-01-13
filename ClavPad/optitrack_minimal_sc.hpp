//
//  optitrack_minimal_sc.c
//  postwimp
//
//  Created by Francois Berard on 15/11/17.
//  Copyright (c)2017 LIG-IIHM. All rights reserved.
//
//	2021 01 25 FB
//		Added winsock initialization in "open_udp_multicast_socket", thx to Theo Recking

#ifndef OPTITRACK_MINIMAL_SC_HPP
#define OPTITRACK_MINIMAL_SC_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "OptitrackData.hpp"

const char		k_default_mcast_group[]	= "239.255.42.99";
const int		k_default_port		= 1511;


typedef enum {
	e_false		= 0,
	e_true		= 1
} bool_t;

extern int g_mcast_socket;
extern void* g_mcast_address;
extern bool_t	g_blocking;


// open_udp_multicast_socket --
//
//	<port>			you should probably pass <k_default_port>.
//	<mcastGroup>		you should probably pass <k_default_mcast_group>.
//	<blocking>		e_true: read will block for next packet,
//				e_false: read will not block even if no packet has been received.
//	<local_interface>	NULL: any interface on this computer, or the address of the local
//				interface to listen to, for example "169.254.108.200".

int open_udp_multicast_socket(int port, const char* mcastGroup, bool_t blocking, const char* local_interface);


// *********************************************************************
//
//  Unpack Data:
//      Recieves pointer to bytes that represent a packet of data
//
//      There are lots of print statements that show what
//      data is being stored
//
//      Most memcpy functions will assign the data to a variable.
//      Use this variable at your descretion. 
//      Variables created for storing data do not exceed the 
//      scope of this function. 
//
// *********************************************************************
void Unpack(glm::vec3 &origin, glm::quat &quaternion, std::vector<glm::vec3> &RigidBodyMarkers, std::vector<glm::vec3> &markers, char* pData);



// receive_packet --

void receive_packet(glm::vec3 &origin, glm::quat &quaternion, std::vector<glm::vec3> &RigidBodyMarkers, std::vector<glm::vec3> &markers, char* buffer, int buffer_size);


#endif
