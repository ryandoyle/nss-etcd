#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <nss.h>

#include "etcd-api/etcd-api2.h"

#include "nss-etcd-api.h"


static char *nss_etcd_key_from_name(const char *name){
    int i;

    if (name == NULL)
        return NULL;
    char *new_name;
    size_t name_length;

    name_length = strlen(name)+1;
    new_name = malloc(name_length);
    for (i=0; i < name_length; i++){
        /* replace '.' with '/' */
        new_name[i] = (char) ((name[i] == '.') ? '/' : name[i]);
    }
    return new_name;
}

enum nss_status nss_etcd_ip_address(const char *name, uint32_t *ip_address){

    char *key;
    char *value = NULL;
    char *servers = "localhost";
    etcd_session    etcd_session = NULL;
    enum nss_status return_code;

    // covert the domain to a key
    key = nss_etcd_key_from_name(name);
    if(!key){
        return_code = NSS_STATUS_UNAVAIL;
        goto cleanup;
    }

    // create the session
    etcd_session = etcd_open_str(servers);
    if(!etcd_session){
        return_code = NSS_STATUS_UNAVAIL;
        goto cleanup;
    }

    // get the value
    value = etcd_get(etcd_session,key);
    if(!value){
        return_code = NSS_STATUS_NOTFOUND;
        goto cleanup;
    }

    // get the value in decimal form
    if(!inet_pton(AF_INET, value, ip_address)){
        return_code = NSS_STATUS_NOTFOUND;
        goto cleanup;
    }

    // We found the record!
    return_code = NSS_STATUS_SUCCESS;
    goto cleanup;

    cleanup:
    free(key);
    if(value != NULL) {
        free(value);
    }
    if(etcd_session != NULL) {
        etcd_close_str(etcd_session);
    }
    return return_code;

}
