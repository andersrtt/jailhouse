Cross-compiling ARM bare-metal demo inmates
===========================================

Download toolchain

Download linux kernel
```
mkdir -p Downloads/linux
cd Downloads/linux
wget https://www.kernel.org/pub/linux/kernel/v4.x/linux-4.14.5.tar.xz
tar xvf linux-4.14.5.5.tar.xz
cp <jailhouse-root>/ci/kernel-config-banana-pi .config
sudo apt-get update && sudo apt-get install -y build-essential libncurses5 libncurses5-dev
make ARCH=arm menuconfig
Enable `File systems`->`FUSE`
```

Compile Kernel
```
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j8 modules dtbs LOADADDR=40008000
```

Build Jailhouse for ARM (32-bit)
```
cd <jailhouse-root>
cp ci/jailhouse-config-banana-pi.h hypervisor/include/jailhouse/config.h
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KDIR=~/Downloads/linux/linux-4.14.5/
```

Running
```
jailhouse enable /usr/share/jailhouse/cells/bananapi.cell
jailhouse cell create /usr/share/jailhouse/cells/bananapi-ivshmem-demo.cell
jailhouse cell load ivshmem-demo /usr/share/jailhouse/inmates/ivshmem-demo.bin -s "pci-cfg-base=0x02000000 ivshmem-irq=155" -a 0x100
/opt/ivshmem-utils/shmem_pump /dev/uio0 "Hello from root-cell"
jailhouse cell start ivshmem-demo
/opt/ivshmem-utils/shmem_dump /dev/uio0
/opt/ivshmem-utils/uio_send /dev/uio0 1 0 0
jailhouse cell shutdown ivshmem-demo
jailhouse cell destroy ivshmem-demo
jailhouse disable
```
