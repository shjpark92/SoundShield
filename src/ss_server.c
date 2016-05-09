#include <pthread.h>
#include "libwebsockets.h"
#include "ss_server.h"
#include "ss_api.h"

/* http server gets files from this path */
#define LOCAL_RESOURCE_PATH "/html"
#define MAX_MESSAGE_QUEUE   32

char *RESOURCE_PATH = LOCAL_RESOURCE_PATH;

int DEBUG_LEVEL = 7;
int PORT = 7681;

struct lws_context *context;
struct lws_plat_file_ops fops_plat;
struct serveable {
    const char *urlpath;
    const char *mimetype;
};
struct a_message {
    void *payload;
    size_t len;
};

volatile int force_exit = 0;
int close_testing;
int max_poll_elements;

#ifdef EXTERNAL_POLL
struct lws_pollfd *pollfds;
int *fd_lookup;
int count_pollfds;
#endif

/*
 * This mutex lock protects code that changes or relies on wsi list outside of
 * the service thread.  The service thread will acquire it when changing the
 * wsi list and other threads should acquire it while dereferencing wsis or
 * calling apis like lws_callback_on_writable_all_protocol() which
 * use the wsi list and wsis from a different thread context.
 */
pthread_mutex_t lock_established_conns;

/*
 * multithreaded version - protect wsi lifecycle changes in the library
 * these are called from protocol 0 callbacks
 */
void test_server_lock(int care) {
    if (care)
        pthread_mutex_lock(&lock_established_conns);
}
void test_server_unlock(int care) {
    if (care)
        pthread_mutex_unlock(&lock_established_conns);
}

enum SS_Protocols {
    /* always first */
    PROTOCOL_HTTP = 0,
    PROTOCOL_LWS_MIRROR,
    
    /* always last */
    DEMO_PROTOCOL_COUNT
};

/* list of supported protocols and callbacks */

static struct lws_protocols protocols[] = {
    /* first protocol must always be HTTP handler */
    {
        "http-only",        /* name */
        callback_http,      /* callback */
        sizeof (struct per_session_data__http), /* per_session_data_size */
        0,          /* max frame size / rx buffer */
    },
    {
        "fft-plot-protocol",    /* name */
        callback_fft_plot,      /* callback */
        sizeof (struct per_session_data__fft_plot), /* per_session_data_size */
        128,          /* max frame size / rx buffer */
    },
    { NULL, NULL, 0, 0 } /* terminator */
};

/* this shows how to override the lws file operations.  You don't need
 * to do any of this unless you have a reason (eg, want to serve
 * compressed files without decompressing the whole archive)
 */
static lws_filefd_type test_server_fops_open(struct lws *wsi, const char *filename, unsigned long *filelen, int flags) {
    lws_filefd_type n;

    /* call through to original platform implementation */
    n = fops_plat.open(wsi, filename, filelen, flags);
    lwsl_notice("%s: opening %s, ret %ld, len %lu\n", __func__, filename, (long)n, *filelen);
    return n;
}

/* Multithreaded server */
void *thread_lws_mirror(void *threadid) {
   while (!force_exit) {
        /*
         * this lock means wsi in the active list cannot
         * disappear underneath us, because the code to add and remove
         * them is protected by the same lock
         */
        pthread_mutex_lock(&lock_established_conns);
        lws_callback_on_writable_all_protocol(context, &protocols[PROTOCOL_LWS_MIRROR]);
        pthread_mutex_unlock(&lock_established_conns);
        usleep(100000);
    }

    pthread_exit(NULL);
}

void sighandler(int sig) {
    force_exit = 1;
    lws_cancel_service(context);
}

static const struct lws_extension exts[] = {
    {
        "permessage-deflate",
        lws_extension_callback_pm_deflate,
        "permessage-deflate; client_no_context_takeover; client_max_window_bits"
    },
    {
        "deflate-frame",
        lws_extension_callback_pm_deflate,
        "deflate_frame"
    },
    { NULL, NULL, NULL /* terminator */ }
};

