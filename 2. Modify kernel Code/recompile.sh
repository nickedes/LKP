# Steps to only recompile the kernel, modules will not be installed again!
# Make suitable changes to kernel code in fs/open.c

# Goto kernel source directory
cd ~/Downloads/linux-4.12.4/;

# create the linux kernel image file
make bzImage;

# Copy the new kernel and replace the old one
sudo cp arch/x86/boot/bzImage /boot/vmlinuz-$(uname -r);

# Boot it!!
sudo reboot;