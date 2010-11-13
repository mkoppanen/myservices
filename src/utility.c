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
#include "utility.h"

bool load_config(service_location_t *service)
{
	if (service->config) {
		iniparser_freedict(service->config);
		service->config = NULL;
	}
	service->config = iniparser_load(service->config_file);
	
	if (!service->config) {
		return false;
	}
	return true;
}

bool open_log_file(service_location_t *service, const char *log_file)
{	
	if (service->log_fd) {
		fclose(service->log_fd);
		service->log_fd = 0;
	}
	
	service->log_fd = fopen(log_file, "a+");
	
	if (!service->log_fd) {
		return false;
	}
	return true;
}

bool write_pid_file(service_location_t *service, const char *pid_file, uid_t uid, gid_t gid)
{
	int status, pid_str_len;
	char *path, *dir, pid_str[96];
	FILE *pid_fd;
	pid_t pid;

	status = access(pid_file, F_OK);
	
	if (!status) {
		LOG_ERR(service->log_fd, "Pid file %s exists", pid_file);
		return false;
	}
	
	path   = strdup(pid_file);
	dir    = dirname(path);
	status = access(dir, R_OK | X_OK | W_OK);
	free(path);

	if (status) {
		LOG_ERR(service->log_fd, "Directory %s is not writable or accessible: %s", pid_file, strerror(errno));
		return false;
	}

	pid_fd = fopen(pid_file, "w+");
	if (!pid_fd) {
		LOG_ERR(service->log_fd, "Failed to create pid file: %s", pid_file, strerror(errno));
		return false;
	}
	/* todo: Check that all was written */
	fprintf(pid_fd, "%u\n", getpid());
	fclose(pid_fd);
	
	if (chown(pid_file, uid, gid) == -1) {
		LOG_WARN(service->log_fd, "Failed to chown pid-file");
	}
	
	LOG_INFO(service->log_fd, "Wrote pid file into %s", pid_file);
	return true;
}

bool resolve_uid_and_gid(service_location_t *service, uid_t *uid, gid_t *gid) 
{
	const char *user, *group;
	struct passwd *res_user;
	struct group *res_group;
	
	user  = iniparser_getstring(service->config, "main:run_as_user", "myservices");
	group = iniparser_getstring(service->config, "main:run_as_group", "myservices");
	
	res_user = getpwnam(user);
	if (!res_user) {
		LOG_WARN(service->log_fd, "Could not find user '%s'", user);
		return false;
	}
	
	res_group = getgrnam(group);

	if (!res_group) {
		LOG_WARN(service->log_fd, "Could not find group '%s'", group);
		return false;
	}

	*uid = res_user->pw_uid;
	*gid = res_group->gr_gid;
	return true;
}


bool drop_privileges(uid_t uid, gid_t gid)
{
	if (setgid(gid) != 0) {
		return false;
	}

	if (setuid(uid) != 0) {
		return false;
	}
	return true;
}

bool str_ends_with(const char *str, const char *suffix) 
{
	size_t str_len = strlen(str);	
	size_t suffix_len = strlen(suffix);

	if (suffix_len > str_len) {
		return false;
	}

	if (strcmp(str + str_len - suffix_len, suffix)) {
		return false;
	}
	return true;
}