static struct option options[] = {
    { "help",	no_argument,		NULL, 'h' },
    { "debug",	required_argument,	NULL, 'd' },
    { "port",	required_argument,	NULL, 'p' },
    { "ssl",	no_argument,		NULL, 's' },
    { "allow-non-ssl",	no_argument,	NULL, 'a' },
    { "interface",  required_argument,	NULL, 'i' },
    { "closetest",  no_argument,		NULL, 'c' },
    { "libev",  no_argument,		NULL, 'e' },
#ifndef LWS_NO_DAEMONIZE
    { "daemonize", 	no_argument,		NULL, 'D' },
#endif
    { "RESOURCE_PATH", required_argument,	NULL, 'r' },
    { NULL, 0, 0, 0 }
};

const char * get_mimetype(const char *file) {
    int n = strlen(file);

    if(n < 5)
        return NULL;

    if(!strcmp(&file[n - 3], ".js")) {
        return "application/x-javascript";
    }
    if(!strcmp(&file[n - 4], ".css")) {
        return "text/css";
    }
    if(!strcmp(&file[n - 4], ".ico")) {
        return "image/x-icon";
    }
    if(!strcmp(&file[n - 4], ".png")) {
        return "image/png";
    }
    if(!strcmp(&file[n - 5], ".html")) {
        return "text/html";
    }
    if(!strcmp(&file[n - 5], "4.2.0")) {
        return "*/*";
    }
    return "*/*";
}

void SS_Server_DumpHandShake(struct lws *webSocketInstance) {
    int token = 0;
    char buffer[256];
    const unsigned char *tokenPtr;
    do {
        tokenPtr = lws_token_to_string(token);
        if (!tokenPtr) {
            token++;
            continue;
        }
        if (!lws_hdr_total_length(webSocketInstance, token)) {
            token++;
            continue;
        }
        lws_hdr_copy(webSocketInstance, buffer, sizeof buffer, token);
        fprintf(stderr, "    %s = %s\n", (char *)tokenPtr, buffer);
        token++;
    } while (tokenPtr);
}

