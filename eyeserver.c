/*
 *  eyeserver.c
 *  maptiler
 *
 *  Created by James Mardell on 13/12/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include "eyeserver.h"

int eyeServer(eye_server_args *args)
{
	eye_data *eye = args->data;
	int *terminate = args->terminate;
	const int expected_elements = 10;
	
	struct sockaddr_in socket_input;
	int socket_pointer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (socket_pointer == -1) {
		perror("Error creating socket");
		return 1;
	}
	
	memset(&socket_input, 0, sizeof(socket_input) );
	
	socket_input.sin_family = AF_INET;
	socket_input.sin_port = htons(args->port);
	socket_input.sin_addr.s_addr = INADDR_ANY;
	
	int socket_opt;
	setsockopt(1, SOL_SOCKET, SO_REUSEADDR, &socket_opt, sizeof(socket_opt) );
	
	if (bind(socket_pointer, (const void *)&socket_input, sizeof(socket_input) ) == -1) {
		perror("Error binding socket");
		return 1;
	}
	
	if (listen(socket_pointer, 10) == -1) {
		perror("Error listening to socket");
		return 1;
	}
	
	fprintf(stdout, "Beginning server loop.\n");
	
	while (*terminate == 0) {
		int connection = accept(socket_pointer, NULL, NULL);
		
		if (connection <= 0) {
			perror("Error accepting connection");
			close(socket_pointer);
			return 1;
		}
		
		char buffer[128];
		eye_data results;
		//memset(buffer, 0, 256 - 1);
		int elements;
		
		fprintf(stdout, "Entering receiving loop.\n");
		
		// Main receiving loop
		while (recv(connection, buffer, 256, 0) > 0) {
			elements = sscanf(buffer, "%lu %i %i %i %f %f %f %f %f %i",
							  &results.ulCameraFieldCount,
							  &results.bGazeVectorFound,
							  &results.iIGaze,
							  &results.iJGaze,
							  &results.fPupilRadiusMm,
							  &results.fXEyeballOffsetMm,
							  &results.fYEyeballOffsetMm,
							  &results.fLensExtOffsetMm,
							  &results.fFocusRangeImageTime,
							  &results.iSpacebar
							  );
			
			if (elements != expected_elements) {
				fprintf(stderr, "Invalid data received by the eye server. Expected %d elements, but received %d.\n", expected_elements, elements);
				fprintf(stderr, "Received: %s\n", buffer);
				close(socket_pointer);
				return 1;
			}
			
			// Data assumed good
			*eye = results;
				
			if (*args->logfile_ready) {
				if (writeLogfile(&args->logfile, results, *args->spacebar, *args->tile) ) {
					perror("Error writing the logfile");
					shutdown(connection, SHUT_RDWR);
					close(connection);
					return 1;
				} 
			}
			
			// Thread termination breakout
			if (*terminate != 0) {
				fprintf(stderr, "Eye server thread terminated...\n");
				break;
			}
				
		} // main receiving loop
		
		fprintf(stderr, "Closing connection with client...\n");
		shutdown(connection, SHUT_RDWR);
		close(connection);
	} // while
	
	fprintf(stdout, "End of server.\n");
	return 0;
}
			

static int writeLogfileHeader(FILE *file)
{
	if (file == NULL) {
		fprintf(stderr, "Unable to open file for writing.\n");
		return 1;
	}
	
	// Calculate time and date
	time_t raw_time;
	struct tm *time_info;
	char time_string[128];
	
	time(&raw_time);
	time_info = localtime(&raw_time);
	
	strftime(time_string, 128, "Gazepoint Data File,  %H:%M:%S  %m/%d/%Y", time_info);
	
	// Output header
	fprintf(file, "%s\n", time_string);
	fprintf(file, "Scene Type: 1024 768\n");
	fprintf(file, "Raw Gazepoint Data (60 Hz Sampling Rate):\n");
	fprintf(file, "\n"); 
	fprintf(file, "Samp  Eye     Gazepoint  Pupil   Eyeball-Position  Focus Spacebar Stimuli\n");
	fprintf(file, "Indx Found    X      Y   Radius    X     Y     Z   Range  Pressed  Shown\n");
	fprintf(file, "     (t/f)  (pix)  (pix)  (mm)   (mm)  (mm)  (mm)   (mm)   (t/f)   (tile)\n");
	fprintf(file, "\n");
	
	return 0;
}

int openLogfile(FILE **file, char *path)
{
	*file = fopen(path, "w");
	
	if (*file == NULL) {
		fprintf(stderr, "Unable to open file '%s' for writing.\n", path);
		return 1;
	}
	
	if (writeLogfileHeader(*file) ) {
		fprintf(stderr, "Error writing logfile header to file '%s'.\n", path);
		return 1;
	}
	
	return 0;
}

void closeLogfile(FILE **file)
{
	fprintf(stderr, "Attempting to close logfile...\n");
	fclose(*file);
	*file = NULL;
	
	return;
}

int writeLogfile(FILE **file, eye_data data, unsigned int spacebar, int tile)
{
	if (*file == NULL) {
		fprintf(stderr, "Error writing sample %lu to logfile.\n", data.ulCameraFieldCount);
		return 1;
	}

	fprintf(*file, "%lu %d %d %d %4.2f %4.2f %4.2f %4.2f %4.2f %d %d\n", 
			data.ulCameraFieldCount,
			data.bGazeVectorFound,
			data.iIGaze,
			data.iJGaze,
			data.fPupilRadiusMm,
			data.fXEyeballOffsetMm,
			data.fYEyeballOffsetMm,
			data.fFocusRangeOffsetMm,
			data.fFocusRangeImageTime,
			data.iSpacebar,
			tile);
	
	return 0;
}