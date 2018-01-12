/* SCTP reference Implementation Copyright (C) 2001 CiscoThis file is part of the SCTP reference ImplementationVersion:4.0.5$Header: /home/sourceTree/src/sctp/user/routingAdaptDef.h,v 1.8 2001/07/24 11:52:39 randall Exp $The SCTP reference implementation  is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published bythe Free Software Foundation; either version 2, or (at your option)any later version.the SCTP reference implementation  is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of                ************************MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theGNU Library General Public License for more details.You should have received a copy of the GNU Library General Public Licensealong with GNU CC; see the file COPYING.  If not, write tothe Free Software Foundation, 59 Temple Place - Suite 330,Boston, MA 02111-1307, USA.  Please send any bug reports or fixes you make to one of the following emailaddresses:rrs@cisco.comkmorneau@cisco.comAny bugs reported given to us we will try to fix... any fixes shared willbe incorperated into the next SCTP release.There are still LOTS of bugs in this code... I always run on the motto"it is a wonder any code ever works :)"*/#ifndef __routingAdaptDef_h__#define __routingAdaptDef_h__#include <sctp.h>#include <dlist.h>/* Types of timers */#define RoutingTimerType_NONE        0x0000#define RoutingTimerType_SCTP_COMMIT 0x0001/* timer running or not */#define RoutingTimer_STOPPED	0x0001#define RoutingTimer_RUNNING	0x0002#define ROUTE_READ_BUF_SIZE 4096struct routingTimer{  u_short up;  u_short type;};#define ROUTING_NUMBER_OF_TIMERS 2 /* we allocate 2 but use only 1 *//* flag values that can be passed to control * behavior. */#define ROUTING_UPDATE_ALL_ASSOC 	0x0001#define ROUTING_DONT_SET_PRIMARY	0x0002#define ROUTING_PRIMARY_TRACKS_DEF_RT	0x0004#define ROUTING_PRIMARY_TRACKS_HOST_RT	0x0008#define ROUTING_DONT_DO_INITIAL_PRIMARY 0x0010#define ROUTING_MAX_LINK_ADDRESS 32struct Route_ifname {  char if_name[(IFNAMSIZ+1)];  int if_type;  int addrLen;  int index;  char linkAddr[ROUTING_MAX_LINK_ADDRESS];  dlist_t *addrList;  dlist_t *routes;};struct routeEntry{  struct Route_ifname *ifp;	/* interface associated with */  struct sockaddr *dst;		/* The destination */  struct sockaddr *gate;	/* The gateway address */  struct sockaddr *netmask;	/* I have as yet to figure this out */};struct routingAdaptor{  u_int flags;		/* behavior flags for add/delete */  u_int flags_passed;	/* user passed flags on creation */  int routefd;		/* my fd */  dlist_t *iflist;  struct distributor *dist;  struct sockaddr *defv6;  struct sockaddr *defv4;  struct sockaddr *lastKnownPrimary;  struct SCTP *sctp;	/* sctp instance to talk to */  struct routingTimer timers[ROUTING_NUMBER_OF_TIMERS];};#endif