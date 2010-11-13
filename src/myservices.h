/*
+-----------------------------------------------------------------------------------+
|  location, service locator for zookeeper                                          |
|  Copyright (c) 2010, Mikko Koppanen <mikko.koppanen@gmail.com>                    |
|  All rights reserved.                                                             |
+-----------------------------------------------------------------------------------+
|  Redistribution and use in source and binary forms, with or without               |
|  modification, are permitted provided that the following conditions are met:      |
|     * Redistributions of source code must retain the above copyright              |
|       notice, this list of conditions and the following disclaimer.               |
|     * Redistributions in binary form must reproduce the above copyright           |
|       notice, this list of conditions and the following disclaimer in the         |
|       documentation and/or other materials provided with the distribution.        |
|     * Neither the name of the copyright holder nor the                            |
|       names of its contributors may be used to endorse or promote products        |
|       derived from this software without specific prior written permission.       |
+-----------------------------------------------------------------------------------+
|  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND  |
|  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED    |
|  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           |
|  DISCLAIMED. IN NO EVENT SHALL MIKKO KOPPANEN BE LIABLE FOR ANY                   |
|  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES       |
|  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;     |
|  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND      |
|  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       |
|  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS    |
|  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                     |
+-----------------------------------------------------------------------------------+
*/

#ifndef __MYSERVICES_H__
# define __MYSERVICES_H__

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "foreign/iniparser.h"
#include <zookeeper.h>

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
typedef bool int;
# define true 1
# define false 0
#endif

#define HOSTNAME_MAX 256

typedef enum {
	NodeDeleted,
	NodeCreating,
	NodePeriodicCheck,
	NodeOK
} node_state;


/* Service info */
typedef struct _service_info_t {
	char **services;
	char **addresses;
	int count;
} service_info_t;

typedef struct _service_location_t {
	zhandle_t *zk;
	dictionary *config;
	char hostname[HOSTNAME_MAX];
	
	service_info_t info;
	FILE *log_fd;
	bool connected_state;
	node_state node_creation_state;
	
	char config_file[PATH_MAX];
	char log_file[PATH_MAX];
	
} service_location_t;


void service_info_init(service_info_t *info);

void service_info_deinit(service_info_t *info);

bool service_info_add(service_info_t *info, const char *service_name, const char *service_address);

bool service_info_parse(service_location_t *service, service_info_t *info);


/* Config defaults */
#define MYSERVICES_PID_FILE "/var/run/myservices.pid"

#define MYSERVICES_PARENT_NODE "/service-location"

#define MYSERVICES_RECV_TIMEOUT 5000

#define MYSERVICES_DEFAULT_SERVICE "dummy"

#define MYSERVICES_WAIT_PERIOD 30

#define MYSERVICES_LOG_FILE "/var/log/myservices.log"

#endif /* __MYSERVICES_H__ */