int openServer() {
    pthread_t pthread_mirror, pthread_service[32];
    struct lws_context_creation_info info;
    unsigned int ms, oldms = 0;
    const char *iface = NULL;
    char interface_name[128] = "";
    char certificationPath[1024];
    char keyPath[1024];
    char resourcePath[1024];
    int threads = 1;
    int useSSL = 0;
    int opts = 0;
    int n = 0;
    void *retval;

#ifndef LWS_NO_DAEMONIZE
    int daemonize = 0;
#endif
    
    /*
     * take care to zero down the info struct, he contains random garbaage
     * from the stack otherwise
     */
    memset(&info, 0, sizeof info);
    info.port = PORT;

    pthread_mutex_init(&lock_established_conns, NULL);
    signal(SIGINT, sighandler);
    
    /* tell the library what debug level to emit and to send it to syslog */
    lws_set_log_level(DEBUG_LEVEL, lwsl_emit_syslog);

    /* Get resource path */
    if (getcwd(resourcePath, sizeof(resourcePath)) != NULL) {
        fprintf(stdout, "Current working dir: %s\n", resourcePath);
    } 
    else {
        perror("getcwd() error");
        return -1;
    }
    strcat(resourcePath, RESOURCE_PATH);
    RESOURCE_PATH = resourcePath;
    printf("Using resource path \"%s\"\n", RESOURCE_PATH);
    
#ifdef EXTERNAL_POLL
    max_poll_elements = getdtablesize();
    pollfds = malloc(max_poll_elements * sizeof (struct lws_pollfd));
    fd_lookup = malloc(max_poll_elements * sizeof (int));
    if (pollfds == NULL || fd_lookup == NULL) {
        lwsl_err("Out of memory pollfds=%d\n", max_poll_elements);
        return -1;
    }
#endif
    
    info.iface = iface;
    info.protocols = protocols;
    info.extensions = exts;
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    
    if (useSSL) {
        if (strlen(RESOURCE_PATH) > sizeof(certificationPath) - 32) {
            lwsl_err("resource path too long\n");
            return -1;
        }
        sprintf(certificationPath, "%s/libwebsockets-test-server.pem", RESOURCE_PATH);
        if (strlen(RESOURCE_PATH) > sizeof(keyPath) - 32) {
            lwsl_err("resource path too long\n");
            return -1;
        }
        sprintf(keyPath, "%s/libwebsockets-test-server.key.pem", RESOURCE_PATH);
        info.ssl_cert_filepath = certificationPath;
        info.ssl_private_key_filepath = keyPath;
    }

    info.gid = -1;
    info.uid = -1;
    info.options = opts | LWS_SERVER_OPTION_VALIDATE_UTF8;
    info.max_http_header_pool = 1;
    context = lws_create_context(&info);
    if (context == NULL) {
        lwsl_err("Server failed to initialize!\n");
        return -1;
    }
    
    /* start the dumb increment thread */
    n = pthread_create(&pthread_mirror, NULL, thread_lws_mirror, 0);
    if (n) {
        lwsl_err("Unable to create dumb thread\n");
        goto done;
    }

    /* stash original platform fops */
    fops_plat = *(lws_get_fops(context));

    /* override the active fops */
    lws_get_fops(context)->open = test_server_fops_open;
    
    n = 0;
    while (n >= 0 && !force_exit) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
#ifdef EXTERNAL_POLL
        printf("ifdef EXTERNAL POLL");
        /*
         * this represents an existing server's single poll action
         * which also includes libwebsocket sockets
         */
        n = poll(pollfds, count_pollfds, 50);
        if (n < 0) continue;
        if (n)
            for (n = 0; n < count_pollfds; n++)
                if (pollfds[n].revents)
                /*
                 * returns immediately if the fd does not
                 * match anything under libwebsockets
                 * control
                 */
                    if (lws_service_fd(context, &pollfds[n]) < 0)
                        goto done;
#else
        /*
         * If libwebsockets sockets are all we care about,
         * you can use this api which takes care of the poll()
         * and looping through finding who needed service.
         *
         * If no socket needs service, it'll return anyway after
         * the number of ms in the second argument.
         */
        n = lws_service(context, 50);
#endif
    }
done:
    lws_context_destroy(context);
    pthread_mutex_destroy(&lock_established_conns);
    lwsl_notice("Server exited cleanly\n");
    return 0;
}

