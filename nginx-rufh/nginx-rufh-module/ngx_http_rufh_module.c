/**
 * @file   ngx_http_rufh_module.c
 * @author António P. P. Almeida <appa@perusio.net>
 * @date   Wed Aug 17 12:06:52 2011
 *
 * @brief  A hello world module for Nginx.
 *
 * @section LICENSE
 *
 * Copyright (C) 2011 by Dominic Fallows, António P. P. Almeida <appa@perusio.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <stdio.h>

#define HELLO_WORLD ""

static char *ngx_http_rufh(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_rufh_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_rufh_handler(ngx_http_request_t *r);

/**
 * This module provided directive: hello world.
 *
 */
static ngx_command_t ngx_http_rufh_commands[] = {

    {ngx_string("resumable_uploads"),     /* directive */
     NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS, /* location context and takes
                                             no arguments*/
     ngx_http_rufh,                /* configuration setup function */
     0,                                   /* No offset. Only one context is supported. */
     0,                                   /* No offset when storing the module configuration on struct. */
     NULL},

    ngx_null_command /* command termination */
};

/* The hello world string. */
static u_char ngx_hello_world[] = HELLO_WORLD;

/* The module context. */
static ngx_http_module_t ngx_http_rufh_module_ctx = {
    NULL,                      /* preconfiguration */
    ngx_http_rufh_init, /* postconfiguration */

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    NULL, /* create location configuration */
    NULL  /* merge location configuration */
};

/* Module definition. */
ngx_module_t ngx_http_rufh_module = {
    NGX_MODULE_V1,
    &ngx_http_rufh_module_ctx, /* module context */
    ngx_http_rufh_commands,    /* module directives */
    NGX_HTTP_MODULE,                  /* module type */
    NULL,                             /* init master */
    NULL,                             /* init module */
    NULL,                             /* init process */
    NULL,                             /* init thread */
    NULL,                             /* exit thread */
    NULL,                             /* exit process */
    NULL,                             /* exit master */
    NGX_MODULE_V1_PADDING};

typedef struct {
    FILE * file;
} ngx_http_live_ctx_t;

static void
ngx_http_rufh_read_handler(ngx_http_request_t *r)
{
    // size_t                    n, left;
    ngx_int_t rc;
    ngx_buf_t                *b;
    ngx_chain_t *cl;
    ngx_event_t *rev;
    // ngx_http_live_msg_t      *msg;
    // ngx_http_live_ctx_t      *ctx;
    ngx_http_request_body_t *rb;
    static size_t len = 0;

    ngx_http_live_ctx_t       *ctx;


    ngx_chain_t *in;

    if (ngx_exiting || ngx_terminate)
    {
        ngx_http_finalize_request(r, NGX_HTTP_CLOSE);
        return;
    }

    rev = r->connection->read;

    if (rev->delayed)
    {
        if (ngx_handle_read_event(rev, 0) != NGX_OK)
        {
            ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        }
        return;
    }

    rb = r->request_body;
    if (rb == NULL)
    {
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    r->read_event_handler = ngx_http_rufh_read_handler;

    ctx = ngx_http_get_module_ctx(r, ngx_http_rufh_module);


    for (;;)
    {
        // Consume all buffers
        for (; rb->bufs; rb->bufs = rb->bufs->next)
        {
            len += ngx_buf_size(rb->bufs->buf);
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "read %d total %d %d", ngx_buf_size(rb->bufs->buf), len, r->reading_body);

            fwrite(rb->bufs->buf->start, 1, ngx_buf_size(rb->bufs->buf), ctx->file);
            
            rb->bufs->buf->pos = rb->bufs->buf->last;
            
        }

        if (r->reading_body)
        {
            // Query nginx for more data
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "before %d %d", rb->bufs == NULL, r->reading_body);
            rc = ngx_http_read_unbuffered_request_body(r);

            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "after %d %d %d", rb->bufs == NULL, r->reading_body, rc == NGX_AGAIN);

            // if (rc == NGX_AGAIN)
            // {
            //     // More data is available now in the buffers
            //     continue;
            // }

            if (rb->bufs == NULL)
            {
                // More data is available in the next callback
                r->read_event_handler = ngx_http_rufh_read_handler;
                return;
            }

            continue;
        }

        break;
    }



    // The entire body has been read
    fclose(ctx->file);

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "next phase");

    // TODO: Add the request body
    r->headers_in.content_length_n = 3;

    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) { return; }

    b = ngx_create_temp_buf(r->pool, 3);
    if (b == NULL) {return; }

    b->last = ngx_cpymem(b->last, "foo", 3);

    cl->buf = b;
    cl->next = NULL;
    // *ll = cl;

    ngx_http_top_request_body_filter(r, cl);

    r->phase_handler++;
    ngx_http_core_run_phases(r);

    return;

    // r->request_body->bufs->buf->in_file = 1;
    // r->request_body->bufs->buf->file_pos = 0;
    // r->request_body->bufs->buf->file_last = 11;
    // r->request_body->bufs->buf->in_file = 1;
    // r->request_body->bufs->buf->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
    // if (r->request_body->bufs->buf->file == NULL) {
    //     return;
    // }

    // r->request_body->bufs->buf->file->fd = open("./uploads/test.txt", O_RDONLY);
    // r->request_body->bufs->buf->file->offset = 0;

