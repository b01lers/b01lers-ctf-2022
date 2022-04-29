# We can build from whatever version
# we want. 20.04 is most typical now.

# FROM ubuntu:18.04
FROM ubuntu:20.04

# Install any software needed to
# build the challenge

RUN apt-get update && apt-get install -y xinetd build-essential gcc-multilib

# Change example to the name of your challenge.

ENV USER gambler_overflow
WORKDIR /home/$USER
RUN useradd $USER

# This adds the critical files.

# wrapper.sh wraps the executable by 
# `cd`ing to the right place

COPY ./deploy/wrapper.sh /home/$USER/

# The xinetd configuration provides run options
# but is very boilerplate. See below. 
COPY ./deploy/xinetd.conf /etc/xinetd.d/$USER

# This assumes a C file but works perfectly OK
# with go/C++/rust, just install the right compiler
COPY ./src/$USER.c /home/$USER/

# This makefile provides the build and outputs
# the challenge binary (see below)
COPY ./src/Makefile /home/$USER/Makefile

# We don't want to forget the flag!
COPY ./solve/flag.txt /home/$USER/flag.txt

# This runs make in the challenge dir
RUN make -C /home/$USER/

# Set permissions. Be *VERY* careful
# about changing this!
RUN chown -R root:$USER /home/$USER
RUN chmod -R 550 /home/$USER
RUN chmod -x /home/$USER/flag.txt
RUN touch /var/log/xinetdlog

# Whatever port you configured in xinetd.
# PROBABLY this should stay 1337. Just change
# The passthrough port in docker-compose.yml below.
EXPOSE 1337

# Start the container by starting xinetd and outputting
# the xinetd log for debugging.
CMD service xinetd start && sleep 2 && tail -f /var/log/xinetdlog