int callback_http(struct lws *webSocketInstance, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    struct per_session_data__http *pss = (struct per_session_data__http *) user;
    struct timeval tv;
    unsigned long amount, file_len;
    const char *contentType;
    static unsigned char buffer4096[4096];
    char buffer256[256];
    char buffer64[64];
    char leaf_path[1024];
    char *other_headers;
    unsigned char *end;
    unsigned char *p;
    int n, m, headerIndex = 0;
    
#ifdef EXTERNAL_POLL
    struct lws_pollargs *pa = (struct lws_pollargs *)in;
#endif
    
    switch (reason) {
        case LWS_CALLBACK_HTTP:
            /* SS_Server_DumpHandShake(webSocketInstance); */
            
            /* dump the individual URI Arg parameters */
            while (lws_hdr_copy_fragment(webSocketInstance, buffer256, sizeof(buffer256),
                                         WSI_TOKEN_HTTP_URI_ARGS, headerIndex) > 0) {
                lwsl_info("URI Arg %d: %s\n", ++headerIndex, buffer256);
            }
            
            if (len < 1) {
                lws_return_http_status(webSocketInstance, HTTP_STATUS_BAD_REQUEST, NULL);
                goto try_to_reuse;
            }
            /* if a legal POST URL, let it continue and accept data */
            if (lws_hdr_total_length(webSocketInstance, WSI_TOKEN_POST_URI)) {
                return 0;
            }
            if(strchr((const char*)in + 1, '/')) {
                if (strcmp(in, "/")) {
                    if (*((const char *)in) != '/') {
                        strcat(buffer256, "/");
                    }
                    strncat(buffer256, in, sizeof(buffer256) - strlen(RESOURCE_PATH));
                } 
                else { /* default file to serve */
                    strcat(buffer256, in);
                }

                buffer256[sizeof(buffer256) - 1] = '\0';
                sprintf(leaf_path, "%s%s", RESOURCE_PATH, buffer256);
                p = buffer4096 + LWS_PRE;
                end = p + sizeof(buffer4096) - LWS_PRE;
                pss->fd = lws_plat_file_open(webSocketInstance, leaf_path, &file_len, LWS_O_RDONLY);
                contentType = get_mimetype(buffer256);

                if (!contentType) {
                    lwsl_err("Unknown content-type for %s\n", buffer256);
                    lws_return_http_status(webSocketInstance, HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, NULL);
                    return -1;
                }
                if (pss->fd == LWS_INVALID_FILE) {
                        return -1;
                }
                if (lws_add_http_header_status(webSocketInstance, 200, &p, end)) {
                    return 1;
                }
                if (lws_add_http_header_by_token(webSocketInstance, WSI_TOKEN_HTTP_SERVER,
                                                 (unsigned char *)"libwebsockets", 13, &p, end)) {
                    return 1;
                }
                if(strncmp(contentType, "*/*", 3) == 0) {
                    if (lws_add_http_header_by_token(webSocketInstance, WSI_TOKEN_HTTP_CONTENT_TYPE,
                                                     (unsigned char *)contentType, 4, &p, end)) {
                        return 1;
                    }
                } else {
                    if(strncmp(contentType, "application/x-javascript", 24) == 0)
                    {
                        if (lws_add_http_header_by_token(webSocketInstance, WSI_TOKEN_HTTP_CONTENT_TYPE,
                                                         (unsigned char *)contentType, 24, &p, end)) {
                            return 1;
                        }
                    }
                    else
                    {
                        if (lws_add_http_header_by_token(webSocketInstance, WSI_TOKEN_HTTP_CONTENT_TYPE,
                                     (unsigned char *)contentType, 9, &p, end)) {
                            return 1;
                        }
                    }
                }
                if (lws_add_http_header_content_length(webSocketInstance, file_len, &p, end)) {
                    return 1;
                }
                if (lws_finalize_http_header(webSocketInstance, &p, end)) {
                    return 1;
                }
                n = lws_write(webSocketInstance, buffer4096 + LWS_PRE, p - (buffer4096 + LWS_PRE), LWS_WRITE_HTTP_HEADERS);
                if (n < 0) {
                    lws_plat_file_close(webSocketInstance, pss->fd);
                    return -1;
                }
                lws_callback_on_writable(webSocketInstance);
                break;
                printf("%s",buffer256);
                other_headers = NULL;
                n = 0;
                if (!strcmp((const char *)in, "/") && !lws_hdr_total_length(webSocketInstance, WSI_TOKEN_HTTP_COOKIE)) {
                    /* this isn't very unguessable but it'll do for us */
                    gettimeofday(&tv, NULL);
                    n = sprintf(buffer64, "test=SOUNDSHIELD_%u_%u_COOKIE;Max-Age=360000",
                                (unsigned int)tv.tv_sec, (unsigned int)tv.tv_usec);
                    p = (unsigned char *)buffer256;
                    if (lws_add_http_header_by_name(webSocketInstance, (unsigned char *)"set-cookie:",
                                                    (unsigned char *)buffer64, n, &p,
                                                    (unsigned char *)buffer256 + sizeof(buffer256))) {
                        return 1;
                    }
                    n = (char *)p - buffer256;
                    other_headers = buffer256;
                }
                n = lws_serve_http_file(webSocketInstance, buffer256, contentType, other_headers, n);
                if (n < 0 || ((n > 0) && lws_http_transaction_completed(webSocketInstance))) {
                    return -1; /* error or can't reuse connection: close the socket */
                }
                break;
            }
            /* if not, send a file the easy way */
            strcpy(buffer256, RESOURCE_PATH);
            if (strcmp(in, "/")) {
                if (*((const char *)in) != '/') {
                    strcat(buffer256, "/");
                }
                strncat(buffer256, in, sizeof(buffer256) - strlen(RESOURCE_PATH));
            } else { /* default file to serve */
                strcat(buffer256, "/index.html");
            }
            buffer256[sizeof(buffer256) - 1] = '\0';
            /* refuse to serve files we don't understand */
            contentType = get_mimetype(buffer256);
            if (!contentType) {
                lwsl_err("Unknown content-type for %s\n", buffer256);
                lws_return_http_status(webSocketInstance, HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, NULL);
                return -1;
            }
            /* demonstrates how to set a cookie on / */
            other_headers = NULL;
            n = 0;
            if (!strcmp((const char *)in, "/") && !lws_hdr_total_length(webSocketInstance, WSI_TOKEN_HTTP_COOKIE)) {
                /* this isn't very unguessable but it'll do for us */
                gettimeofday(&tv, NULL);
                n = sprintf(buffer64, "test=LWS_%u_%u_COOKIE;Max-Age=360000", (unsigned int)tv.tv_sec, (unsigned int)tv.tv_usec);
                p = (unsigned char *)leaf_path;
                if (lws_add_http_header_by_name(webSocketInstance, (unsigned char *)"set-cookie:",
                                                (unsigned char *)buffer64, n, &p,
                                                (unsigned char *)leaf_path + sizeof(leaf_path))) {
                    return 1;
                }
                n = (char *)p - leaf_path;
                other_headers = leaf_path;
            }
            n = lws_serve_http_file(webSocketInstance, buffer256, contentType, other_headers, n);
            if (n < 0 || ((n > 0) && lws_http_transaction_completed(webSocketInstance))) {
                return -1; /* error or can't reuse connection: close the socket */
            }
            /*
             * notice that the sending of the file completes asynchronously,
             * we'll get a LWS_CALLBACK_HTTP_FILE_COMPLETION callback when
             * it's done
             */
            break;
        case LWS_CALLBACK_HTTP_BODY:
            strncpy(buffer256, in, 20);
            buffer256[20] = '\0';
            if (len < 20) {
                buffer256[len] = '\0';
            }
            lwsl_notice("LWS_CALLBACK_HTTP_BODY: %s... len %d\n", (const char *)buffer256, (int)len);
            break;
        case LWS_CALLBACK_HTTP_BODY_COMPLETION:
            lwsl_notice("LWS_CALLBACK_HTTP_BODY_COMPLETION\n");
            /* the whole of the sent body arrived, close or reuse the connection */
            lws_return_http_status(webSocketInstance, HTTP_STATUS_OK, NULL);
            goto try_to_reuse;
        case LWS_CALLBACK_HTTP_FILE_COMPLETION:
            goto try_to_reuse;
        case LWS_CALLBACK_HTTP_WRITEABLE:
            /*
             * we can send more of whatever it is we were sending
             */
            do {
                /* we'd like the send this much */
                n = sizeof(buffer4096) - LWS_PRE;
                /* but if the peer told us he wants less, we can adapt */
                m = lws_get_peer_write_allowance(webSocketInstance);
                /* -1 means not using a protocol that has this info */
                if (m == 0) {
                /* right now, peer can't handle anything */
                    goto later;
                }
                if (m != -1 && m < n) {
                /* he couldn't handle that much */
                    n = m;
                }
                n = lws_plat_file_read(webSocketInstance, pss->fd, &amount, buffer4096 + LWS_PRE, n);
                /* problem reading, close conn */
                if (n < 0) { 
                    goto bail;
                }
                n = (int)amount;
                /* sent it all, close conn */
                if (n == 0) {
                    goto flush_bail;
                }
                /*
                 * To support HTTP2, must take care about preamble space
                 *
                 * identification of when we send the last payload frame
                 * is handled by the library itself if you sent a
                 * content-length header
                 */
                m = lws_write(webSocketInstance, buffer4096 + LWS_PRE, n, LWS_WRITE_HTTP);
                if (m < 0) {
                /* write failed, close conn */
                    goto bail;
                }
                /*
                 * http2 won't do this
                 */
                if (m != n) {
                /* partial write, adjust */
                    if (lws_plat_file_seek_cur(webSocketInstance, pss->fd, m - n) == (unsigned long)-1)
                        goto bail;
                }
                if (m) { /* while still active, extend timeout */
                    lws_set_timeout(webSocketInstance, PENDING_TIMEOUT_HTTP_CONTENT, 5);
                }
                /* if we have indigestion, let him clear it
                 * before eating more */
                if (lws_partial_buffered(webSocketInstance)) break;      
            } while (!lws_send_pipe_choked(webSocketInstance));
        later:
            lws_callback_on_writable(webSocketInstance);
            break;
        flush_bail:
            /* true if still partial pending */
            if (lws_partial_buffered(webSocketInstance)) {
                lws_callback_on_writable(webSocketInstance);
                break;
            }
            lws_plat_file_close(webSocketInstance, pss->fd);
            goto try_to_reuse;
        bail:
            lws_plat_file_close(webSocketInstance, pss->fd);
            return -1;
            /*
             * callback for confirming to continue with client IP appear in
             * protocol 0 callback since no websocket protocol has been agreed
             * yet.  You can just ignore this if you won't filter on client IP
             * since the default uhandled callback return is 0 meaning let the
             * connection continue.
             */
        case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
            /* if we returned non-zero from here, we kill the connection */
            break;
            /*
             * callbacks for managing the external poll() array appear in
             * protocol 0 callback
             */
            
        case LWS_CALLBACK_LOCK_POLL:
            /*
             * lock mutex to protect pollfd state
             * called before any other POLL related callback
             * if protecting wsi lifecycle change, len == 1
             */
            test_server_lock(len);
            break;
        case LWS_CALLBACK_UNLOCK_POLL:
            /*
             * unlock mutex to protect pollfd state when
             * called after any other POLL related callback
             * if protecting wsi lifecycle change, len == 1
             */
            test_server_unlock(len);
            break;
#ifdef EXTERNAL_POLL
        case LWS_CALLBACK_ADD_POLL_FD:
            if (count_pollfds >= max_poll_elements) {
                lwsl_err("LWS_CALLBACK_ADD_POLL_FD: too many sockets to track\n");
                return 1;
            }
            fd_lookup[pa->fd] = count_pollfds;
            pollfds[count_pollfds].fd = pa->fd;
            pollfds[count_pollfds].events = pa->events;
            pollfds[count_pollfds++].revents = 0;
            break;
        case LWS_CALLBACK_DEL_POLL_FD:
            if (!--count_pollfds) {
                break;
            }
            m = fd_lookup[pa->fd];
            /* have the last guy take up the vacant slot */
            pollfds[m] = pollfds[count_pollfds];
            fd_lookup[pollfds[count_pollfds].fd] = m;
            break;
        case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
            pollfds[fd_lookup[pa->fd]].events = pa->events;
            break;
#endif
        case LWS_CALLBACK_GET_THREAD_ID:
            /*
             * if you will call "lws_callback_on_writable"
             * from a different thread, return the caller thread ID
             * here so lws can use this information to work out if it
             * should signal the poll() loop to exit and restart early
             */
            /* return pthread_getthreadid_np(); */
            break;
        default:
            break;
    }
    return 0;
    /* if we're on HTTP1.1 or 2.0, will keep the idle connection alive */
try_to_reuse:
    if (lws_http_transaction_completed(webSocketInstance)) {
        return -1;
    }
    return 0;
}


