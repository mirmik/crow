export PWD0=$PWD

cd $PWD0/crowker
./make.py && ./make.py install

cd $PWD0/ctrans
./make.py && ./make.py install

cd $PWD0/crowpulse
./make.py && ./make.py install