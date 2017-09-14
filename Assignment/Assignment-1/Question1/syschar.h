#ifndef __SYSCHAR_H_
#define __SYSCHAR_H_

#define DEV_MAJOR 247

#define IOCTL_LOGIN_PROCESS _IOR(DEV_MAJOR, 0, unsigned char *)
#define IOCTL_LOGOUT_PROCESS _IOR(DEV_MAJOR, 1, unsigned int *)
#endif
