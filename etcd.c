#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "etcd-api.h"


char *etcd_key_from_name(const char *name){
	int i;

	if (name == NULL)
		return NULL;
	char *new_name;
	new_name = malloc(strlen(name)+1);
	for (i=0; i < strlen(name)+1; i++){
		/* replace '.' with '/' */
		new_name[i] = (name[i] == '.') ? '/' : name[i];
	}
	return new_name;
}

uint32_t etcd_ip_address(const char *name){
	char *key;
	char *value;
	char *servers = "localhost,localhost";
	etcd_session    etcd_session;


	struct sockaddr_in sa;

	uint32_t in_addr;
	//uint32_t address_in_network_order;

	// covert the domain to a key
	key = etcd_key_from_name(name);

	// create the session
	etcd_session = etcd_open_str(servers);

	// get the value
	value = etcd_get(etcd_session,key);

	// get the value in decimal form
	inet_pton(AF_INET, value, &in_addr);

	// print it
	//address_in_network_order = htonl(in_addr);

	// Cleanup
	etcd_close_str(etcd_session);
	free(key);
	free(value);
	//return address_in_network_order;
	return in_addr;
}
