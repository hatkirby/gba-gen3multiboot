cd gba
make clean
make
cd ..
mv -f gba/gba_pkjb.gba data/gba_mb.gba
make -f Makefile.gc clean
make -f Makefile.gc
make -f Makefile.wii clean
make -f Makefile.wii
pause
