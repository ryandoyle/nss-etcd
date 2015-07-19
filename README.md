Dynamic configuration is cool, but we still live in a world of static configuration. `nss-etcd` provides a host resolution module for GNU libc Name Service Switch (NSS). Think of it as a distributed and consistent hosts file without the complication and delay of normal DNS. `nss-etcd` does _not_ require any modification to your existing DNS setup.
```shell
$ curl -L http://127.0.0.1:2379/v2/keys/database/primary -XPUT -d value="10.0.0.20"
$ ping database.primary
PING database.primary (10.0.0.20) 56(84) bytes of data.
64 bytes from 10.0.0.20: icmp_seq=1 ttl=64 time=0.320 ms
```
#Getting started
### Prerequisites
CMake, libyajl and a C compiler are required to build and install `nss-etcd`
```shell
apt-get install cmake libyajl-dev gcc
```
**Currently etcd is also required to be running on localhost.**
### Building and installing nss-etcd
```shell
git clone https://github.com/ryandoyle/nss-etcd.git
cd nss-etcd
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr .
make install
```
### Configuring
`nss-etcd` has to be enabled in `/etc/nsswitch.conf` to start to be used for host resolution. Add `etcd` in the hosts section of this file.
```file:/etc/nsswitch.conf
hosts:          files myhostname etcd dns
```
Resolution is ordered left to right. For most cases, it should be before `dns`.

### Adding and resolving names
The namespace in etcd is mapped to dot-delimitered names. You have full control of the namespace you create in etcd.
```shell
$ curl -L http://127.0.0.1:2379/v2/keys/database/primary -XPUT -d value="10.0.0.20"
$ getent hosts database.primary
10.0.0.20       database.primary
```
- `mysql/info/address` -> `mysql.info.address`
- `dns/services/db/us-east/primary` -> `dns.services.db.us-east.primary`
- You get the idea


# Issues
### Minimal tolerance for bugs
Bugs in `nss-etcd` have an impact on **all** processes that do name resolution. I would highly suggest that `nss-etcd` is not run in production environments.
### Caching
`nss-etcd` does not cache (on purpose). It is possible that your application **does** cache resolution though.
### Out-of-band resolution
Most applications will use `glibc` and `gethostbyname` but it's possible some directly query DNS by reading `/etc/resolv.conf`  and performing their own resolution. This would bypass `nss` and therefore `nss-etcd`.


# Other works
- Consul provides a DNS interface to their service discovery tool.
- SkyDNS is a DNS server built on-top of etcd