//         ngx_fd_t                   fd;
//     ngx_str_t                  name;
//     ngx_file_info_t            info;

//     off_t                      offset;
//     off_t                      sys_offset;

//     ngx_log_t                 *log;

// #if (NGX_THREADS || NGX_COMPAT)
//     ngx_int_t                (*thread_handler)(ngx_thread_task_t *task,
//                                                ngx_file_t *file);
//     void                      *thread_ctx;
//     ngx_thread_task_t         *thread_task;
// #endif

// #if (NGX_HAVE_FILE_AIO || NGX_COMPAT)
//     ngx_event_aio_t           *aio;
// #endif

//     unsigned                   valid_info:1;
//     unsigned                   directio:1;

    // r->request_body->bufs->buf->


    for (in = r->request_body->bufs; in; in = in->next)
    {
        // len += ngx_buf_size(in->buf);
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "read %d", ngx_buf_size(in->buf));
    }

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                  "before %d %d", rb->bufs == NULL, r->reading_body);
    rc = ngx_http_read_unbuffered_request_body(r);

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                  "after %d %d %d", rb->bufs == NULL, r->reading_body, rc == NGX_AGAIN);

    for (in = r->request_body->bufs; in; in = in->next)
    {
        // len += ngx_buf_size(in->buf);
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "read %d", ngx_buf_size(in->buf));
    }

    return;

    for (;;)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "in loop %d %d", rb->bufs == NULL, r->reading_body);

        if (rb->bufs == NULL && r->reading_body)
        {
            rc = ngx_http_read_unbuffered_request_body(r);

            if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
            {
                ngx_http_finalize_request(r, rc);
                return;
            }
        }

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "a %d", rb->bufs == NULL);

        if (rb->bufs == NULL)
        {
            break;
        }

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "b");

        cl = rb->bufs;

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "read chunk %d %d", ngx_buf_size(cl->buf), cl->buf->last - cl->buf->pos);

        // n = cl->buf->last - cl->buf->pos;
        // left = b->end - b->last;

        // if (n > left) {
        //     n = left;
        // }

        // b->last = ngx_cpymem(b->last, cl->buf->pos, n);

        // cl->buf->pos += n;

        // if (b->last == b->end) {
        //     break;
        // }

        // len = 0;

        rb->bufs = cl->next;
        ngx_free_chain(r->pool, cl);
    }

    // if (b->last != b->pos + offsetof(ngx_http_live_msg_t, data)) {
    //     msg = (ngx_http_live_msg_t *) b->pos;
    //     msg->size = b->last - b->pos;
    //     msg->live = ctx->live;
    //     msg->counter = ctx->counter++;
    //     msg->last = (rb->bufs || r->reading_body) ? 0 : 1;
    //     ngx_memcpy(msg->md5, ctx->md5, 16);

    //     rc = ngx_http_live_notify(r, b);
    //     if (rc == NGX_ERROR) {
    //         ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    //         return;
    //     }

    //     if (msg->last) {
    //         ctx->last_sent = 1;
    //     }
    // }

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                  "after loop %d %d", rb->bufs == NULL, r->reading_body);

    if (rb->bufs)
    {
        r->read_event_handler = ngx_http_rufh_read_handler;
        ngx_add_timer(rev, 1);
        rev->delayed = 1;
        return;
    }

    if (r->reading_body)
    {
        r->read_event_handler = ngx_http_rufh_read_handler;
        return;
    }

    ngx_http_finalize_request(r, NGX_HTTP_NO_CONTENT);
}

