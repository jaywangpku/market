# install glog/logging
cd ../rely
git clone https://github.com/jeewang/glog.git
cd glog
# sudo apt-get install autoconf automake libtool
./autogen.sh && ./configure && make && make install
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

