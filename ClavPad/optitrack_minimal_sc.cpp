#include "optitrack_minimal_sc.hpp"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#else
#include <winsock2.h>
#include <tchar.h>
#include <conio.h>
#include <ws2tcpip.h>
#endif

#include <string>

int g_mcast_socket = -1;
void* g_mcast_address = NULL;
bool_t g_blocking = e_false;
// open_udp_multicast_socket --
//
//	<port>			you should probably pass <k_default_port>.
//	<mcastGroup>		you should probably pass <k_default_mcast_group>.
//	<blocking>		e_true: read will block for next packet,
//				e_false: read will not block even if no packet has been received.
//	<local_interface>	NULL: any interface on this computer, or the address of the local
//				interface to listen to, for example "169.254.108.200".

int open_udp_multicast_socket(int port, const char* mcastGroup, bool_t blocking, const char* local_interface)
{
	struct sockaddr_in*	si_me;
	struct ip_mreq		mreq;
	int			res;

#ifdef _WIN32
	static int		winsock_initialized = 0;

	if (!winsock_initialized) {
		WORD		wVersionRequested;
		WSADATA		wsaData;
		int		err;

		/* Use the MAKEWORD(lowbyte,highbyte) macro declared in Windef.h */
		wVersionRequested = MAKEWORD(2,2);

		err = WSAStartup(wVersionRequested, &wsaData);
		if(err != 0) {
			/* Tell the user that we could not find a usable */
			/* Winsock DLL                                   */
			printf("WSAStartup failed with error: %d\n",err);
			return 1;
		}
		winsock_initialized	= 1;
	}
#endif
	if (g_mcast_socket != -1) {
		fprintf(stderr, "optitrack_t: multicast socket already opened\n");
		return 0;
	}
	
	if ((g_mcast_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "optitrack_t: Could not create UDP socket\n");
		goto failure;
	}
	
	// Save local address: we will use it to wake the listening thread by sending a packet.

	g_mcast_address		= malloc(sizeof(struct sockaddr_in));
	si_me			= (struct sockaddr_in*)g_mcast_address;
	memset((char*)si_me, 0, sizeof(*si_me));
	si_me->sin_family	= AF_INET;
	si_me->sin_port		= htons(port);
	si_me->sin_addr.s_addr	= htonl(INADDR_ANY);

	if (bind(g_mcast_socket, (struct sockaddr*)si_me, sizeof(*si_me)) < 0) {
		fprintf(stderr, "optitrack_t: Could not bind UDP socket to local address and port\n");
		goto failure;
	}
	
	mreq.imr_multiaddr.s_addr = inet_addr(mcastGroup);
	if (local_interface == NULL)
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	else
		mreq.imr_interface.s_addr = inet_addr(local_interface);
	
	#ifndef _WIN32
		if (setsockopt(g_mcast_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
	#else
		if (setsockopt(g_mcast_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
	#endif
			fprintf(stderr, "optitrack_t: Could not join multicast group\n");
			goto failure;
		}
	#if 0
		// This helps the editor to match braces.
		}
	#endif

	g_blocking = blocking;
	if (!blocking) {
		#ifndef _WIN32
			res = fcntl(g_mcast_socket, F_SETFL, O_NONBLOCK);
		#else
			u_long iMode = 1;
			res = ioctlsocket(g_mcast_socket,FIONBIO,&iMode);
		#endif
	}
	
	return 0;
	
failure:
	if (g_mcast_socket != -1) {
		#ifndef _WIN32
				close(g_mcast_socket);
		#else
				closesocket(g_mcast_socket);
		#endif
		g_mcast_socket = -1;
	}
	
	return 1;
}



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
void Unpack(glm::vec3 &keyboard_origin, glm::quat &keyboard_orientation, std::vector<glm::vec3> &keyboard_markers_markers, std::vector<glm::vec3> &markers, char* pData)
{
    char *ptr = pData;
    int i, j;

    //printf("Begin Packet\n-------\n");

    // First 2 Bytes is message ID
    int MessageID = 0;
    memcpy(&MessageID, ptr, 2); ptr += 2;
    //printf("Message ID : %d\n", MessageID);

    // Second 2 Bytes is the size of the packet
    int nBytes = 0;
    memcpy(&nBytes, ptr, 2); ptr += 2;
    //printf("Byte count : %d\n", nBytes);
    
    if(MessageID == 7)      // FRAME OF MOCAP DATA packet
    {
        // Next 4 Bytes is the frame number
        int frameNumber = 0; memcpy(&frameNumber, ptr, 4); ptr += 4;
        //printf("Frame # : %d\n", frameNumber);
        
        // Next 4 Bytes is the number of data sets (markersets, rigidbodies, etc)
        int nMarkerSets = 0; memcpy(&nMarkerSets, ptr, 4); ptr += 4;
        //printf("Marker Set Count : %d\n", nMarkerSets);

        // Loop through number of marker sets and get name and data
        for (i=0; i < nMarkerSets; i++)
        {    
            // Markerset name
            char szName[256];
            strcpy_s(szName, ptr);

            int nDataBytes = (int) strlen(szName) + 1;
            ptr += nDataBytes;
            //printf("Model Name: %s\n", szName);

			
            // marker data
            int nMarkers = 0; memcpy(&nMarkers, ptr, 4); ptr += 4;
            //printf("Marker Count : %d\n", nMarkers);

            for(j=0; j < nMarkers; j++)
            {
                float x = 0; memcpy(&x, ptr, 4); ptr += 4;
                float y = 0; memcpy(&y, ptr, 4); ptr += 4;
                float z = 0; memcpy(&z, ptr, 4); ptr += 4;
	
				if (strcmp(szName, "keyboard") == 0) {
					keyboard_markers_markers.push_back(glm::vec3(x, y, z));
				}
            }
        }

        // Loop through unlabeled markers
        int nOtherMarkers = 0; memcpy(&nOtherMarkers, ptr, 4); ptr += 4;
        // OtherMarker list is Deprecated
        //printf("Unidentified Marker Count : %d\n", nOtherMarkers);
        for(j=0; j < nOtherMarkers; j++)
        {
            float x = 0.0f; memcpy(&x, ptr, 4); ptr += 4;
            float y = 0.0f; memcpy(&y, ptr, 4); ptr += 4;
            float z = 0.0f; memcpy(&z, ptr, 4); ptr += 4;
            
            // Deprecated
            markers.push_back(glm::vec3(x, y, z));
        }
        
        // Loop through rigidbodies
        int nRigidBodies = 0;
        memcpy(&nRigidBodies, ptr, 4); ptr += 4;
        //printf("Rigid Body Count : %d\n", nRigidBodies);
        for (j=0; j < nRigidBodies; j++)
        {
            // Rigid body position and orientation 
            int ID = 0; memcpy(&ID, ptr, 4); ptr += 4;
            float x = 0.0f; memcpy(&x, ptr, 4); ptr += 4;
            float y = 0.0f; memcpy(&y, ptr, 4); ptr += 4;
            float z = 0.0f; memcpy(&z, ptr, 4); ptr += 4;

            float qx = 0; memcpy(&qx, ptr, 4); ptr += 4;
            float qy = 0; memcpy(&qy, ptr, 4); ptr += 4;
            float qz = 0; memcpy(&qz, ptr, 4); ptr += 4;
            float qw = 0; memcpy(&qw, ptr, 4); ptr += 4;

			keyboard_orientation.x = qx; keyboard_orientation.y = qy; keyboard_orientation.z = qz; keyboard_orientation.w = qw;
			keyboard_origin.x = x; keyboard_origin.y = y; keyboard_origin.z = z;

            // NatNet version 2.0 and later

            // Mean marker error
            float fError = 0.0f; memcpy(&fError, ptr, 4); ptr += 4;
            //printf("Mean marker error: %3.2f\n", fError);

            // NatNet version 2.6 and later
            short params = 0; memcpy(&params, ptr, 2); ptr += 2;
		
        } // Go to next rigid body
    }
}



// receive_packet --

void receive_packet(glm::vec3& keyboard_origin, glm::quat& keyboard_orientation, std::vector<glm::vec3>& keyboard_markers_markers, std::vector<glm::vec3>& markers, char* buffer, int buffer_size)
{
	int				nbytes;
	struct sockaddr_in		peer;
	socklen_t			slen				= sizeof(peer);

	#ifndef _WIN32
		nbytes = recvfrom(g_mcast_socket, (void*)buffer, buffer_size, 0, (struct sockaddr*)&peer, &slen);
	#else
		nbytes = recvfrom(g_mcast_socket, (char*)buffer, buffer_size, 0, (struct sockaddr*)&peer, &slen);
	#endif

	if (nbytes > 0)
		Unpack(keyboard_origin, keyboard_orientation, keyboard_markers_markers, markers, buffer);
}

