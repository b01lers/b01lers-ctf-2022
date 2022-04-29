# We can build from whatever version
# we want. 20.04 is most typical now.

# FROM ubuntu:18.04
FROM python:3.10.0rc2-bullseye

# Install any software needed to
# build the challenge

RUN apt-get update
RUN apt-get install -y xinetd curl git build-essential
RUN pip3 install git+https://github.com/novafacing/asm_practice.git --force-reinstall
RUN pip3 uninstall qiling unicorn -y
RUN pip3 install qiling==1.2.4

# Change example to the name of your challenge.

ENV USER extreme_64_part_2
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
COPY ./src/* /home/$USER/

WORKDIR /home/$USER

# Set permissions. Be *VERY* careful
# about changing this!
RUN chown -R root:$USER /home/$USER
RUN chmod -R 550 /home/$USER
RUN touch /var/log/xinetdlog

# Whatever port you configured in xinetd.
# PROBABLY this should stay 1337. Just change
# The passthrough port in docker-compose.yml below.
EXPOSE 1337

# Start the container by starting xinetd and outputting
# the xinetd log for debugging.
CMD service xinetd start && sleep 2 && tail -f /var/log/xinetdlog
