#ifndef __SYSCHAR_H_
#define __SYSCHAR_H_

#define DEV_MAJOR 247

#define IOCTL_LOGIN _IOR(DEV_MAJOR, 0, unsigned char *)
#define IOCTL_LOGOUT _IOR(DEV_MAJOR, 1, unsigned int *)
#define CHECK_LOGIN _IOR(DEV_MAJOR, 2, unsigned char *)
#define LOGIN_CA _IOW(DEV_MAJOR, 3, unsigned int)
#define IOCTL_SETPAIR _IOW(DEV_MAJOR, 4, unsigned char *)
#endif
