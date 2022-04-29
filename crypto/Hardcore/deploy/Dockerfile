# We can build from whatever version
# we want. 20.04 is most typical now.

# FROM ubuntu:18.04
FROM ubuntu:20.04

# Install any software needed to
# build the challenge

RUN apt-get update
RUN apt-get install -y xinetd
RUN apt-get update
RUN apt-get install python3-pip -y

# Change example to the name of your challenge.

ENV USER Hardcore
WORKDIR /home/$USER
RUN useradd $USER

# This adds the critical files.

COPY ./deploy/requirements.txt .
RUN pip install -r requirements.txt

# wrapper.sh wraps the executable by 
# `cd`ing to the right place

COPY ./deploy/wrapper.sh /home/$USER/

# The xinetd configuration provides run options
# but is very boilerplate. See below. 
COPY ./deploy/xinetd.conf /etc/xinetd.d/$USER

# This assumes a C file but works perfectly OK
# with go/C++/rust, just install the right compiler
COPY ./src/* /home/$USER/

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