/*
 * Copyright (c) 2013, Red Hat
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.  Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials
 * provided with the distribution.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* For asprintf */
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yajl/yajl_tree.h>
#include "etcd-api.h"
#include "ghttp/ghttp.h"


#define DEFAULT_ETCD_PORT       4001
#define SL_DELIM                "\n\r\t ,;"

typedef struct {
        etcd_server     *servers;
} _etcd_session;

typedef struct {
        char            *key;
        char            *value;
        int             *index_in;      /* pointer so NULL can be special */
        int             index_out;      /* NULL would be meaningless */
} etcd_watch_t;

typedef size_t curl_callback_t (void *, size_t, size_t, void *);

const char      *value_path[]   = { "node", "value", NULL };
const char      *nodes_path[]   = { "node", "nodes", NULL };
const char      *entry_path[]   = { "key", NULL };

/*
 * We only call this in case where it should be safe, but gcc doesn't know
 * that so we use this to shut it up.
 */
static char *
MY_YAJL_GET_STRING (yajl_val x)
{
        char *y = YAJL_GET_STRING(x);

        return y ? y : (char *)"bogus";
}

#if defined(DEBUG)
void
print_curl_error (char *intro, CURLcode res)
{
        printf("%s: %s\n",intro,curl_easy_strerror(res));
}
#else
#define print_curl_error(intro,res)
#endif

 
etcd_session
etcd_open (etcd_server *server_list)
{
    _etcd_session   *session;

    session = malloc(sizeof(*session));
    if (!session) {
        return NULL;
    }

    /*
     * Some day we'll set up more persistent connections, and keep track
     * (via redirects) of which server is leader so that we can always
     * try it first.  For now we just push that to the individual request
     * functions, which do the most brain-dead thing that can work.
     */

    session->servers = server_list;
    return session;
}


void
etcd_close (etcd_session session)
{
    free(session);
}

/*
 * Normal yajl_tree_get is returning NULL for these paths even when I can
 * verify (in gdb) that they exist.  I suppose I could debug this for them, but
 * this is way easier.
 *
 * TBD: see if common distros are packaging a JSON library that isn't total
 * crap.
 */
static yajl_val
my_yajl_tree_get (yajl_val root, char const **path, yajl_type type)
{
        yajl_val        obj    = root;
        int             i;

        for (;;) {
                if (!*path) {
                        if (obj && (obj->type != type)) {
                                return NULL;
                        }
                        return obj;
                }
                if (obj->type != yajl_t_object) {
                        return NULL;
                }
                for (i = 0; /* nothing */; ++i) {
                        if (i >= obj->u.object.len) {
                                return NULL;
                        }
                        if (!strcmp(obj->u.object.keys[i],*path)) {
                                obj = obj->u.object.values[i];
                                ++path;
                                break;
                        }
                }
        }
}


/*
 * Looking directly at node->u.array seems terribly un-modular, but the YAJL
 * tree interface doesn't seem to have any exposed API for iterating over the
 * elements of an array.  I tried using yajl_tree_get with an index in the
 * path, either as a type-casted integer or as a string, but that didn't work.
 */
static char *
parse_array_response (yajl_val parent)
{
        size_t          i;
        yajl_val        item;
        yajl_val        value;
        char            *retval = NULL;
        char            *saved;
        yajl_val        node;

        node = my_yajl_tree_get(parent,nodes_path,yajl_t_array);
        if (!node) {
                return NULL;
        }

        for (i = 0; i < node->u.array.len; ++i) {
                item = node->u.array.values[i];
                if (!item) {
                        break;
                }
                value = my_yajl_tree_get(item,entry_path,yajl_t_string);
                if (!value) {
                        break;
                }
                if (retval) {
                        saved = retval;
                        retval = NULL;
                        (void)asprintf (&retval, "%s\n%s",
                                        saved, MY_YAJL_GET_STRING(value));
                        free(saved);
                }
                else {
                        retval = strdup(MY_YAJL_GET_STRING(value));
                }
                if (!retval) {
                        break;
                }
        }

        return retval;
}

static char *
parse_get_response(char *body)
{
        yajl_val node;
        yajl_val value;
        char     *json_data = NULL;

        node = yajl_tree_parse(body, NULL, 0);
        if (node) {
                value = my_yajl_tree_get(node,value_path,yajl_t_string);
                if (value) {
                        /* 
                         * YAJL probably copied it once, now we're going to
                         * copy it again.  If anybody really cares for such
                         * small and infrequently used values, we'd have to do
                         * do something much more complicated (like using the
                         * stream interface) to avoid the copy.  Right now it's
                         * just not worth it.
                         */
                        json_data = strdup(MY_YAJL_GET_STRING(value));
                }
                else {
                        /* Might as well try this. */
                         json_data = parse_array_response(node);
                }
                yajl_tree_free(node);
        }

    return json_data;

}


