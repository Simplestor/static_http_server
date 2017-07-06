#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "userstring.h"
#include "log.h"

int read_property(char* words,char* property)
{
    char property_buffer[64];
    char buffer[32];
    char* temp;
    int temp_length;
    sprintf(property_buffer,"option %s '",property);
    temp_length = strcutfront(words,property_buffer);
    if(temp_length <= 0){
        DBG(DBG_ERR,"can not find option --- %s ----\n",property);
        return -EINVAL;
    }else{
        temp = words + temp_length;
        temp_length = strcutfront(temp,"'");
        if(temp_length <= 0){
            DBG(DBG_ERR,"%s is null\n",property);
            return -EINVAL;
        }else{
            strncpy(buffer,temp,temp_length);
            return atoi(buffer);
        }
    }
    return -EINVAL;
}

