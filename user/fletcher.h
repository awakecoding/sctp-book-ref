/* SCTP reference Implementation Copyright (C) 2001 Cisco And MotorolaThis file is part of the SCTP reference ImplementationVersion:4.0.5$Header: /home/sourceTree/src/sctp/user/fletcher.h,v 1.2 2001/07/24 11:52:39 randall Exp $The SCTP reference implementation  is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published bythe Free Software Foundation; either version 2, or (at your option)any later version.the SCTP reference implementation  is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of                ************************MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theGNU Library General Public License for more details.You should have received a copy of the GNU Library General Public Licensealong with GNU CC; see the file COPYING.  If not, write tothe Free Software Foundation, 59 Temple Place - Suite 330,Boston, MA 02111-1307, USA.  Please send any bug reports or fixes you make to one of the following emailaddresses:rrs@cisco.comkmorneau@cisco.comAny bugs reported given to us we will try to fix... any fixes shared willbe incorperated into the next SCTP release.There are still LOTS of bugs in this code... I always run on the motto"it is a wonder any code ever works :)"*/#ifndef __fletcher_h__#define __fletcher_h__#ifdef	__cplusplus#include <sctpConstants.h>#include <sctpStructs.h>extern "C" {#endif  unsigned int fletcher(const unsigned char *buf, int len);  unsigned int tcp32(const unsigned char *buf, int len);#ifdef	__cplusplus}#endif#endif