#ifndef _PLAN9_ETHER_4330_H
#define _PLAN9_ETHER_4330_H

#ifdef __cplusplus
extern "C" {
#endif

#include <device/etherif.h>
#include <stdint.h>

void etherbcmattach();
int etherbcmpnp(Ether* edev);

#ifdef __cplusplus
}
#endif

#endif