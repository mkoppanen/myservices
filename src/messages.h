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

#ifndef __MYSERVICES_MESSAGES_H__
# define __MYSERVICES_MESSAGES_H__

#include <time.h>

#define LOGGER_OUT(_log_file, _x, ...) { \
	time_t my_time;\
	char _buf[26]; \
	time(&my_time); \
	ctime_r(&my_time, _buf); \
	fprintf(_log_file, "[%24.24s] [%s] ", _buf, _x); \
	fprintf(_log_file, __VA_ARGS__); \
	fputs("\n", _log_file); \
	fflush(_log_file); \
}

#define LOG_FATAL(_log_file, ...) { \
	LOGGER_OUT(_log_file, "FATAL", __VA_ARGS__); \
}

#define LOG_ERR(_log_file, ...) { \
	LOGGER_OUT(_log_file, "ERROR", __VA_ARGS__); \
}

#define LOG_WARN(_log_file, ...) { \
	LOGGER_OUT(_log_file, "WARN", __VA_ARGS__); \
}

#define LOG_INFO(_log_file, ...) { \
	LOGGER_OUT(_log_file, "INFO", __VA_ARGS__); \
}

#endif /* __MYSERVICES_MESSAGES_H__ */
