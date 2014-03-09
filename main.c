#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <nss.h>
#include "nss-etcd.h"
#include "etcd-api.h"

void main(){

const char *name = "databasee.primary";
	uint32_t address;
	enum nss_status result;

	result = nss_etcd_ip_address(name, &address);

	printf("value: %u, result: %i", address, result);

}
