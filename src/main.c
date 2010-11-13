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

static bool g_terminate = false;
static bool g_sighup    = false;

/* {{{ static void signal_handler_terminate(int sig)
   Signal handler to indicate that we have received signal to terminate processing
*/
static void signal_handler_terminate(int sig)
{
	g_terminate = true;
}
/* }}} */

/* {{{ static void signal_handler_terminate(int sig)
   Signal handler to indicate that daemon needs to reload config
   TODO: not handled anywhere yet
*/
static void signal_handler_sighup(int sig)
{
	g_sighup = true;
}
/* }}} */

static bool reconnect_zookeeper(service_location_t *service);
static void cb_switch_state(zhandle_t *zh, int type, int state, const char *path, void *ctx);
static bool create_node_if_not_exists(service_location_t *service, int flags, const char *value, int value_len, const char *fmt, ...);
static bool connect_zookeeper(service_location_t *service, bool create_nodes);
static void create_service_nodes(service_location_t *service);

static bool connect_zookeeper(service_location_t *service, bool create_nodes)
{
	const char *host;
	int recv_timeout;

	host = iniparser_getstring(service->config, "zookeeper:host", "no_host");

	if (!strcmp(host, "no_host")) {
		LOG_FATAL(service->log_fd, "No zookeeper:host defined in configuration file");
		return false;
	}
	
	service->connected_state = false;
	recv_timeout = iniparser_getint(service->config, "zookeeper:recv_timeout", MYSERVICES_RECV_TIMEOUT);
	service->zk  = zookeeper_init(host, cb_switch_state, recv_timeout, 0, service, 0);
	
	if (!service->zk) {
		return false;
	}
	
	if (create_nodes == true) {
		create_service_nodes(service);
	}
	return true;
}

static void disconnect_zookeeper(service_location_t *service)
{
	zookeeper_close(service->zk);
	service->zk = NULL;
}

static void create_service_nodes(service_location_t *service)
{
	const char *parent = iniparser_getstring(service->config, "zookeeper:parent_node", MYSERVICES_PARENT_NODE);

	service->node_creation_state = NodeCreating;

	if (service->connected_state == false) {
		return;
	}

	/* If parent node creation fails that is really bad */
	if (false == create_node_if_not_exists(service, 0, 0, 0, "%s", parent)) {
		LOG_FATAL(service->log_fd, "Failed to create parent node for daemon");
		return;
	} else {
		int i;
		for (i = 0; i < service->info.count; i++) {

			if (false == create_node_if_not_exists(service, 0, 0, 0, "%s/%s", parent, service->info.services[i])) {
				LOG_ERR(service->log_fd, "Failed to create service node");
				continue;
			}

			if (false == create_node_if_not_exists(service, ZOO_EPHEMERAL, 
													service->info.addresses[i], strlen(service->info.addresses[i]), 
													"%s/%s/%s", parent, service->info.services[i], service->hostname)) {
				LOG_ERR(service->log_fd, "Failed to create service address node");
				continue;
			}
		}
	}
	
	if (service->node_creation_state == NodeCreating) {
		service->node_creation_state = NodeOK;
	}
}

static void cb_switch_state(zhandle_t *zh, int type, int state, const char *path, void *ctx) 
{
	service_location_t *service = (service_location_t *) ctx;
	
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			LOG_INFO(service->log_fd, "Changed to connected state");
			service->connected_state = true;
			create_service_nodes(service);
		}
		else if (state == ZOO_EXPIRED_SESSION_STATE || 
				 state == ZOO_CONNECTING_STATE) {
			LOG_INFO(service->log_fd, "Changed to disconnected state, trying to reconnect");
			service->connected_state = false;
			disconnect_zookeeper(service);
			connect_zookeeper(service, true);
		}
	}
	else if (type == ZOO_DELETED_EVENT) {
		char token[HOSTNAME_MAX + 1];
		snprintf(token, HOSTNAME_MAX + 1, "/%s", service->hostname);

		if (str_ends_with(path, token) == true) {
			create_service_nodes(service);
		}
	}
}

static bool create_node_if_not_exists(service_location_t *service, int flags, const char *value, int value_len, const char *fmt, ...)
{
	struct Stat stat;
	int status, retry_count;
	char node_key[512];
	va_list args;
	
	if (service->connected_state == false) {
		return false;
	}

	va_start(args, fmt);
	vsnprintf(node_key, 512, fmt, args);
	va_end(args);
	
	retry_count = 3;
	do {
		status = zoo_exists(service->zk, node_key, 1, &stat);
		retry_count++;
	} while (status == ZCONNECTIONLOSS && retry_count--);
		
	if (status == ZNONODE) {
		LOG_INFO(service->log_fd, "Creating node=[%s] value=[%s]", node_key, value);
		
		retry_count = 3;
		do {
			status = zoo_create(service->zk, node_key, value, value_len, &ZOO_OPEN_ACL_UNSAFE, flags, 0, 0);
			retry_count++;
		} while (status == ZCONNECTIONLOSS && retry_count--);
	}

	return (status == ZOK) ? true : false;
}

