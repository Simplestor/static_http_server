#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "server_socket.h"

struct http_server;

struct http_server_operation
{
    int (*init)(struct http_server*);
    int (*release)(struct http_server*);
};

struct http_server
{
    struct server_socket* psocket;
    struct http_server_operation* op;
};

struct http_server* create_init_http_server();
int release_destroy_http_server(struct http_server* pserver);


#endif
