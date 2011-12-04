/*
 *  eyeserver.h
 *  maptiler
 *
 *  Created by James Mardell on 13/12/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include "time.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

#ifndef _INC_EYESTRUCTS
	#define _INC_EYESTRUCTS
	typedef struct eye_data_type {
		int bGazeVectorFound;
		int iIGaze;
		int iJGaze;
		float fPupilRadiusMm;
		float fXEyeballOffsetMm;
		float fYEyeballOffsetMm;
		float fFocusRangeImageTime;
		float fFocusRangeOffsetMm;
		float fLensExtOffsetMm;
		unsigned long ulCameraFieldCount;
		double dPentiumTSCMicroSec;
		int iSpacebar;
	} eye_data;

	typedef struct eye_server_argtype {
		char *client_hostname;
		unsigned int port;
		int *terminate;
		int *logfile_ready;
		FILE *logfile;
		volatile int *tile;
		unsigned int *spacebar;
		eye_data *data;
	} eye_server_args;
#endif // _INC_EYESTRUCTS

int eyeServer(eye_server_args *);
int openLogfile(FILE **, char *);
void closeLogfile(FILE **);
int writeLogfile(FILE **, eye_data, unsigned int, int);