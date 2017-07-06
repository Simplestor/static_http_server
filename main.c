#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "http_server.h"
#include "log.h"

int main(int argc,char* argv[])
{
	struct http_server* pserver;
    pserver = create_init_http_server();
    if(!pserver){
        DBG(DBG_ERR,"create http server failed\n");
        return -EINVAL;
    }
	while(1){
		sleep(1);
	}
	if(release_destroy_http_server(pserver) < 0){
		DBG(DBG_ERR,"release destory http server failed\n");
	}
    return 0;
}

