# all steps in sequence to be done

# Goto kernel source directory
cd ~/Downloads/linux-4.12.4/;

# Do your configuration
make defconfig;

# Compile the main kernel
make;

# Compile the kernel modules
make modules;

# Install the kernel modules
make modules_install;

# Install the new kernel on the system
sudo make install;

# Boot it!
sudo reboot;
