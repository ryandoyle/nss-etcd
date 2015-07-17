#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <nss.h>
#include <stdlib.h>

#include "nss-etcd-api.h"


enum nss_status _nss_etcd_gethostbyname2_r(
    const char *name,
    int af,
    struct hostent * result,
    char *buffer,
    size_t buflen,
    int *errnop,
    int *h_errnop) {

	enum nss_status status;
	size_t idx;
	uint32_t address;

	/* We could get back here if ghttp is trying to lookup the host it needs to
	 * connect to (part of etcd-api). Get out of here so we don't end up in an
	 * endless loop
	 */
	if(!strcmp(name, "localhost"))
		return NSS_STATUS_UNAVAIL;

	/* Bail out if we're not IPv4 */
	if(af != AF_INET)
		return NSS_STATUS_UNAVAIL;

	/* Get the ip address */
	status = nss_etcd_ip_address(name, &address);
	if(status != NSS_STATUS_SUCCESS)
		return status;


	/* Populate the hostent struct */
	result->h_length = sizeof(uint32_t); // ipv4 only

	/* null out our buffer */
	*((char**) buffer) = NULL;

	/* get the pointer address, casted as a char** */
	result->h_aliases = (char**) buffer;
	idx = sizeof(char*);

	/* Official name */
	strcpy(buffer+idx, name);
	result->h_name = buffer+idx;
	idx += strlen(name)+1;

	/* always IPv4 for now */
	result->h_addrtype = AF_INET;

	if (idx % sizeof(char*))
		idx += (sizeof(char*) - idx % sizeof(char*)); /* Align on 32 bit boundary */


	/* Copy in the IP address that we got from nss_etcd_ip_address() */
	memcpy(buffer+idx, &address, sizeof(uint32_t));

	/* pointer to the first address */
	((char**) (buffer+idx+4))[0] = buffer+idx;

	/* null denotes the end of the addr_list */
	((char**) (buffer+idx+4))[1] = NULL;

	/* pointer to the address list (in the buffer) */
	result->h_addr_list = (char**) (buffer+idx+4);

	return status;
}

enum nss_status _nss_etcd_gethostbyname_r (
    const char *name,
    struct hostent *result,
    char *buffer,
    size_t buflen,
    int *errnop,
    int *h_errnop) {

    return _nss_etcd_gethostbyname2_r(
        name,
        AF_INET,
        result,
        buffer,
        buflen,
        errnop,
        h_errnop);
}