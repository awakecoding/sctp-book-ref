/* SCTP reference Implementation Copyright (C) 2001 Cisco Systems Inc.This file is part of the SCTP reference ImplementationVersion:4.0.5$Header: /home/sourceTree/src/sctp/apps/primelist.c,v 1.7 2001/07/24 11:50:37 randall Exp $The SCTP reference implementation  is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published bythe Free Software Foundation; either version 2, or (at your option)any later version.the SCTP reference implementation  is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of                ************************MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theGNU Library General Public License for more details.You should have received a copy of the GNU Library General Public Licensealong with GNU CC; see the file COPYING.  If not, write tothe Free Software Foundation, 59 Temple Place - Suite 330,Boston, MA 02111-1307, USA.  Please send any bug reports or fixes you make to one of the following emailaddresses:rrs@cisco.comkmorneau@cisco.comAny bugs reported given to us we will try to fix... any fixes shared willbe incorperated into the next SCTP release.There are still LOTS of bugs in this code... I always run on the motto"it is a wonder any code ever works :)"*/#include <stdio.h>#include <stdlib.h>#include <math.h>intisPrime(int n){  int divisor;  int prime = 1;  int limit;  double input;  input = n * 1.0;  limit = (int)(sqrt(input));  for(divisor = 2;divisor <= limit;divisor++){    if((n % divisor) == 0){      prime = 0;      break;    }  }  return(prime);}intmain(int argc, char **argv){  long limit;  int i,cnt;  if(argc < 2){    printf("use %s limit\n",argv[0]);    return(-1);  }  limit = strtol(argv[1],NULL,0);  if(limit < 1){    printf("Sorry limit must be a positive int(%d)\n",(int)limit);	    return(-1);  }  cnt = 0;  printf("#ifndef __primeList_h__\n");  printf("#define __primeList_h__\n");  printf("int primeList[] = {\n");  for(i=0;i<limit;i++){    if(isPrime(i)){      if(cnt == 0){	printf("%d",i);      }else{	printf(",%d",i);      }      cnt++;      if((cnt % 16 == 0) && (cnt)){	printf("\n");      }    }  }  printf("\n};\n");  printf("#define PRIME_NUMBER_COUNT %d\n",cnt);  printf("#endif\n");  return(0);}