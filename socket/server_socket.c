#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sys/fcntl.h>
#include <time.h>
#include "log.h"
#include "server_socket.h"

int server_socket_init(struct server_socket* psocket)
{
	lock_init(&psocket->lock);
    if(psocket->op->create_socket(psocket)){
        DBG(DBG_ERR,"create socket failed\n");
        return -EINVAL;
    }
   	DBG(DBG_DEBUG,"init server_socket success\n");
    return 0;
}

int server_socket_release(struct server_socket *psocket)
{
    psocket->op->close_socket(psocket);
	lock_destroy(&psocket->lock);
	DBG(DBG_DEBUG,"release socket success\n");
    return 0;
}

int server_socket_create_socket(struct server_socket *psocket)
{
    int error;
    int keep_alive = 1;
    int keep_idle = 5;
    int keep_interval = 5;
    int keep_count = 2;
    int reuse = 1;
    struct sockaddr_in server_addr;
    struct timeval timeout = {3,0};
    struct linger m_sLinger;
    m_sLinger.l_onoff = 1;
    m_sLinger.l_linger = 0;

    psocket->fd = socket(AF_INET,SOCK_STREAM,0);
    if(psocket->fd < 0){
        DBG(DBG_ERR,"create socket fail\n");
        return -EINVAL;
    }

    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    error = bind(psocket->fd,(struct sockaddr *)&server_addr,sizeof(server_addr));  
    if(error < 0){
        DBG(DBG_ERR,"set socket  port %d fail:%d \n",SERVER_PORT,error);
        return error;
    }
    error = setsockopt(psocket->fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if(error < 0){
        DBG(DBG_ERR,"set socket reuse addr fail:%d \n",error);
        return error;
    }
    error = fcntl(psocket->fd, F_SETFL, O_NONBLOCK);
    if(error < 0){
        DBG(DBG_ERR,"set socket nonblock fail:%d \n",error);
        return error;
    }
    error = setsockopt(psocket->fd, SOL_SOCKET, SO_LINGER, (const char*)&m_sLinger, sizeof(struct linger)); 
    if(error < 0){
        DBG(DBG_ERR,"set no linger fail\n");
        return error;
    }
    error = setsockopt(psocket->fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keep_alive, sizeof(keep_alive));
    if(error < 0){
        DBG(DBG_ERR,"set socket keep alive fail:%d \n",error);
        return error;
    }
    error = setsockopt(psocket->fd, SOL_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count));
    if(error < 0){
        DBG(DBG_ERR,"set socket keep_count fail:%d \n",error);
        return error;
    }
    error = setsockopt(psocket->fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keep_idle, sizeof(keep_idle));
    if(error < 0){
        DBG(DBG_ERR,"set socket keep_idle fail:%d \n",error);
        return error;
    }
    error = setsockopt(psocket->fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval));
    if(error < 0){
        DBG(DBG_ERR,"set socket keep_interval fail:%d \n",error);
        return error;
    }
    error = setsockopt(psocket->fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
    if(error < 0){
        DBG(DBG_ERR,"set socket send_timeout fail:%d \n",error);
        return error;
    }
    error = setsockopt(psocket->fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    if(error < 0){
        DBG(DBG_ERR,"set socket recv_timeout fail:%d \n",error);
        return error;
    }
    error = listen(psocket->fd, MAX_CONNECT_FD);  
    if (error < 0){
        DBG(DBG_ERR,"set socket listen max fd fail:%d \n",error);
        return error;
    }
    return 0;
}

int server_socket_close_socket(struct server_socket* psocket)
{
    if(psocket->fd > 0){
        shutdown(psocket->fd,2);
        close(psocket->fd);
    }
	return 0;
}

int server_socket_register_event(struct server_socket* psocket, socket_event_callback callback, int event_id, void* object){
	struct socket_callback_event* pcallbackevent;
	if(!psocket || !callback || (event_id >= SOCKET_EVENT_MAX)){
		DBG(DBG_ERR,"register event invalid param\n");
		return -EINVAL;
	}
	if(!(psocket->pcallbackevent[event_id])){
		pcallbackevent = (struct socket_callback_event*)malloc(sizeof(struct socket_callback_event));
		if(!pcallbackevent){
			DBG(DBG_ERR,"malloc event error\n");
			return -ENOMEM;
		}
		pcallbackevent->callback = callback;
		pcallbackevent->object = object;
		psocket->pcallbackevent[event_id] = pcallbackevent;
	}else{
		DBG(DBG_WARN,"event has been registed\n");
	}
	return 0;
}

int server_socket_unregister_event(struct server_socket* psocket,int event_id,void* object){
	struct socket_callback_event* pcallbackevent;
	if(!psocket || (event_id >= SOCKET_EVENT_MAX)){
		DBG(DBG_ERR,"register event invalid param\n");
		return -EINVAL;
	}
	pcallbackevent = psocket->pcallbackevent[event_id];
	if(pcallbackevent->object == object){
		free(pcallbackevent);
		pcallbackevent = NULL;
	}
	return 0;
}

int server_socket_trigger_event(struct server_socket* psocket,int event_id,void* param){
	struct socket_callback_event* pcallbackevent;
	pcallbackevent = psocket->pcallbackevent[event_id];
	if(!pcallbackevent){
		DBG(DBG_ERR,"task trigger event can not find event id\n");
		return -EINVAL;
	}
	if(pcallbackevent->callback){
		pcallbackevent->callback(psocket, pcallbackevent->object, event_id, param);
	}
	return 0;
}



struct server_socket_operation server_socket_op =
{
    .init               = server_socket_init,
    .release            = server_socket_release,

    .create_socket      = server_socket_create_socket,
    .close_socket       = server_socket_close_socket,

	.register_event     = server_socket_register_event,
	.unregister_event   = server_socket_unregister_event,
	.trigger_event      = server_socket_trigger_event,
};

struct server_socket* create_init_server_socket()
{
    struct server_socket * psocket = NULL;
    psocket = (struct server_socket*)malloc(sizeof(struct server_socket));
    if(!psocket){
        DBG(DBG_ERR,"malloc server socket failed\n");
        return NULL;
    }
	memset(psocket,0,sizeof(struct server_socket));
    psocket->op = &server_socket_op;
    if(psocket->op->init(psocket) < 0){
        DBG(DBG_ERR,"create init server socket failed, init faield\n");
		release_destroy_server_socket(psocket);
        return NULL;
    }
    DBG(DBG_DEBUG,"create server socket success, fd = %d \n",psocket->fd);
    return psocket;
}

int release_destroy_server_socket(struct server_socket* psocket)
{
    if(psocket){
        psocket->op->release(psocket);
	    free(psocket);
    }
	return 0;
}


