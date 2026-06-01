#ifndef __FEBAO_MD_VERSION_H__
#define __FEBAO_MD_VERSION_H__
#include <stdio.h>

#define FEBAO_MD_API_VERSION  "FEBAO_MD_V1.3"

inline void show_md_version() {
printf("febao_md_api_version: [%s] git_info: git_a0a70b2 2023-07-20 15:51:31 +0800 \n",FEBAO_MD_API_VERSION);
};

#endif
