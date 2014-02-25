#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>
#include <sys/socket.h>
#include <nss.h>
#include <stdio.h>
#include <stdlib.h>

enum nss_status _nss_etcd_gethostbyname2_r(
    const char *name,
    int af,
    struct hostent * result,
    char *buffer,
    size_t buflen,
    int *errnop,
    int *h_errnop) {

	enum nss_status status = NSS_STATUS_UNAVAIL;
	size_t idx = sizeof(char*);

	/* stub failure */
	if(0) {
		*errnop = ETIMEDOUT;
		*h_errnop = HOST_NOT_FOUND;
		goto finish;
	}

	result->h_length = sizeof(uint32_t); // ipv4 only

	// deref the location, set it to null, cast as a char**
	*((char**) buffer) = NULL;


	// get the pointer address, casted as a char**
	result->h_aliases = (char**) buffer;
	idx = sizeof(char*);

	/* Official name */
	strcpy(buffer+idx, name);
	result->h_name = buffer+idx;
	idx += strlen(name)+1;

	result->h_addrtype = AF_INET;

	if (idx%sizeof(char*))
	        idx+=(sizeof(char*)-idx%sizeof(char*)); /* Align on 32 bit boundary */

	/* copy the ip address into the buffer */

	//memcpy(buffer+idx+1, NULL, 1);

	/* Address array */
	/*    for (i = 0; i < u.count; i++)
	        ((char**) (buffer+idx))[i] = buffer+astart+address_length*i;
	    ((char**) (buffer+idx))[i] = NULL;
	*/

	//memcpy(buffer+idx, (int*) 2053886942, sizeof(uint32_t));

	/* put the address into the buffer */
	//((char**) (buffer+idx))[0] = 2053886942;
	//int *address_1 = (buffer+idx);
	//idx += sizeof(uint32_t);

	//uint32_t *ip_address = 2053886942;


	/* populate this! */
	//((char**) (buffer+idx))[0] = &ip_address;

	/* SET THE VALUE */
	uint32_t address = htonl(16909060);;
	memcpy(buffer+idx, &address, sizeof(uint32_t));

	/* pointer to the first address */
	((char**) (buffer+idx+4))[0] = buffer+idx;

	/* null denotes the end of the addr_list */
	((char**) (buffer+idx+4))[1] = NULL;

	/* pointer to the address list (in the buffer) */
	result->h_addr_list = (char**) (buffer+idx+4);

	status = NSS_STATUS_SUCCESS;

	return status;

	finish:
		return status;

}


/* struct hostent {
    char  *h_name;            official name of host
    char **h_aliases;          alias list
    int    h_addrtype;         host address type
    int    h_length;           length of address
    char **h_addr_list;        list of addresses
}
*/
