#ifndef __SERVER_SOCKET__
#define __SERVER_SOCKET__
#include "lock.h"

#define DEFAULT_TIME_OUT    60
#define MAX_CONNECT_FD      1024
#define SERVER_PORT         8080
#define CACHE_BUFFER_SIZE   2048


enum SOCKET_EVENT{
	SOCKET_EVENT_SOCKET_CONNECT,
	SOCKET_EVENT_SOCKET_RECV,
	SOCKET_EVENT_MAX,
};

void server_socket_accpet_callback(int fd,short event_id,void* context);
void server_socket_receive_callback(int fd,short event_id,void* context);

struct server_socket;

typedef int (*socket_event_callback)(struct server_socket *psocket, void *object, int event_id, void *param);

struct socket_callback_event
{
	int event_id;
	socket_event_callback callback;
	void* object;
};


struct server_socket_operation
{
    int (*init)(struct server_socket*);
    int (*release)(struct server_socket*);
	
    int (*create_socket)(struct server_socket*);
    int (*close_socket)(struct server_socket*);

	int (*register_event)(struct server_socket*, socket_event_callback, int, void*);
	int (*unregister_event)(struct server_socket*, int, void*);
	int (*trigger_event)(struct server_socket*,int,void*);
};

struct server_socket
{
    int fd;
    lock_t lock;
	struct socket_callback_event *pcallbackevent[SOCKET_EVENT_MAX];
    struct server_socket_operation *op;
};


struct server_socket* create_init_server_socket();
int release_destroy_server_socket(struct server_socket* psocket);


#endif


