#ifndef _SS_SERVER_H_
#define _SS_SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include "lws_config.h"
#include "libwebsockets.h"

#ifdef _WIN32
#include <io.h>
#include "gettimeofday.h"
#else
#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>
#endif

extern int close_testing;
extern int max_poll_elements;

#ifdef EXTERNAL_POLL
extern struct lws_pollfd *pollfds;
extern int *fd_lookup;
extern int count_pollfds;
#endif

extern volatile int force_exit;
extern struct lws_context *context;
extern char *resource_path;

void test_server_lock(int care);
void test_server_unlock(int care);

#ifndef __func__
#define __func__ __FUNCTION__
#endif

struct per_session_data__http {
    lws_filefd_type fd;
};

struct per_session_data__lws_mirror {
    struct lws *wsi;
    int ringbuffer_tail;
};

struct per_session_data__volume {
    lws_filefd_type fd;
};

struct per_session_data__fft_plot {
    struct lws *wsi;
    int ringbuffer_tail;
};

int openServer();

extern int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user,
                         void *in, size_t len);

extern int callback_lws_mirror(struct lws *wsi, enum lws_callback_reasons reason,
                               void *user, void *in, size_t len);

extern int callback_fft_plot(struct lws *wsi, enum lws_callback_reasons reason,
                               void *user, void *in, size_t len);

extern void dump_handshake_info(struct lws *wsi);

#endif //_SS_SERVER_H_