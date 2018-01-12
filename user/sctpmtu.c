/* SCTP reference Implementation Copyright (C) 2001 Cisco And MotorolaThis file is part of the SCTP reference ImplementationVersion:4.0.5$Header: /home/sourceTree/src/sctp/user/sctpmtu.c,v 1.44 2001/07/24 11:52:39 randall Exp $The SCTP reference implementation  is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published bythe Free Software Foundation; either version 2, or (at your option)any later version.the SCTP reference implementation  is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of                ************************MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theGNU Library General Public License for more details.You should have received a copy of the GNU Library General Public Licensealong with GNU CC; see the file COPYING.  If not, write tothe Free Software Foundation, 59 Temple Place - Suite 330,Boston, MA 02111-1307, USA.  Please send any bug reports or fixes you make to one of the following emailaddresses:rrs@cisco.comkmorneau@cisco.comqxie1@email.mot.comAny bugs reported given to us we will try to fix... any fixes shared willbe incorperated into the next SCTP release.There are still LOTS of bugs in this code... I always run on the motto"it is a wonder any code ever works :)"*/#include <sctp.h>#include <sctputil.h>#include <stdio.h>#include <stdlib.h>#include <unistd.h>#include <sys/types.h>#include <netinet/in_systm.h>#include <netinet/in.h>#include <netinet/ip.h>#include <arpa/inet.h>#include <sys/ioctl.h>#include <sys/stat.h>#include <stdarg.h>#include <signal.h>#include <fcntl.h>#include <sys/socket.h>#if ! ( defined(LINUX) || defined(TRU64) || defined(AIX))#include <sys/sockio.h>#endif#include <time.h>#include <string.h>#ifdef LINUX #include <stropts.h>#endif#include <net/if.h>#include <errno.h>#include <netinet/ip_icmp.h>#include <adler32.h>#include <modAdler32.h>#include <crc32.h>#ifdef LINUX#include <linux/socket.h>#include <sys/uio.h>/* this is ugly but I do not feel like * getting the includes to work. They * seem to be broken. */struct msocket_extended_err {  u_int	 ee_errno;	  u_char ee_origin;  u_char ee_type;  u_char ee_code;  u_char ee_pad;  u_int  ee_info;  u_int  ee_data;};#ifndef MSG_ERRQUEUE#define MSG_ERRQUEUE	0x2000#endif#endif#define NUMBER_OF_MTU_SIZES 18/* a list of sizes based on * typical mtu's, used only if next * hop size not returned. */int mtuSizes[]={  68,  296,  508,  512,  544,  576,  1006,  1492,  1500,  1536,  2002,  2048,  4352,  4464,  8166,  17914,  32000,  65535};void SCTPfixCRC(struct SCTP *m,		struct SCTP_association *asoc,		struct sctpHeader *mh,		int siz){  /* take care of putting on a CRC or not based   * on the flag stuck to the association   */  unsigned int val;  mh->adler32 = 0;  /* generate crc  and place in postion in common header */#ifdef SCTP_USECRC_CRC16  val = crc16_tbl((u_char *)mh,siz);#endif#ifdef SCTP_USECRC_SSHCRC32  val = ssh_crc32((char *)mh,siz);#endif#ifdef SCTP_USECRC_MODADLER  val = count_mcrc((char *)mh,siz);#endif#ifdef SCTP_USECRC_FASTCRC32  val = ether_crc32_le_fast((u_char *)mh,siz);#endif#ifdef SCTP_USECRC_CRC16SMAL  val = crc16((u_char *)mh,siz);#endif#ifdef SCTP_USECRC_CRC32  val = ether_crc32_le((u_char *)mh,siz);#endif#ifdef SCTP_USECRC_TCP32  val = tcp32((u_char *)mh,siz);#endif#ifdef SCTP_USECRC_FLETCHER  val = fletcher((u_char *)mh,siz);#endif#ifdef SCTP_USECRC_RFC2960  val = count_crc((char *)mh,siz);#endif  mh->adler32 = htonl(val);  SCTPdebugPrint(m,"CRC enabled checking %d bytes at %x crc=%4.4x\n",		 siz,(u_int)mh,val);}int findNextBestMTU(int totsz){  int i,perfer;  /* if we are in here we must find   * the next best fit based on th   * size of the dg that failed to be   * sent.   */  perfer = 0;  for(i=0;i<NUMBER_OF_MTU_SIZES;i++){    if(totsz < mtuSizes[i]){      perfer = i -1;      if(perfer < 0)	perfer = 0;      break;    }  }  return(mtuSizes[perfer]);}u_longsctpGetNextMTU(struct SCTP *m,u_long curMtu){  /* select another MTU that is   * just bigger than this one   */  int i;  if(curMtu >= m->max_mtu){    /* never get bigger than the max of     * all our interface MTU's     */    return(m->max_mtu);  }  for(i=0;i<NUMBER_OF_MTU_SIZES;i++){    if(curMtu < mtuSizes[i]){      if(m->max_mtu < mtuSizes[i]){	/* is max_mtu smaller? if so return it */	return(m->max_mtu);      }else{	/* no max_mtu is bigger than this one */	return(mtuSizes[i]);      }    }  }  /* here return the highest allowable */  return(m->max_mtu);}struct SCTP_association *SCTPhandleICMP6(struct SCTP *m,struct ip6_hdr *ip6h){  int nxtsz;  struct ip6_hdr *badip6;  struct icmp6_hdr *icmph6;  struct sctpHeader *sch;  struct SCTP_association *asoc,*ret;  struct sockaddr_in6 sa;  int indx;  ret = NULL;  /* here a ICMP message awaits us */  SCTPdebugPrint(m,"!!!!!!!!!!!!!!Got a icmp6 message to process!!!!!!!!\n");    icmph6 = (struct icmp6_hdr *)((caddr_t)ip6h + sizeof(struct ip6_hdr));  if(icmph6->icmp6_type != ICMP6_PACKET_TOO_BIG){    /* not PACKET TO BIG */    return(ret);  }  /* ok we need to look closely,  it could be   * We could even get smarter and   * look at anyone that we sent to in case we   * get a different ICMP that tells us there   * is no way to reach a host, but for this   * impl, all we care about is MTU discovery.   */  nxtsz = ntohl(icmph6->icmp6_mtu);  badip6 = (struct ip6_hdr *)((caddr_t)icmph6 + sizeof(struct icmp6_hdr));  sch = (struct sctpHeader *)((caddr_t)badip6 + sizeof(struct ip6_hdr));  /* do we have a asoc for this guy? */  memset(&sa,0,sizeof(sa));  sa.sin6_family = AF_INET6;#ifdef USES_BSD_4_4_SOCKET	  sa.sin6_len = sizeof(struct sockaddr_in6);#endif  sa.sin6_port = sch->destination;  sa.sin6_addr = badip6->ip6_dst;  asoc = SCTPfindAssociation(m,			     (struct sockaddr *)&sa,			     &indx);  if(asoc == NULL){    SCTPdebugPrint(m,"Can't find bye\n");    return(ret);  }  /* we need to look to make sure that the VT is correct   * before proceeding with this   */  if(ntohl(sch->verificationTag) != asoc->hisTag){    /* incorrect tag */    return(ret);  }  /* stop the MTU raise timer */  timerWork(m,asoc,SCTP_TIMER_STOP,SctpTimerTypePathMtuRaise,0);  SCTPdebugPrint(m,"Verify that %d is smaller than %d (cur mtu of %d\n",		 nxtsz,asoc->nets[indx].MTU,indx);  if(nxtsz < asoc->nets[indx].MTU){    /* only update if this is smaller */    SCTPdebugPrint(m,"Yep update\n");    asoc->nets[indx].MTU = nxtsz;  }  /* we may want to update the smallest, if so   * we may have other pieces that need to allow   * IP fragmentation.   */  SCTPdebugPrint(m,"Now is the smallest MTU to update too?\n");  if((unsigned int)nxtsz < asoc->smallestMTU){    struct SCTP_transmitOnQueue *tmp;    SCTPdebugPrint(m,"Yep it was %d\n",asoc->smallestMTU);    asoc->smallestMTU = nxtsz;    /* hard case, go through all dg's and see if some     * need to allow fragmentation.     */    SCTPdebugPrint(m,"scanning out queue\n");    tmp = asoc->outqueue;    while(tmp != NULL){      if((unsigned int)tmp->sizeOfTmit > asoc->smallestMTU){	/* turn on the bit to allow fragmentation */	SCTPdebugPrint(m,"Found one to turn off df\n");	tmp->sendOptions &= ~SCTP_DONT_FRAGMENT;	tmp->sendOptions |= SCTP_FRAGMENT_OK;	tmp->sent = SCTP_DATAGRAM_RESEND;	asoc->numberToRetran++;	asoc->peersRwnd += (tmp->sizeOfTmit - sizeof(struct sctpUnifiedDatagram));	if(asoc->numnets > 1){	  /* We here play a trick. We do want	   * to retransmit to the same place	   * so we rig the retransmit algorithm	   * to reselect our same destination.	   */	  tmp->whichNet--;	  if(tmp->whichNet >= asoc->numnets)	    tmp->whichNet = asoc->numnets - 1;	  asoc->lastNetSentTo = tmp->whichNet;	}	ret = asoc;      }      tmp = tmp->next;    }  }  /* ok restart the timer to go off a long time from now */  timerWork(m,asoc,SCTP_TIMER_START,SctpTimerTypePathMtuRaise,0);  return(ret);}struct SCTP_association *SCTPhandleICMP(struct SCTP *m,struct ip *iph){  int totsz,nxtsz;  unsigned long off;  struct ip *badip;  struct icmp *icmph;  struct sctpHeader *sch;  struct SCTP_association *asoc,*ret;  struct sockaddr_in sa;  int indx;  ret = NULL;  /* here a ICMP message awaits us */  SCTPdebugPrint(m,"!!!!!!!Got a icmp message to process!!!!!!!!\n");  off = iph->ip_hl << 2;  icmph = (struct icmp *)((u_long)iph + off);  if(icmph->icmp_type != ICMP_UNREACH){    /* not unreachable */    return(ret);  }  SCTPdebugPrint(m,"Code is %d\n",icmph->icmp_code);  if(icmph->icmp_code != ICMP_UNREACH_NEEDFRAG){    /* not a unreachable message due to frag. */    SCTPdebugPrint(m,"Not unreachable .. out of here \n");    return(ret);  }  /* ok we need to look closely,  it could be   * We could even get smarter and   * look at anyone that we sent to in case we   * get a different ICMP that tells us there   * is no way to reach a host, but for this   * impl, all we care about is MTU discovery.   */  nxtsz = ntohs(icmph->icmp_seq);  SCTPdebugPrint(m,"Size in icmp_seq is %d\n",nxtsz);  badip = &icmph->icmp_ip;  off = badip->ip_hl << 2;#if (defined (TRU64) || defined (FREEBSD))  totsz = (badip->ip_len - off);#else  totsz = (((unsigned short)ntohs(badip->ip_len)) - off);#endif  SCTPdebugPrint(m,"Total size is %d (badip->ip_len:%d - off:%d)\n",      totsz,badip->ip_len,off);  SCTPdebugPrintArry(m,(char *)badip,sizeof(struct ip));  sch = (struct sctpHeader *)((u_long)badip + off);  /* do we have a asoc for this guy? */  SCTPdebugPrint(m,		 "Lookup assoc for %x:%d nxtsz = %d; totsz = %d; ip offset = %d\n",		 badip->ip_dst.s_addr,sch->destination, nxtsz, totsz, off);  memset(&sa,0,sizeof(sa));  sa.sin_family = AF_INET;#ifdef USES_BSD_4_4_SOCKET  sa.sin_len = sizeof(struct sockaddr_in);#endif  sa.sin_port = sch->destination;  sa.sin_addr = badip->ip_dst;  asoc = SCTPfindAssociation(m,(struct sockaddr *)&sa,&indx);  if(asoc == NULL){    SCTPdebugPrint(m,"Can't find bye\n");    return(ret);  }  /* we need to look to make sure that the VT is correct   * before proceeding with this   */  if(sch->verificationTag == 0){    /* incorrect tag */    return(ret);  }  if(ntohl(sch->verificationTag) != asoc->hisTag){    /* incorrect tag */    return(ret);  }  /* stop the MTU raise timer */  timerWork(m,asoc,SCTP_TIMER_STOP,SctpTimerTypePathMtuRaise,0);  if(nxtsz == 0){    /* old type router that does not tell     * us what the next size mtu     * is. Rats we will have to guess (in a      * educated fashion of course)     */    nxtsz = findNextBestMTU(totsz);    SCTPdebugPrint(m,"did best fit algo, got %d\n",nxtsz);  }  SCTPdebugPrint(m,"Verify that %d is smaller than %d (cur mtu of %d\n",		 nxtsz,asoc->nets[indx].MTU,indx);  if(nxtsz < asoc->nets[indx].MTU){    /* only update if this is smaller */    SCTPdebugPrint(m,"Yep update\n");    asoc->nets[indx].MTU = nxtsz;  }  /* we may want to update the smallest, if so   * we may have other pieces that need to allow   * IP fragmentation.   */  SCTPdebugPrint(m,"Now is the smallest MTU to update too?\n");  if((unsigned int)nxtsz < asoc->smallestMTU){    struct SCTP_transmitOnQueue *tmp;    SCTPdebugPrint(m,"Yep it was %d\n",asoc->smallestMTU);    asoc->smallestMTU = nxtsz;    /* hard case, go through all dg's and see if some     * need to allow fragmentation.     */    SCTPdebugPrint(m,"scanning out queue\n");    tmp = asoc->outqueue;    while(tmp != NULL){      if((unsigned int)tmp->sizeOfTmit > asoc->smallestMTU){	/* turn on the bit to allow fragmentation */	SCTPdebugPrint(m,"Found one to turn off df\n");	tmp->sendOptions &= ~SCTP_DONT_FRAGMENT;	tmp->sendOptions |= SCTP_FRAGMENT_OK;	tmp->sent = SCTP_DATAGRAM_RESEND;	asoc->numberToRetran++;	asoc->peersRwnd += (tmp->sizeOfTmit - sizeof(struct sctpUnifiedDatagram));	if(asoc->numnets > 1){	  /* We here play a trick. We do want	   * to retransmit to the same place	   * so we rig the retransmit algorithm	   * to reselect our same destination.	   */	  tmp->whichNet--;	  if(tmp->whichNet >= asoc->numnets)	    tmp->whichNet = asoc->numnets - 1;	  asoc->lastNetSentTo = tmp->whichNet;	}	ret = asoc;      }      tmp = tmp->next;    }  }  /* ok restart the timer to go off a long time from now */  timerWork(m,asoc,SCTP_TIMER_START,SctpTimerTypePathMtuRaise,0);  return(ret);}u_longSCTPmakeABrainDeadIPv4ChoiceOfSrcAddress(struct SCTP *m,					 struct SCTP_association *asoc,					 u_long toaddr,					 struct in_addr *fillin)					 {  /* Has you can probably tell by the name of this function   * call I think this is a BAD idea. If I was in the kernel   * I could consult the routing tables and first determine   * if the packet destination in "toaddr" was going to be   * sent on a NIC card that I have an address bound to. If   * not I could use round robin has a backup where none of   * my address are bound to that card... In this case the   * only thing I can do is use the netmask list to see if   * the destination is on one of the same subnets as me. If so   * then I will use that address, otherwise I drop back to the   * 40 and round-robin.. This would be much better if I was   * in the kernel.. I guess I said that already... well you   * get the idea :)   */  int i;  struct sockaddr_in *adr,*msk;  for(i=0;i<asoc->numberOfNets;i++){    if(asoc->addrStats[i] == SCTP_ADDR_BEING_DELETED){      /* we don't use critters being deleted */      SCTPdebugPrint(m,"We skip ones being deleted %d\n",i);      continue;    }    if(asoc->localNets[i]->sa_family != AF_INET){      SCTPdebugPrint(m,"Not my family\n");      continue;    }    adr = (struct sockaddr_in *)asoc->localNets[i];    msk = (struct sockaddr_in *)&asoc->maskList[i];    if((adr->sin_addr.s_addr & msk->sin_addr.s_addr) ==       (toaddr & msk->sin_addr.s_addr)){      /* boy that was a lot, but we have a winner. I sure hope that       * IPv6 has some helper functions or this could get real cute :)       */      SCTPdebugPrint(m,"Hey, we lucked out, its on my sub-net index:%d addr:%x\n",			     i,ntohl(adr->sin_addr.s_addr));      fillin->s_addr = adr->sin_addr.s_addr;      return(0);    }  }  fillin->s_addr = 0;    return(-1);}intSCTPmakeABrainDeadIPv6ChoiceOfSrcAddress(struct SCTP *m,					 struct SCTP_association *asoc,					 struct sockaddr_in6 *To,					 struct in6_addr *fillin,					 struct sockaddr *toAddr){  /* First if the address is link local scope, we trust   * the O/S to figure out which interface it goes with   * so we can safetly put the un-specified address in.   */  struct sockaddr_in6 *Dest;  int i;  if(IN6_IS_ADDR_LINKLOCAL(&To->sin6_addr)){    /*      * This has the ramification that we cannot     * allow any of the link local addresses     * from being deleted.     */    SCTPdebugPrint(m,"Sending to a link local address O/S picks\n");    memset(fillin,0,sizeof(struct in6_addr));    return(0);  }  if(asoc == NULL){    if((toAddr != NULL) && (toAddr->sa_family == AF_INET6)){      *fillin = ((struct sockaddr_in6 *)toAddr)->sin6_addr;      return(0);    }    SCTPdebugPrint(m,"no association and no toAddr?\n");    return(-1);  }  if(IN6_IS_ADDR_SITELOCAL(&To->sin6_addr)){    /* Find the SITE local that is the same subnet,     * if possible, based on the side bits which     * are the 16 bits right above the 64 bit      * interface ID.      * Ie: fec0:0000:0000:xxxx:IIII:IIII:IIII:IIII:     * where xxxx is the site ide we want.     */    SCTPdebugPrint(m,"Sending to site local, select a matching one?\n");    for(i=0;i<asoc->numberOfNets;i++){          if(asoc->addrStats[i] == SCTP_ADDR_BEING_DELETED)	/* we don't use critters being deleted */	continue;      if(asoc->localNets[i]->sa_family != AF_INET6)	continue;      Dest = (struct sockaddr_in6 *)asoc->localNets[i];      if(!IN6_IS_ADDR_SITELOCAL(&Dest->sin6_addr))	continue;      if((To->sin6_addr.s6_addr[6] == Dest->sin6_addr.s6_addr[6]) &&	 (To->sin6_addr.s6_addr[7] == Dest->sin6_addr.s6_addr[7])){	/* Same site subnet	 */	*fillin = Dest->sin6_addr;	return(0);      }    }    /*      * Ok if we reach here we don't have a interface on     * the same subnet. So lets round robin amongst site     * local addresses.     */    for(i=(asoc->lastAddrSourced+1);i<asoc->numberOfNets;i++){      if(asoc->addrStats[i] == SCTP_ADDR_BEING_DELETED)	/* we don't use critters being deleted */	continue;      if(asoc->localNets[i]->sa_family != AF_INET6)	continue;      Dest = (struct sockaddr_in6 *)asoc->localNets[i];      if(!IN6_IS_ADDR_SITELOCAL(&Dest->sin6_addr))	continue;      *fillin = Dest->sin6_addr;      asoc->lastAddrSourced = i;      return(0);    }    /* Ok if we reach here, there is NO site      * address presently set for us.. kinda wierd     * but lets get a global scope address and     * hand that out.     */  }  /* Select from the global scope an address   * to use.   */  SCTPdebugPrint(m,"Select a global scope address round robin\n");  for(i=(asoc->lastAddrSourced+1);i<asoc->numberOfNets;i++){    if(asoc->addrStats[i] == SCTP_ADDR_BEING_DELETED){      /* we don't use critters being deleted */      SCTPdebugPrint(m,"Address %d is scheduled for deletion - skip\n",i);      continue;    }    if(asoc->localNets[i]->sa_family != AF_INET6)      continue;      Dest = (struct sockaddr_in6 *)asoc->localNets[i];      if(IN6_IS_ADDR_SITELOCAL(&Dest->sin6_addr))	continue;      if(IN6_IS_ADDR_LINKLOCAL(&Dest->sin6_addr))	continue;      *fillin = Dest->sin6_addr;      asoc->lastAddrSourced = i;      return(0);  }  /* Drop back to the 40 and punt */  memset(fillin,0,sizeof(struct in6_addr));      return(-1);}voidSCTPfixMTUFromSendError(struct SCTP *m,			struct SCTP_association *asoc,			struct sockaddr *to,int SizTOBig){  struct SCTP_association *lasoc;  int indx,nxtsz;  int sizeLimit;  /* find which net */  lasoc = SCTPfindAssociation(m,to,&indx);	  if(lasoc == NULL){    SCTPdebugPrint(m,"Can't find bye ASSOC!!\n");    return;  }  /* stop the MTU raise timer */  timerWork(m,asoc,SCTP_TIMER_STOP,SctpTimerTypePathMtuRaise,0);  nxtsz = findNextBestMTU(SizTOBig);  if(nxtsz < asoc->nets[indx].MTU){    /* only update if this is smaller */    SCTPdebugPrint(m,		   "Shrinking MTU size of net, update lan MTU\n");    asoc->nets[indx].MTU = nxtsz;  }  /* we may want to update the smallest, if so   * we may have other pieces that need to allow   * IP fragmentation.   */  SCTPdebugPrint(m,		 "Now is the smallest MTU to update too?\n");  if((unsigned int)nxtsz < asoc->smallestMTU){    struct SCTP_transmitOnQueue *tmp;    SCTPdebugPrint(m,"Yep it was %d\n",asoc->smallestMTU);    asoc->smallestMTU = nxtsz;    /* hard case, go through all dg's and see if some     * need to allow fragmentation.     */    if(asoc->ECN_Allowed){      /* Always reserve size (8 bytes) for ECN */      sizeLimit = asoc->smallestMTU - ((sizeof(struct sctpHeader) + IP_HDR_SIZE) + 					   sizeof(struct sctpECN_echo));    }else{      sizeLimit = asoc->smallestMTU - ((sizeof(struct sctpHeader) + IP_HDR_SIZE));    }    SCTPdebugPrint(m,		   "scanning out queue for smaller than %d (mtu:%d)\n",		   (int)sizeLimit,(int)asoc->smallestMTU);    tmp = asoc->outqueue;    while(tmp != NULL){      if((unsigned int)tmp->sizeOfTmit > sizeLimit){	/* turn on the bit to allow fragmentation */	SCTPdebugPrint(m,		       "Found one to turn off df\n");	tmp->sendOptions &= ~SCTP_DONT_FRAGMENT;	tmp->sendOptions |= SCTP_FRAGMENT_OK;	tmp->sent = SCTP_DATAGRAM_RESEND;	asoc->numberToRetran++;	asoc->peersRwnd += (tmp->sizeOfTmit - sizeof(struct sctpUnifiedDatagram));	if(asoc->numnets > 1){	  /* We here play a trick. We do want	   * to retransmit to the same place	   * so we rig the retransmit algorithm	   * to reselect our same destination.	   */	  tmp->whichNet--;	  if(tmp->whichNet >= asoc->numnets)	    tmp->whichNet = asoc->numnets - 1;	  asoc->lastNetSentTo = tmp->whichNet;	}      }      tmp = tmp->next;    }  }  /* ok restart the timer to go off a long time from now */  timerWork(m,asoc,SCTP_TIMER_START,SctpTimerTypePathMtuRaise,0);}intSCTPlsendto(struct SCTP *m,	    struct SCTP_association *asoc,	    char *sendbuffer,int sendSize,int flg,	    struct sockaddr *to,	    int flow,	    char tos,	    struct sockaddr *toAddr,	    int netIndx){  /* send based on what the MTU discover state is */  /* send the raw IP datagram. */  struct ip *iph;  struct ip6_hdr *ip6h;  struct sctpHeader *sctph;  struct sockaddr_in deamon;  struct sockaddr *outto;  int sizaddr,iphdrSz;  int ret;  /*    u_short ssm; */  char buf[SCTP_MAX_READBUFFER];  /* build and send a IP datagram complete with   * headers etc.   */  if((to->sa_family != AF_INET) && (to->sa_family != AF_INET6)){    SCTPdebugPrint(m,"send:Unknown protocol type %d return -1\n",to->sa_family);    return(-1);  }  if(to->sa_family == AF_INET){    struct sockaddr_in *To;    iph = (struct ip *)buf;    sctph = (struct sctpHeader *)((u_long)buf + sizeof(struct ip));    To = (struct sockaddr_in *)to;    if(IN_MULTICAST(To->sin_addr.s_addr)){      errno = ENOPROTOOPT;      return(-1);    }    SCTPdebugPrint(m,"AF_INET in to address\n");    iphdrSz = sizeof(struct ip);    iph->ip_hl = 5;    iph->ip_v = IPVERSION;    iph->ip_ttl = (u_char)(m->ttl & 0x000000ff);#if (defined (TRU64) || defined (FREEBSD))  /* SCTP_PB_3 */    iph->ip_len = sendSize + iphdrSz;#else    iph->ip_len = htons(sendSize+iphdrSz);#endif    iph->ip_id = htons(m->ipcounter);    m->ipcounter++;    /* to fragment or not to fragment? */    if(flg & SCTP_DONT_FRAGMENT){      SCTPdebugPrint(m,"Setting IP DF %x\n",IP_DF);#if (defined (TRU64) || defined (FREEBSD))    /* SCTP_PB_3 */      iph->ip_off = IP_DF;#else       iph->ip_off = htons(IP_DF);#endif    }else{      SCTPdebugPrint(m,"Fragmentation PLEASE! %x\n",IP_DF);      iph->ip_off = 0;    }    /* set in the other stuff */    if(asoc !=  NULL){      if(asoc->ECN_Allowed){	/* Enable ECN */	iph->ip_tos = (u_char)((tos & 0x000000fc) | SCTP_ECT_BIT);      }else{	/* No ECN */	iph->ip_tos = (u_char)(tos & 0x000000fc);      }    }else{      /* we could get no assoc if it is a O-O-T-B packet */      iph->ip_tos = (u_char)(tos & 0x000000fc);    }    iph->ip_p = IPPROTO_SCTP;    iph->ip_sum = 0;    if(asoc == NULL){      /* Here we are responding to a INIT */      SCTPdebugPrint(m, "Assoc is NULL\n");      if(m->numberOfNets == 1){	if(m->localNets[0]->sa_family != to->sa_family){	  SCTPdebugPrint(m, "Gak to family is not same as localNet[0]\n");	  errno = ENOPROTOOPT;	  return(-1);	}	/* only one source possible */	SCTPdebugPrint(m, "Source address set to my local source address [0]\n");	SCTPdebugPrintAddress(m,m->localNets[0]);	iph->ip_src.s_addr = ((struct sockaddr_in *)m->localNets[0])->sin_addr.s_addr;      }else{	/* hmm, let operating system provide value */	SCTPdebugPrint(m, "Multiple addresses set src to 0\n");	iph->ip_src.s_addr = 0;        }    }else{      if(asoc->listConsistent){	/* Here is the prefered, we don't specify an address	 * since we are consistent with our known address	 * list. Let the O/S pick the best address.	 */	SCTPdebugPrint(m,"List consistent, yes 0\n");	iph->ip_src.s_addr = 0;        }else{	/* Yikes, this is the worst! We must make a	 * choice amongst OUR address list. This may	 * will lead to trouble, since I really have	 * such limited knowledge and you really want	 * to assign the address (if possible) that 	 * the packet is emited upon. This of course	 * is based on the routing tables which we do	 * not have easy access to up here in user land.	 */	SCTPdebugPrint(m,"El-yucko address selection required\n");	/* This may error and if so we get a 0 address 	 * One cause of such an error is when we are	 * singly homed and do a ADD/DELETE. Otherwise	 * this packet is headed for trouble .. and	 * the association too...	 */	iph->ip_src.s_addr = 0;  	SCTPmakeABrainDeadIPv4ChoiceOfSrcAddress(m,asoc,						 To->sin_addr.s_addr,						 &iph->ip_src);	SCTPdebugPrint(m,"Set in %x\n",iph->ip_src.s_addr);      }    }	    SCTPdebugPrint(m,"Ok set in destination\n");    iph->ip_dst.s_addr = To->sin_addr.s_addr;    iph->ip_sum = 0;  }else if(to->sa_family == AF_INET6){    struct sockaddr_in6 *To;    u_char flowTop;    u_short flowBottom;    u_char tosBottom,tosTop;    ip6h = (struct ip6_hdr *)buf;    iphdrSz = sizeof(struct ip6_hdr);    To = (struct sockaddr_in6 *)to;    SCTPdebugPrint(m,"Filling in a IPV6 header\n");    if(IN6_IS_ADDR_MULTICAST(&To->sin6_addr)){      errno = ENOPROTOOPT;      return(-1);    }    sctph = (struct sctpHeader *)((u_long)buf + sizeof(struct ip6_hdr));    flowBottom = (flow & 0x0000ffff);    flowTop = ((flow&0x000f0000) >> 16);    tosTop = (((tos & 0xf0) >> 4) | IPV6_VERSION);    if(asoc !=  NULL){      if(asoc->ECN_Allowed){	/* Enable ECN */	tosBottom = (((tos & 0x0c)| SCTP_ECT_BIT) << 4);      }else{	/* No ECN */	tosBottom = ((tos & 0x0c) << 4);      }    }else{      /* we could get no assoc if it is a O-O-T-B packet */      tosBottom = ((tos & 0x0c) << 4);    }    ip6h->ip6_flow = htonl(((tosTop  << 24) | ((tosBottom|flowTop) << 16) | flowBottom));    SCTPdebugPrint(m,"Upper 32 bits will be %x %x %x %x\n",		   buf[0],buf[1],buf[2],buf[3]);    /* set by lower layer, but we will set it anyway */    ip6h->ip6_plen = htons(sendSize);    ip6h->ip6_nxt = IPPROTO_SCTP;    SCTPdebugPrint(m,"next hop:%x\n",ip6h->ip6_nxt);    ip6h->ip6_hlim = (u_char)(m->ttl & 0x000000ff);    SCTPdebugPrint(m," hop limit:%x\n",ip6h->ip6_hlim);    ip6h->ip6_dst  = To->sin6_addr;    if(asoc == NULL){      /* Here we are responding to a INIT */      if(m->numberOfNets == 1){	/* verify the types */	if(m->localNets[0]->sa_family != to->sa_family){	  errno = ENOPROTOOPT;	  return(-1);	}	/* only one source possible */	memcpy(&ip6h->ip6_src,	       &((struct sockaddr_in6 *)(m->localNets[0]))->sin6_addr,	       sizeof(struct in6_addr));      }else{	/* hmm, let operating system provide value */	memset(&ip6h->ip6_src,0,sizeof(struct in6_addr));#ifdef OS_CANT_SELECT_SRC	SCTPmakeABrainDeadIPv6ChoiceOfSrcAddress(m,asoc,To,						 &ip6h->ip6_src,						 toAddr);#else	SCTPdebugPrint(m, "Multiple addresses set src to 0\n");#endif      }    }else{      if(asoc->listConsistent){	/* Here is the prefered, we don't specify an address	 * since we are consistent with our known address	 * list. Let the O/S pick the best address.	 */	memset(&ip6h->ip6_src,0,sizeof(struct in6_addr));#ifdef OS_CANT_SELECT_SRC	/* This may error and if so we get a 0 address 	 * One cause of such an error is when we are	 * singly homed and do a ADD/DELETE. Otherwise	 * this packet is headed for trouble .. and	 * the association too...	 */	SCTPmakeABrainDeadIPv6ChoiceOfSrcAddress(m,asoc,To,&ip6h->ip6_src,						 toAddr);#else	SCTPdebugPrint(m,"List consistent, yes 0\n");#endif      }else{	/* Yikes, this is the worst! We must make a	 * choice amongst OUR address list. This may	 * will lead to trouble, since I really have	 * such limited knowledge and you really want	 * to assign the address (if possible) that 	 * the packet is emited upon. This of course	 * is based on the routing tables which we do	 * not have easy access to up here in user land.	 */	memset(&ip6h->ip6_src,0,sizeof(struct in6_addr));	SCTPdebugPrint(m,"El-yucko address selection required\n");	SCTPmakeABrainDeadIPv6ChoiceOfSrcAddress(m,asoc,To,&ip6h->ip6_src,						 toAddr);      }    }  }else{    errno = EPROTONOSUPPORT;    return(-1);  }  /* copy the data */  memcpy((u_char *)sctph,(u_char *)sendbuffer,sendSize);  /* fix the port number */  SCTPdebugPrint(m,"Source port:%d Destination port:%d\n",			 ntohs(m->port),ntohs(((struct sockaddr_in *)to)->sin_port));  sctph->source = m->port;  sctph->destination = ((struct sockaddr_in *)to)->sin_port;  SCTPfixCRC(m,asoc,sctph,sendSize);  if(m->usingTheDeamon){    memset(&deamon,0,sizeof(deamon));    deamon.sin_port = htons(SCTP_DEAMON_PORT);    deamon.sin_family = AF_INET;#ifdef USES_BSD_4_4_SOCKET    deamon.sin_len = sizeof(struct sockaddr_in);#endif    deamon.sin_addr.s_addr = htonl(0x7f000001);    outto = (struct sockaddr *)&deamon;    sizaddr = sizeof(struct sockaddr_in);  }else{    /* sending directly!! */    outto = to;    if(to->sa_family == AF_INET)      sizaddr = sizeof(struct sockaddr_in);    else      sizaddr = sizeof(struct sockaddr_in6);  }  SCTPdebugPrint(m,"Sending out %d bytes (addr:%x):\n",(sendSize+iphdrSz),(u_int)buf);  SCTPdebugPrintArry(m,buf,(sendSize+iphdrSz));  ret = sendto(m->fd,	       buf,	       (sendSize+iphdrSz),	       0,	       outto,	       sizaddr);  if(ret > 0){    if(asoc != NULL){      asoc->pegs[SCTP_DATAGRAMS_SENT]++;    }  }else if(ret < 0){    if(errno == EMSGSIZE){      /* message we just sent was TOO large */      SCTPdebugPrint(m,"Message of sz:%d was to large\n",(sendSize+iphdrSz));      SCTPfixMTUFromSendError(m,asoc,to,(sendSize+iphdrSz));    }else{      if(((errno == EHOSTDOWN) || (errno == EHOSTUNREACH)) && (asoc != NULL)){	/* Fix it so the path is marked unreachable */	if(netIndx == -1){	  /* upper layer does not know, find it */	  int myNet;	  struct SCTP_association *lassoc;	  lassoc = SCTPfindAssociation(m,to , &myNet);	  if(lassoc == asoc)	    netIndx = myNet;	}	if((netIndx != -1) && (netIndx < asoc->numnets)){	  struct SCTP_nets *netp;	  netp = &asoc->nets[netIndx];	  if((netp->intfRotationState&SCTP_ADDR_NOT_REACHABLE) != SCTP_ADDR_NOT_REACHABLE){	    netp->errorCount = netp->failureThreshold;	    SCTPdebugPrint(m,"Lost lan %d in SEND with HOST unreachable %d!\n",netIndx,errno);	    netp->intfRotationState &= ~SCTP_ADDR_REACHABLE;	    netp->intfRotationState |= SCTP_ADDR_NOT_REACHABLE;	    SCTPupperLayerNotification(m,asoc,SCTP_NOTIFY_INTF_DOWN,				       (int)to);	  }	  return(ret);	}      }      SCTPdebugPrint(m,"Send error %d\n",errno);    }  }  return(ret);}