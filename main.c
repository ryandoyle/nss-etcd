#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "etcd.h"
#include "etcd-api.h"

void main(){

	const char *name = "database.primary";
	uint32_t address;

	address = etcd_ip_address(name);

	printf("value: %u", address);



}
