make distclean
make re3splandroid_config
make -j8
cp SPL /tftpboot
cp u-boot.img /tftpboot
