# Filename: Dockerfile

FROM ubuntu

RUN apt-get update && apt-get install -y \
	nano \
	build-essential \ 
	csh && apt-get clean

ADD . /opt/rom

RUN cd /opt/rom/src && make -k
RUN mkdir -p /opt/rom/log
RUN mkdir -p /opt/rom/player
RUN mkdir -p /opt/rom/json/areas

WORKDIR /opt/rom

VOLUME [ "/opt/rom" ]
EXPOSE 4000

ENTRYPOINT [ "./run.sh" ]