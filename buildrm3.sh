make distclean
make rm3spl_config
make -j8
cp SPL /tftpboot
cp u-boot.img /tftpboot
