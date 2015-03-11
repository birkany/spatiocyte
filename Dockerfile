FROM ubuntu:14.04
RUN apt-get update; apt-get install -y git autoconf automake libtool g++ libgsl0-dev python-numpy python-ply python-gtk2-dev libboost-dev libboost-python-dev libgtkmm-2.4-dev libgtkglextmm-x11-1.2-dev libhdf5-dev openssh-server valgrind; git clone git://github.com/ecell/spatiocyte; cd /spatiocyte; ./autogen.sh; ./configure; make; make install
ENV LD_LIBRARY_PATH /usr/local/lib

RUN mkdir /var/run/sshd
RUN echo 'root:screencast' |chpasswd
RUN sed -ri 's/PermitRootLogin without-password/PermitRootLogin yes/g' /etc/ssh/sshd_config

EXPOSE 22
CMD    ["/usr/sbin/sshd", "-D"]
