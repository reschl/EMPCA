#ifndef _LIBNETTOOLS_H_
#define _LIBNETTOOLS_H_

extern int if_init();
extern int if_close();

extern int if_up(char* ifname);
extern int if_down(char* ifname);
extern int if_promsic(char* ifname);

extern int set_flag(char *ifname, short flag);
extern int clr_flag(char *ifname, short flag);

#endif // _LIBNETTOOLS_H_