/**
 * Content handler.
 *
 * @param r
 *   Pointer to the request structure. See http_request.h.
 * @return
 *   The status of the response generation.
 */
static ngx_int_t ngx_http_rufh_post_handler(ngx_http_request_t *r)
{
    ngx_int_t rc;
    ngx_table_elt_t *h;
    ngx_http_live_ctx_t       *ctx;

    // char * upload_id = "9deb0d5e-fc3b-496a-868c-f1ec18c0e481";
    char *file_path = "./uploads/9deb0d5e-fc3b-496a-868c-f1ec18c0e481";

    FILE *file = fopen(file_path, "a");

    if (file == NULL)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "error creating upload file");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_live_ctx_t));
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    ctx->file = file;

    ngx_http_set_ctx(r, ctx, ngx_http_rufh_module);


    // Write some data to the file
    // fprintf(file, "This is a test.\n");

    // Close the file when done
    // fclose(file);

    if (r->http_version >= NGX_HTTP_VERSION_11
#if (NGX_HTTP_V2)
        && r->stream == NULL
#endif
#if (NGX_HTTP_V3)
        && r->connection->quic == NULL
#endif
    )
    {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "send 104 Continue");

        r->connection->send(r->connection,
                                (u_char *) "HTTP/1.1 104 Upload Resumption Supported" CRLF CRLF,
                                sizeof("HTTP/1.1 104 Upload Resumption Supported" CRLF CRLF) - 1);
    }

    r->request_body_no_buffering = 1;

    rc = ngx_http_read_client_request_body(r, ngx_http_rufh_read_handler);

    if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
    {
        return rc;
    }

    return NGX_DONE;

    /* send header */

    r->headers_out.status = 102;
    r->headers_out.content_length_n = 0;

    /* X-Foo: foo */

    h = ngx_list_push(&r->headers_out.headers);
    if (h == NULL)
    {
        return NGX_ERROR;
    }

    h->hash = 1;
    ngx_str_set(&h->key, "Location");
    ngx_str_set(&h->value, "foo");

    return NGX_HTTP_NO_CONTENT;

    rc = 5;
    // rc = ngx_http_send_header(r);

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_http_send_header: %d", rc);

    // if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
    //     return rc;
    // }

    return NGX_HTTP_OK;
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Sending 103 Early Hints");

    // Set a 103 Early Hints response
    r->headers_out.status = 103;

    // Set the "Link" header to provide hints for the next request
    ngx_table_elt_t *link_header = ngx_list_push(&r->headers_out.headers);
    if (link_header)
    {
        link_header->hash = 1;
        ngx_str_set(&link_header->key, "Link");
        ngx_str_set(&link_header->value, "</foo>; rel=preload");
    }

    ngx_http_send_header(r);

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Sending 200 OK");

    // Set a 200 OK response
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = 0;

    ngx_http_send_header(r);

    return NGX_OK;

    ngx_table_elt_t *location_header = ngx_list_push(&r->headers_out.headers);
    if (location_header)
    {
        location_header->hash = 1;
        ngx_str_set(&location_header->key, "Location");
        ngx_str_set(&location_header->value, "/foo");
    }

    r->headers_out.status = 104;
    r->headers_out.content_length_n = 0;

    ngx_http_send_header(r);

    return 201;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = 0;

    return ngx_http_send_header(r);

    ngx_buf_t *b;
    ngx_chain_t out;

    /* Set the Content-Type header. */
    // r->headers_out.location->len = sizeof("/files/fooo") - 1;
    // r->headers_out.location->data = (u_char *) "/files/fooo";

    /* Allocate a new buffer for sending out the reply. */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    /* Insertion in the buffer chain. */
    out.buf = b;
    out.next = NULL; /* just one buffer */

    b->pos = ngx_hello_world;  /* first position in memory of the data */
    b->last = ngx_hello_world; /* last position in memory of the data */
    b->memory = 1;             /* content is in read-only memory */
    b->last_buf = 1;           /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    // r->headers_out.status = 104; /* 200 status code */
    // /* Get the content length of the body. */
    // // r->headers_out.content_length_n = sizeof(ngx_hello_world) - 1;
    // ngx_http_send_header(r); /* Send the headers */

    /* Sending the headers for the reply. */
    r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = 0;
    ngx_http_send_header(r); /* Send the headers */

    // ngx_http_finalize_request(r, NGX_DONE);

    // return NGX_HTTP_OK;

    /* Send the body, and return the status code of the output filter chain. */
    return ngx_http_output_filter(r, &out);
} /* ngx_http_rufh_handler */

