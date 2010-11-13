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

#include "myservices.h"
#include "messages.h"

void service_info_init(service_info_t *info) 
{
	info->services  = NULL;
	info->addresses = NULL;
	info->count     = 0;
}

void service_info_deinit(service_info_t *info) 
{
	if (info->count) {	
		int i;
	
		for (i = 0; i < info->count; i++) {
			free(info->services[i]);
			free(info->addresses[i]);
		}
		free(info->services);
		free(info->addresses);
	}
	info->services  = NULL;
	info->addresses = NULL;
	info->count     = 0;
}


bool service_info_add(service_info_t *info, const char *service_name, const char *service_address) 
{
	info->services  = realloc(info->services, (info->count + 1) * sizeof(char *));
	info->addresses = realloc(info->addresses, (info->count + 1) * sizeof(char *));
	
	info->services[info->count]  = strdup(service_name);
	info->addresses[info->count] = strdup(service_address);
	
	info->count++;
}


bool service_info_parse(service_location_t *service, service_info_t *info) 
{
	const char *p;
	char *ptr, *pch;
	int spaces;
	bool success = true;
	
	p   = iniparser_getstring(service->config, "services:provides", MYSERVICES_DEFAULT_SERVICE);
	ptr = (char *)p;
	pch = strtok(ptr, " ");

	while (pch) {
		char key[512];
		const char *service_address;
		
		snprintf(key, 512, "locations:%s", pch);
		service_address = iniparser_getstring(service->config, key, "no_address");
		
		if (!strcmp(service_address, "no_address")) {
			LOG_ERR(service->log_fd, "No locations:%s defined in configuration", pch);
			success = false;	
			break;
		}
		
		service_info_add(info, pch, service_address);
		pch = strtok(NULL, " ");
	}
	
	if (false == success) {
		service_info_deinit(info);
		return false;
	}
	return true;
}
