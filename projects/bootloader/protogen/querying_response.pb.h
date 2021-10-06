/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6-dev */

#ifndef PB_QUERYING_RESPONSE_PB_H_INCLUDED
#define PB_QUERYING_RESPONSE_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _QueryingResponse { 
    uint32_t id; 
    pb_callback_t name; 
    pb_callback_t current_project; 
    pb_callback_t project_info; 
    pb_callback_t git_version; 
} QueryingResponse;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define QueryingResponse_init_default            {0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}
#define QueryingResponse_init_zero               {0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define QueryingResponse_id_tag                  1
#define QueryingResponse_name_tag                2
#define QueryingResponse_current_project_tag     3
#define QueryingResponse_project_info_tag        4
#define QueryingResponse_git_version_tag         5

/* Struct field encoding specification for nanopb */
#define QueryingResponse_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   id,                1) \
X(a, CALLBACK, SINGULAR, STRING,   name,              2) \
X(a, CALLBACK, SINGULAR, STRING,   current_project,   3) \
X(a, CALLBACK, SINGULAR, STRING,   project_info,      4) \
X(a, CALLBACK, SINGULAR, STRING,   git_version,       5)
#define QueryingResponse_CALLBACK pb_default_field_callback
#define QueryingResponse_DEFAULT NULL

extern const pb_msgdesc_t QueryingResponse_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define QueryingResponse_fields &QueryingResponse_msg

/* Maximum encoded size of messages (where known) */
/* QueryingResponse_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif