FROM ubuntu:20.04 as snr_builder

# Install dependencies
RUN apt-get update && apt-get install -y build-essential gcc-multilib python3 python3-pip python3-setuptools python3-wheel ninja-build
RUN python3 -m pip install meson
# Change example to the name of your challenge.
# Set up the user
ENV USER search_and_rescue
WORKDIR /home/$USER
RUN useradd $USER

# Add source files
COPY ./src/ /home/$USER/
# Copy the binary AND any libraries it depends on
# into the build output directory
RUN meson setup --buildtype=release build
RUN ninja -C /home/$USER/build

FROM ubuntu:20.04 as snr_container

# Install any software needed to
# build the challenge

RUN apt-get update && apt-get install -y xinetd python3 xxd

# Change example to the name of your challenge.

ENV USER search_and_rescue
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
COPY --from=snr_builder /home/$USER/build/pactc /home/$USER/
COPY --from=snr_builder /home/$USER/build/pactvm /home/$USER/
COPY --from=snr_builder /home/$USER/chals/snr.pact.template /home/$USER/
COPY --from=snr_builder /home/$USER/chals/snr_gen.py /home/$USER/

# We don't want to forget the flag!
COPY ./solve/flag.txt /home/$USER/flag.txt

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
