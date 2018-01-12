/* * Copyright (c) 2001 Cisco Systems Inc. * Copyright (c) 1982, 1986, 1989, 1993 *	The Regents of the University of California.  All rights reserved. * * Redistribution and use in source and binary forms, with or without * modification, are permitted provided that the following conditions * are met: * 1. Redistributions of source code must retain the above copyright *    notice, this list of conditions and the following disclaimer. * 2. Redistributions in binary form must reproduce the above copyright *    notice, this list of conditions and the following disclaimer in the *    documentation and/or other materials provided with the distribution. * 3. All advertising materials mentioning features or use of this software *    must display the following acknowledgement: *	This product includes software developed by the University of *	California, Berkeley and its contributors. * 4. Neither the name of the University nor the names of its contributors *    may be used to endorse or promote products derived from this software *    without specific prior written permission. * * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF * SUCH DAMAGE. * * $Header: /home/sourceTree/src/sctp/FreeBsdKern/netinet43/sctp.h,v 1.3 2001/04/30 17:57:10 randall Exp $ */#ifndef _NETINET_SCTP_H_#define _NETINET_SCTP_H_/* * SCTP protocol - RFC2960. *  */#define SCTP_DATA		0x00#define SCTP_INITIATION		0x01#define SCTP_INITIATION_ACK	0x02#define SCTP_SELECTIVE_ACK	0x03 #define SCTP_HEARTBEAT_REQUEST	0x04	#define SCTP_HEARTBEAT_ACK	0x05#define SCTP_ABORT_ASSOCIATION	0x06#define SCTP_SHUTDOWN		0x07#define SCTP_SHUTDOWN_ACK	0x08#define SCTP_OPERATION_ERR	0x09#define SCTP_COOKIE		0x0a#define SCTP_COOKIE_ACK         0x0b#define SCTP_ECN_ECHO		0x0c#define SCTP_ECN_CWR		0x0d#define SCTP_SHUTDOWN_COMPLETE	0x0e#define SCTP_FORWARD_CUM_TSN    0xc0#define SCTP_RELIABLE_CNTL      0xc1#define SCTP_RELIABLE_CNTL_ACK  0xc2/* ABORT and SHUTDOWN COMPLETE FLAG */#define SCTP_HAD_NO_TCB		0x01/* Modulo number for Adler32 csum */#define SCTP_ADLER32_BASE 65521 struct sctphdr {  u_short source;		/* source port */  u_short destination;		/* destination port */  u_int verificationTag;	/* Tag of inbound packet */  u_int adler32;		/* Adler32 C-Sum */};struct sctpChk{  u_char chunkID;  u_char chunkFlg;  u_short chunkLength;};struct sctpAbort{  struct sctphdr mh;  struct sctpChk uh;};#endif