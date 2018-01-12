/* SCTP reference Implementation Copyright (C) 2001 CiscoThis file is part of the SCTP reference ImplementationVersion:4.0.5$Header: /home/sourceTree/src/sctp/user/routingAdaptor.h,v 1.6 2001/07/24 11:52:39 randall Exp $The SCTP reference implementation  is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published bythe Free Software Foundation; either version 2, or (at your option)any later version.the SCTP reference implementation  is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of                ************************MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theGNU Library General Public License for more details.You should have received a copy of the GNU Library General Public Licensealong with GNU CC; see the file COPYING.  If not, write tothe Free Software Foundation, 59 Temple Place - Suite 330,Boston, MA 02111-1307, USA.  Please send any bug reports or fixes you make to one of the following emailaddresses:rrs@cisco.comkmorneau@cisco.comAny bugs reported given to us we will try to fix... any fixes shared willbe incorperated into the next SCTP release.There are still LOTS of bugs in this code... I always run on the motto"it is a wonder any code ever works :)"*/#ifndef __routingAdaptor_h__#define __routingAdaptor_h__#include <distributor.h>#include <sctp.h>#include <sys/param.h>#include <sys/types.h>#include <sys/socket.h>#if ! (defined (SOLARIS))#include <sys/sysctl.h>#endif#include <sys/ioctl.h>#include <net/route.h>#include <net/if.h>#if ! (defined (LINUX))#include <net/if_dl.h>#include <net/if_types.h>#endif#include <netinet/in.h>#include <netinet/if_ether.h>#include <arpa/inet.h>#include <errno.h>#include <stdio.h>#include <string.h>#include <stdlib.h>#include <unistd.h>#if ! (defined (SOLARIS))#include <err.h>#endif#include <fcntl.h>#include "routingAdaptDef.h"struct routingAdaptor *createRoutingAdaptor(struct distributor *d, struct SCTP *s, u_int flags);voiddestroyRoutingAdaptor(struct routingAdaptor *r);introutingAdaptorFdEvent(void *ra, int fd, int event);voidroutingAdaptorTimer(void *ra, void *timer);voidprintRoutingTable(struct routingAdaptor *r);#endif