/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2015-2022 The Fluent Bit Authors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef FLB_OUT_FORWARD
#define FLB_OUT_FORWARD

#include <fluent-bit/flb_info.h>
#include <fluent-bit/flb_sds.h>
#include <fluent-bit/flb_upstream_ha.h>
#include <fluent-bit/flb_record_accessor.h>
#include <fluent-bit/flb_upstream_conn.h>

#ifdef FLB_HAVE_TLS
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/ctr_drbg.h>
#endif

/* Forward modes */
#define MODE_MESSAGE               0
#define MODE_FORWARD               1
#define MODE_FORWARD_COMPAT        3

/* Compression modes */
#define COMPRESS_NONE              0
#define COMPRESS_GZIP              1

/*
 * Configuration: we put this separate from the main
 * context so every Upstream Node can have it own configuration
 * reference and pass it smoothly to the required caller.
 *
 * On simple mode (no HA), the structure is referenced
 * by flb_forward->config. In HA mode the structure is referenced
 * by the Upstream node context as an opaque data type.
 */
struct flb_forward_config {
    int secured;              /* Using Secure Forward mode ?  */
    int compress;             /* Using compression ? */
    int time_as_integer;      /* Use backward compatible timestamp ? */

    /* config */
    flb_sds_t shared_key;        /* shared key                   */
    flb_sds_t self_hostname;     /* hostname used in certificate  */
    flb_sds_t tag;               /* Overwrite tag on forward */
    int empty_shared_key;        /* use an empty string as shared key */
    int require_ack_response;    /* Require acknowledge for "chunk" */
    int send_options;            /* send options in messages */
    flb_sds_t unix_path;         /* unix socket path */
    int       unix_fd;

    const char *username;
    const char *password;

    /* mbedTLS specifics */
    unsigned char shared_key_salt[16];
#ifdef FLB_HAVE_TLS
    mbedtls_entropy_context tls_entropy;
    mbedtls_ctr_drbg_context tls_ctr_drbg;
#endif

#ifdef FLB_HAVE_RECORD_ACCESSOR
    struct flb_record_accessor *ra_tag; /* Tag Record accessor */
    int ra_static;                      /* Is the record accessor static ? */
#endif
    int (*io_write)(struct flb_upstream_conn* conn, int fd, const void* data,
                        size_t len, size_t *out_len);
    int (*io_read)(struct flb_upstream_conn* conn, int fd, void* buf, size_t len);
    struct mk_list _head;     /* Link to list flb_forward->configs */
};

/* Plugin Context */
struct flb_forward {
    /* if HA mode is enabled */
    int ha_mode;              /* High Availability mode enabled ? */
    char *ha_upstream;        /* Upstream configuration file      */
    struct flb_upstream_ha *ha;

    /* Upstream handler and config context for single mode (no HA) */
    struct flb_upstream *u;
    struct mk_list configs;
    struct flb_output_instance *ins;
};

struct flb_forward_ping {
    const char *nonce;
    int nonce_len;
    const char *auth;
    int auth_len;
    int keepalive;
};

/* Flush callback context */
struct flb_forward_flush {
    struct flb_forward_config *fc;
    char checksum_hex[33];
};

struct flb_forward_config *flb_forward_target(struct flb_forward *ctx,
                                              struct flb_upstream_node **node);

#endif
