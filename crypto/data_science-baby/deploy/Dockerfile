# We can build from whatever version
# we want. 20.04 is most typical now.

FROM ubuntu:20.04
#FROM python:3

# Install any software needed to
# build the challenge

#RUN apt-get install -y xinetd python3-pip
#RUN apt-get install -y xinetd python3
RUN apt-get update && apt-get install -y gcc make xinetd

# Change example to the name of your challenge.

ENV USER data_science-baby
WORKDIR /home/$USER
RUN useradd $USER

# This adds the critical files.

#COPY ./deploy/requirements.txt .
#RUN pip3 install -r requirements.txt

# wrapper.sh wraps the executable by 
# `cd`ing to the right place

COPY ./deploy/wrapper.sh ./

# The xinetd configuration provides run options
# but is very boilerplate. See below. 
COPY ./deploy/$USER.xinetd /etc/xinetd.d/$USER

COPY ./src/* ./
RUN make

# We don't want to forget the flag! -> already in ./src
#COPY ./solve/flag.txt /home/$USER/flag.txt

# Set permissions. Be *VERY* careful
# about changing this!
RUN chown -R root:$USER /home/$USER
RUN chmod -R 550 /home/$USER
RUN chmod -x /home/$USER/flag.txt /home/$USER/*.c /home/$USER/Makefile
RUN touch /var/log/xinetdlog

# Whatever port you configured in xinetd.
# PROBABLY this should stay 1337. Just change
# The passthrough port in docker-compose.yml below.
EXPOSE 2001

# Start the container by starting xinetd and outputting
# the xinetd log for debugging.
ENTRYPOINT service xinetd start && sleep 2 && tail -f /var/log/xinetdlog