/**
 * Content handler.
 *
 * @param r
 *   Pointer to the request structure. See http_request.h.
 * @return
 *   The status of the response generation.
 */
static ngx_int_t ngx_http_rufh_handler(ngx_http_request_t *r)
{
    ngx_buf_t *b;
    ngx_chain_t out;

    switch (r->method)
    {
    case NGX_HTTP_POST:
        // TODO: HAndle upload creation
        return ngx_http_rufh_post_handler(r);
    case NGX_HTTP_PATCH:
        // TODO: Handle upload appending
        break;
    case NGX_HTTP_HEAD:
        // TODO: Handle offset retrieval
        break;
    case NGX_HTTP_DELETE:
        // TODO: Handle upload termination/cancellation
        break;
    default:
        return NGX_DECLINED;
    }

    /* Set the Content-Type header. */
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *)"text/plain";

    /* Allocate a new buffer for sending out the reply. */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    /* Insertion in the buffer chain. */
    out.buf = b;
    out.next = NULL; /* just one buffer */

    b->pos = ngx_hello_world;                                /* first position in memory of the data */
    b->last = ngx_hello_world + sizeof(ngx_hello_world) - 1; /* last position in memory of the data */
    b->memory = 1;                                           /* content is in read-only memory */
    b->last_buf = 1;                                         /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = sizeof(ngx_hello_world) - 1;
    ngx_http_send_header(r); /* Send the headers */

    /* Send the body, and return the status code of the output filter chain. */
    return ngx_http_output_filter(r, &out);
} /* ngx_http_rufh_handler */

/**
 * Configuration setup function that installs the content handler.
 *
 * @param cf
 *   Module configuration structure pointer.
 * @param cmd
 *   Module directives structure pointer.
 * @param conf
 *   Module configuration structure pointer.
 * @return string
 *   Status of the configuration setup.
 */
static char *ngx_http_rufh(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    // ngx_http_core_loc_conf_t *clcf; /* pointer to core location configuration */

    // /* Install the hello world handler. */
    // clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    // clcf->handler = ngx_http_rufh_handler;

    // ngx_http_handler_pt        *h;
    // ngx_http_core_main_conf_t  *cmcf;

    // cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    // h = ngx_array_push(&cmcf->phases[NGX_HTTP_PRECONTENT_PHASE].handlers);
    // if (h == NULL) {
    //     return NGX_ERROR;
    // }

    // *h = ngx_http_rufh_handler;

    return NGX_OK;
} /* ngx_http_rufh */

static ngx_int_t
ngx_http_rufh_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_PRECONTENT_PHASE].handlers);
    if (h == NULL)
    {
        return NGX_ERROR;
    }

    *h = ngx_http_rufh_handler;

    return NGX_OK;
}