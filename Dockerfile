# Filename: Dockerfile
FROM debian:bookworm-20240211
WORKDIR ~/blockchain/cai/core/
RUN apt update
RUN apt -y upgrade
RUN apt install -y software-properties-common net-tools
RUN apt install -y git unzip curl ufw
RUN apt install -y build-essential libtool bsdmainutils autotools-dev autoconf pkg-config automake python3 libssl-dev libgmp-dev libboost-all-dev
RUN apt install -y libminiupnpc-dev libzmq3-dev qtbase5-dev qttools5-dev qttools5-dev-tools libqt5charts5-dev libqt5svg5-dev libprotobuf-dev protobuf-compiler libqrencode-dev
RUN apt install -y libsodium-dev cargo
RUN git clone https://github.com/libevent/libevent
RUN cd libevent && ./autogen.sh && ./configure && make && make install && cd ..
RUN git clone https://github.com/zeromq/libzmq
RUN cd libzmq && ./autogen.sh && ./configure && make && make install && cd ..
RUN git clone https://github.com/EngineApp-Project/Engine-App-Core.git engineapp
RUN cd engineapp && git checkout hotfix/upgrade
RUN cd engineapp/core && unzip plugin.zip
RUN cd engineapp/core && chmod 755 -R build-aux && chmod 755 -R depends && chmod 755 -R share && chmod 755 -R mubdi.sh
RUN cd engineapp/core && chmod 755 -R src/leveldb && chmod 755 -R src/univalue
RUN cd engineapp/core && ./mubdi.sh && ./configure --enable-cxx --with-incompatible-bdb && make && make install
COPY engineapp/core/src/mubdi* .
EXPOSE 41031
EXPOSE 41131
CMD ["eappd", "eapp-cli getinfo"]
