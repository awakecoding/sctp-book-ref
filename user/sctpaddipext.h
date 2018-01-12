/* SCTP reference Implementation Copyright (C) 2001 Cisco And MotorolaThis file is part of the SCTP reference ImplementationVersion:4.0.5$Header: /home/sourceTree/src/sctp/user/sctpaddipext.h,v 1.11 2001/07/24 11:52:39 randall Exp $The SCTP reference implementation  is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published bythe Free Software Foundation; either version 2, or (at your option)any later version.the SCTP reference implementation  is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of                ************************MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theGNU Library General Public License for more details.You should have received a copy of the GNU Library General Public Licensealong with GNU CC; see the file COPYING.  If not, write tothe Free Software Foundation, 59 Temple Place - Suite 330,Boston, MA 02111-1307, USA.  Please send any bug reports or fixes you make to one of the following emailaddresses:rrs@cisco.comkmorneau@cisco.comqxie1@email.mot.comAny bugs reported given to us we will try to fix... any fixes shared willbe incorperated into the next SCTP release.There are still LOTS of bugs in this code... I always run on the motto"it is a wonder any code ever works :)"*/#ifndef __sctpaddipext_h__#define __sctpaddipext_h__#include <sctp.h>#ifdef	__cplusplusextern "C" {#endif/* These modules all return a non-zero to indicate that * the replyParam was filled in. If they return 0, then * the operation was sucessful and no reply will be * added (using the implict sucess mode) */intSCTPcheckListConsistency(struct SCTP *m,			 struct SCTP_association *asoc,			 struct sockaddr **newlist,			 int numnets);intSCTP_processAddAddress(struct SCTP *m,		       struct SCTP_association *asoc,		       u_short ptype,		       ushort plen,		       struct sctpParamDesc *inreq,		       struct sctpParamDesc *replyParam);intSCTP_processDelAddress(struct SCTP *m,		       struct SCTP_association *asoc,		       u_short ptype,		       ushort plen,		       struct sctpParamDesc *inreq,		       struct sctpParamDesc *replyParam);intSCTP_processSetPrimary(struct SCTP *m,		       struct SCTP_association *asoc,		       u_short ptype,		       ushort plen,		       struct sctpParamDesc *inreq,		       struct sctpParamDesc *replyParam);voidSCTP_handleAddResponse(struct SCTP *m,		       struct SCTP_association *asoc,		       struct sctpParamDesc *request,		       struct sctpParamDesc *reply);voidSCTP_handleDelResponse(struct SCTP *m,		       struct SCTP_association *asoc,		       struct sctpParamDesc *request,		       struct sctpParamDesc *reply);voidSCTP_handleSetPrimaryResponse(struct SCTP *m,			      struct SCTP_association *asoc,			      struct sctpParamDesc *request,			      struct sctpParamDesc *reply);#ifdef	__cplusplus}#endif#endif