/* SCTP reference Implementation Copyright (C) 2001 Cisco And MotorolaThis file is part of the SCTP reference ImplementationVersion:4.0.5The SCTP reference implementation  is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published bythe Free Software Foundation; either version 2, or (at your option)any later version.the SCTP reference implementation  is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of                ************************MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theGNU Library General Public License for more details.You should have received a copy of the GNU Library General Public Licensealong with GNU CC; see the file COPYING.  If not, write tothe Free Software Foundation, 59 Temple Place - Suite 330,Boston, MA 02111-1307, USA.  Please send any bug reports or fixes you make to one of the following emailaddresses:rrs@cisco.comkmorneau@cisco.comqxie1@email.mot.comAny bugs reported given to us we will try to fix... any fixes shared willbe incorperated into the next SCTP release.There are still LOTS of bugs in this code... I always run on the motto"it is a wonder any code ever works :)"*/#include <stdlib.h>#include <stdio.h>#include <signal.h>#include <strings.h>#ifdef LINUX#include <stropts.h>#endif#include <poll.h>#include <errno.h>#include <setjmp.h>#include <string.h>#include <sys/types.h>#include <sys/socket.h>#include <unistd.h>#include <sys/time.h>#include <netinet/in.h>#include <arpa/inet.h>#include "sctp.h"#include "sctpConstants.h"#include "sctpStructs.h"#include "md5global.h"#include "md5.h"intmain(){  MD5_CTX ctx;  unsigned char message[1000002];  unsigned char digestBuf[100];  int notDone,at,i;  notDone = 1;  at = 1000000;  memset(message,0,sizeof(message));  for(i=0;i<1000000;i++){    message[i] = 'a';  }  if(at){    /* process the block */    printf("Processing a message of %d length\n",at);    MD5Init(&ctx);    MD5Update(&ctx, message, at);    MD5Final (digestBuf, &ctx);    printf("Digest buffer is as follows\n");    for(i=0;i<16;i++){      printf("%2.2x ",digestBuf[i]);    }    printf("\n");  }else{    printf("No message specified\n");  }  return(0);}