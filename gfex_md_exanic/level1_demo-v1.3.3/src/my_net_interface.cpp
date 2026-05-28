#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <map>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "my_net_interface.h"
int getNetInterface(char *deviceName,int len)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[2048];
    int success = 0;
 
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        printf("socket error\n");
        return -1;
    }
 
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        printf("ioctl error\n");
        return -1;
    }
 
    map<int,string> deviceNameMap;
    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    char szMac[64];
    int count = 0;
    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    count ++ ;
                    unsigned char * ptr ;
                    ptr = (unsigned char  *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
                    snprintf(szMac,64,"%02X:%02X:%02X:%02X:%02X:%02X",*ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));
                    printf("[%d] DEV: %s, MAC: %s, ",count,ifr.ifr_name,szMac);
                    string value = ifr.ifr_name;
                    deviceNameMap[count] = value;
                }
                //get the IP of this interface
		            if (!ioctl(sock, SIOCGIFADDR, &ifr))
		            {
		            	char ip[20] = {0};
		                snprintf(ip, sizeof(ip), "%s",(char *)inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));
		                printf("IP: %s\n", ip);
		            }
		            else
		            {
		            	printf("IP: ERR\n");
		                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
		                close(sock);
		                return -1;
		            }
            }
        }else{
            printf("get mac info error\n");
            return -1;
        }
    }
    
    if(count == 1)
    {
    	strncpy(deviceName,deviceNameMap[count].c_str(),len);
	    return 0;
    }
    
    bool isQuit = false;
    while(!isQuit){
	    printf("please select dev:");
	    string str;
	    getline(cin,str);
	    int nSel = atoi(str.c_str());
	    if(deviceNameMap.count(nSel))
	    {
	        strncpy(deviceName,deviceNameMap[nSel].c_str(),len);
	        printf("select:%d=>%s\n",nSel,deviceName);
	        isQuit = true;
	        break;
	    }
	    else
	   	{
	   		printf("error:out of range\n");
	   	}
	    
  	}
  	return 0;
}