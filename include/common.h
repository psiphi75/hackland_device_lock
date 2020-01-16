
#ifndef _COMMON_H_
#define _COMMON_H_

#define TIME_24_HOURS_IN_SEC (24 * 60 * 60)

// This get's set to the universal MAC address of the ESP at boot time.
#define MAC_LEN         6
#define MAX_TOPIC_LEN   256

void set_my_uuid();
char *get_uuid();

#endif // _COMMON_H_