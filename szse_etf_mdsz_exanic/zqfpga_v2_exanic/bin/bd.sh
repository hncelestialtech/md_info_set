cd ../lsrc
make clean
rm -f ../output_addr*/*.csv

make debug
make release
cd ../bin