static etcd_result
etcd_get_one (_etcd_session *session, const char *key, etcd_server *srv, const char *prefix, char *stream_to)
{
        char            *url;
        etcd_result     res             = ETCD_WTF;
        void            *err_label      = &&done;
        ghttp_request *request;
        char *body;

        if (asprintf(&url,"http://%s:%u/v2/%s%s",
                     srv->host,srv->port,prefix,key) < 0) {
                goto *err_label;
        }
        err_label = &&free_url;

        request = ghttp_request_new();
        if (!request) {
                goto *err_label;
        }
        err_label = &&cleanup_ghttp;

        ghttp_set_uri(request, url);
        ghttp_set_header(request, http_hdr_Connection, "close");
        ghttp_prepare(request);

        ghttp_process(request);
        body = ghttp_get_body(request);
        if (!body) {
            #if defined(DEBUG)
                fprintf(stderr, ghttp_get_error(request));
            #endif
            goto *err_label;
        }

        stream_to = parse_get_response(body);
        if(!stream_to) {
            goto *err_label;
        }

        res = ETCD_OK;

cleanup_ghttp:
        ghttp_request_destroy(request);
free_url:
        free(url);
done:
        return res;
}


char *
etcd_get (etcd_session session_as_void, char *key)
{
        _etcd_session   *session   = session_as_void;
        etcd_server     *srv;
        etcd_result     res;
        char            *value;

        for (srv = session->servers; srv->host; ++srv) {
                res = etcd_get_one(session,key,srv, (const char *)"keys/",value);
                if ((res == ETCD_OK) && value) {
                        return value;
                }
        }

        return NULL;
}

static void
free_sl (etcd_server *server_list)
{
        size_t          num_servers;

        for (num_servers = 0; server_list[num_servers].host; ++num_servers) {
                free(server_list[num_servers].host);
        }
        free(server_list);
}


static int
_count_matching (const char *text, const char *cset, int result)
{
        char    *t;
        int     res     = 0;

        for (t = (char *)text; *t; ++t) {
                if ((strchr(cset,*t) != NULL) != result) {
                        break;
                }
                ++res;
        }

        return res;
}

#define count_matching(t,cs)    _count_matching(t,cs,1)
#define count_nonmatching(t,cs) _count_matching(t,cs,0)


etcd_session
etcd_open_str (char *server_names)
{
        char            *snp;
        int             run_len;
        int             host_len;
        size_t           num_servers;
        etcd_server     *server_list;
        etcd_session    *session;

        /*
         * Yeah, we iterate over the string twice so we can allocate an
         * appropriately sized array instead of turning it into a linked list.
         * Unfortunately this means we can't use strtok* which is destructive
         * with no platform-independent way to reverse the destructive effects.
         */

        num_servers = 0;
        snp = server_names;
        while (*snp) {
                run_len = count_nonmatching(snp,SL_DELIM);
                if (!run_len) {
                        snp += count_matching(snp,SL_DELIM);
                        continue;
                }
                ++num_servers;
                snp += run_len;
        }

        if (!num_servers) {
                return NULL;
        }

        server_list = calloc(num_servers+1,sizeof(*server_list));
        if (!server_list) {
                return NULL;
        }
        num_servers = 0;

        snp = server_names;
        while (*snp) {
                run_len = count_nonmatching(snp,SL_DELIM);
                if (!run_len) {
                        snp += count_matching(snp,SL_DELIM);
                        continue;
                }
                host_len = count_nonmatching(snp,":");
                if ((run_len - host_len) > 1) {
                        server_list[num_servers].host = strndup(snp,host_len);
                        server_list[num_servers].port = (unsigned short)
                                strtoul(snp+host_len+1,NULL,10);
                }
                else {
                        server_list[num_servers].host = strndup(snp,run_len);
                        server_list[num_servers].port = DEFAULT_ETCD_PORT;
                }
                ++num_servers;
                snp += run_len;
        }

        session = etcd_open(server_list);
        if (!session) {
                free_sl(server_list);
        }
        return session;
}


void
etcd_close_str (etcd_session session)
{
        free_sl(((_etcd_session *)session)->servers);
        etcd_close(session);
}
