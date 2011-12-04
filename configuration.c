/*
 *  configuration.c
 *  maptiler
 *
 *  Created by James Mardell on 08/12/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include "configuration.h"

static int parseNumber(struct config_t *config, const char *query)
{
	// Initialise empty setting buffer
	config_setting_t *setting = NULL;
	
	setting = config_lookup(config, query);
	
	if (!setting) {
		fprintf(stderr, "libconfig: Query '%s' returned error '%s' on line '%d' for file '%s'.\n",
				query, config_error_text(config), config_error_line(config), config_error_file(config) );
		config_destroy(config);
		exit(1);
	}
	
	return config_setting_get_int(setting);
}

static char parseChar(struct config_t *config, const char *query)
{
	// Initialise empty setting buffer
	config_setting_t *setting = NULL;
	
	setting = config_lookup(config, query);
	
	if (!setting) {
		fprintf(stderr, "libconfig: Query '%s' returned error '%s' on line '%d' for file '%s'.\n",
				query, config_error_text(config), config_error_line(config), config_error_file(config) );
		config_destroy(config);
		exit(1);
	}
	char result;
	
	strncpy(&result, config_setting_get_string(setting), 1);
	
	return result;
}

int parseExperimentMetadata(unsigned int *runs, int *participant, const char *path)
{
	// libconfig parameters
	struct config_t cfg;
	config_setting_t *setting = NULL;
	
	// Open and parse configuration file 'path'
	config_init(&cfg);
	if (config_read_file(&cfg, path) == CONFIG_FALSE) {
		fprintf(stderr, "libconfig: Error parsing '%s': %s on line %d.\n",
				config_error_file(&cfg), config_error_text(&cfg), config_error_line(&cfg) );
	
		config_destroy(&cfg);
		
		return 1;
	}
	
	// Extract participant number
	*participant = parseNumber(&cfg, "participant");
	
	// Extract number of runs
	setting = config_lookup(&cfg, "runs");
	if (!setting) {
		fprintf(stderr, "No runs entry found in '%s'.\n", path);
		config_destroy(&cfg);
		return 1;
	}
	*runs = config_setting_length(setting);
	
	// Clean-up and leave
	config_destroy(&cfg);
	
	return 0;
}

int parseExperimentRun(const unsigned int run, char *map, int *seg, const char *path)
{
	// libconfig parameters
	struct config_t config;
	char buffer[64];
	
	// Open and parse configuration file 'path'
	config_init(&config);
	if (config_read_file(&config, path) == CONFIG_FALSE) {
		fprintf(stderr, "libconfig: Error parsing '%s': %s on line %d.\n",
				config_error_file(&config), config_error_text(&config), config_error_line(&config) );
		
		config_destroy(&config);
		
		return 1;
	}
	
	// Create and run queries
	sprintf(buffer, "runs.[%d].map", run);
	*map = parseChar(&config, buffer);
	sprintf(buffer, "runs.[%d].seg", run);
	*seg = parseNumber(&config, buffer);
	
	// Clean-up and leave
	config_destroy(&config);
	
	return 0;
}