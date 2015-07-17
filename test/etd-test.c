#include <stdio.h>
#include <arpa/inet.h>
#include <nss.h>
#include <nss-etcd-api.h>

void main(){

	const char *name = "database.primary";
	uint32_t address = NULL;
	enum nss_status result;

	result = nss_etcd_ip_address(name, &address);

	printf("value: %u, result: %i\n", address, result);

}