static bool run_myservices(service_location_t *service)
{
	int j, period = iniparser_getint(service->config, "zookeeper:wait_period", MYSERVICES_WAIT_PERIOD); 
	const char *parent;
	
	if (!service->info.count) {
		LOG_INFO(service->log_fd, "Not advertising any services");
	}
	
	/* On SIGINT and SIGTERM shutdown */
	signal(SIGINT, signal_handler_terminate);
	signal(SIGTERM, signal_handler_terminate);

	/* On SIGHUP reload config */
	signal(SIGHUP, signal_handler_sighup);
	
	j = 0;
	do {

		if (g_terminate == true) {
			LOG_INFO(service->log_fd, "Received signal. Terminating..");
			break;
		}
		
		if (g_sighup == true) {
			// TODO: reload configuration
		}

		j++;
		if (j % period == 0 || j > period) {
			if (service->connected_state == true) {
				create_service_nodes(service);
				j = 0;
			}
		}
		sleep(1);
		
	} while(true);
	
	return true;
}

int main(int argc, char *argv[]) 
{
	service_location_t service = {0};
	const char *host, *parent, *pid_file, *log_file;
	int i;
	
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <config file>", argv[0]);
		exit(1);
	}
	
	/* Parse the configuration */
	if (strlen(argv[1]) >= PATH_MAX) {
		LOG_WARN(stderr, "The config file path exceeds PATH_MAX, possibly truncating");
	}
	strncpy(service.config_file, argv[1], PATH_MAX);

	if (load_config(&service) == false) {
		LOG_FATAL(stderr, "Failed to load configuration(%s): %s", service.config_file, strerror(errno));
		exit(1);
	}
	
	/* Open log file */
	log_file = iniparser_getstring(service.config, "main:log_file", MYSERVICES_LOG_FILE);
	
	if (strlen(log_file) >= PATH_MAX) {
		LOG_WARN(stderr, "The log_file path exceeds PATH_MAX, possibly truncating");
	}
	strncpy(service.log_file, log_file, PATH_MAX);
	
	if (open_log_file(&service, service.log_file) == false) {
		LOG_FATAL(stderr, "Failed to open log file: %s", log_file, strerror(errno));
		exit(1);
	}

	signal(SIGINT, SIG_IGN);
	signal(SIGKILL, SIG_IGN);

	if (fork() == 0) {
		int code;
		uid_t uid;
		gid_t gid;
		
		signal(SIGINT, SIG_DFL);
		signal(SIGKILL, SIG_DFL);
		
		if (gethostname(service.hostname, HOSTNAME_MAX)) {
			LOG_FATAL(service.log_fd, "Failed to gethostname: %s", strerror(errno));
			exit(1);
		}

		/* Initialize service locations */
		service_info_init(&(service.info));

		if (service_info_parse(&service, &(service.info)) == false) {
			LOG_FATAL(service.log_fd, "Failed to parse service information");
			exit(1);
		}
		
		if (resolve_uid_and_gid(&service, &uid, &gid) == false) {
			LOG_FATAL(service.log_fd, "Failed to resolve user/group");
			exit(1);
		}
		
		pid_file = iniparser_getstring(service.config, "main:pid_file", MYSERVICES_PID_FILE);
		if (write_pid_file(&service, pid_file, uid, gid) == false) {
			LOG_FATAL(service.log_fd, "Failed to write pid file");
			exit(1);
		}

		// Drop privileges and chown pid to the current user
		if (drop_privileges(uid, gid) == false) {
			LOG_FATAL(service.log_fd, "Failed to lower privileges: %s", strerror(errno));
			exit(1);
		}

		fclose(stdin);
		fclose(stdout);
		
		do {
			/* ZooKeeper client floods the logs with messages if given a chance */
			zoo_set_debug_level(0);
			
			if (connect_zookeeper(&service, true) == false) {
				LOG_FATAL(service.log_fd, "Failed to connect");
				break;
			}

			if (false == run_myservices(&service)) {
				LOG_FATAL(service.log_fd, "Failed to run MyServices daemon");
				break;
			}
		
		} while (0);
		
		/* Unlink pid-file */
		if (unlink(pid_file) == -1) {
			LOG_FATAL(service.log_fd, "Failed to remove pid file (%s): %s", pid_file, strerror(errno));
		}
		service_info_deinit(&(service.info));
		
		if (service.config)
			iniparser_freedict(service.config);
		
		if (service.zk)
			zookeeper_close(service.zk);
		
		LOG_INFO(service.log_fd, "Terminating MyServices daemon..");
		fclose(service.log_fd);
		exit(0);
	} else {
		signal(SIGINT, SIG_DFL);
		signal(SIGKILL, SIG_DFL);
		exit(0);
	}

	return 0;
}
