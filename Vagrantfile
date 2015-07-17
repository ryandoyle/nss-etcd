# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "ubuntu/trusty64"
  config.vm.network "forwarded_port", guest: 2379, host: 2379
  config.vm.provision 'shell', :inline => '
  curl -L  https://github.com/coreos/etcd/releases/download/v2.1.0-rc.0/etcd-v2.1.0-rc.0-linux-amd64.tar.gz -o etcd-v2.1.0-rc.0-linux-amd64.tar.gz
  tar xzvf etcd-v2.1.0-rc.0-linux-amd64.tar.gz
  cd etcd-v2.1.0-rc.0-linux-amd64
  ./etcd &
  '
end
