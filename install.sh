# Removing the existing kamailio modules
dpkg -r kamailio-ims-modules
dpkg -r	kamailio-mysql-modules
dpkg -r	kamailio-presence-modules
dpkg -r	kamailio-sctp-modules
dpkg -r	kamailio-tls-modules
dpkg -r	kamailio-xml-modules
dpkg -r kamailio

# Install the new kamailio modules
dpkg -i bin/kamailio_4.3.6_amd64.deb
dpkg -i bin/kamailio-ims-modules_4.3.6_amd64.deb 
dpkg -i bin/kamailio-mysql-modules_4.3.6_amd64.deb
dpkg -i bin/kamailio-presence-modules_4.3.6_amd64.deb
dpkg -i bin/kamailio-sctp-modules_4.3.6_amd64.deb
dpkg -i bin/kamailio-tls-modules_4.3.6_amd64.deb
dpkg -i bin/kamailio-xml-modules_4.3.6_amd64.deb

