#include <stdio.h>
#include <arpa/inet.h>
#include <nss.h>
#include "nss-etcd.h"

void main(){

	const char *name = "database.primary";
	uint32_t address;
	enum nss_status result;

	result = nss_etcd_ip_address(name, &address);

	printf("value: %u, result: %i", address, result);

}
