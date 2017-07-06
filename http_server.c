#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "http_server.h"
#include "log.h"

static int http_server_init(struct http_server* pserver)
{
    pserver->psocket = create_init_server_socket();
    if(!pserver->psocket){
        DBG(DBG_ERR,"create server socket failed\n");
        return -EINVAL;
    }
    return 0;
}

static int http_server_release(struct http_server* pserver)
{
    if(pserver->psocket){
        release_destroy_server_socket(pserver->psocket);
    }
    return 0;
}

struct http_server_operation http_server_op = {
    .init           = http_server_init,
    .release        = http_server_release,
};

struct http_server* create_init_http_server()
{
    struct http_server* pserver;
    pserver = (struct http_server*)malloc(sizeof(struct http_server));
    if(!pserver){
        DBG(DBG_ERR,"malloc http server error\n");
        return NULL;
    }
    memset(pserver,0,sizeof(struct http_server));
    pserver->op = &http_server_op;
    if(pserver->op->init(pserver) < 0){
        DBG(DBG_ERR,"init http_server error\n");
        release_destroy_http_server(pserver);
        return NULL;
    }
    return pserver;
}

int release_destroy_http_server(struct http_server* pserver)
{
    if(pserver){
        pserver->op->release(pserver);
        free(pserver);
    }
    return 0;
}
