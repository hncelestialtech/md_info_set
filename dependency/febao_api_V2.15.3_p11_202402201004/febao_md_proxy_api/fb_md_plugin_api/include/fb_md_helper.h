/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: renjh
 * Date: 2022-05-16
 */

#ifndef FB_MD_HELPER_H
#define FB_MD_HELPER_H

#include <string>
#include <vector>

typedef enum {
    FB_XLOG_FATAL   = 0X01,
    FB_XLOG_ERROR   = 0X02,
    FB_XLOG_WARNING = 0X04,
    FB_XLOG_INFO    = 0X08,
    FB_XLOG_DEBUG   = 0X10,
    FB_XLOG_TRACE   = 0X20,
    FB_XLOG_ALL     = 0XFF
} FB_XLOG_LEVEL;

#define MD_XLOG(xlog_helper, level, fmt, ...)                                          \
    do {                                                                               \
        if (NULL != xlog_helper && level == (level & xlog_helper->get_xlog_level())) { \
            xlog_helper->xlog(level, fmt, ##__VA_ARGS__);                              \
        }                                                                              \
    } while (0)

#define FB_SET_GUID_TAG fb_md_config_helper::set_guid_tag

#ifndef OUT
#define OUT
#endif

namespace cffex {
namespace fb {
namespace api {

class fb_md_config_helper {
public:
    enum { FB_MAX_ELEMENT_COUNT = 0XFFFF };
    enum { FB_MAX_ELEMENT_LEN = 0XFF };

    static uint64_t set_guid_tag();

    virtual ~fb_md_config_helper() {}

    virtual void get_attribute(const char *name, const char *path, OUT std::string &out) = 0;
    virtual void get_attribute_elements(const char *name,
                                        const char *path,
                                        OUT std::vector<std::string> &out)               = 0;

    virtual char *get_attribute(const char *name, const char *path) = 0;
    virtual int   get_attribute_elements(const char *name,
                                         const char *path,
                                         OUT char (*elements)[FB_MAX_ELEMENT_LEN],
                                         OUT int &nCount)           = 0;

    virtual void parse_passwd(std::string raw_passwd, std::string encrypt, std::string &passwd) = 0;
};

class fb_md_xlog_helper {
public:
    virtual ~fb_md_xlog_helper() {}
    virtual void    xlog(uint8_t loglevel, const char *fmt, ...) = 0;
    virtual void    set_xlog_level(const char *level)            = 0;
    virtual uint8_t get_xlog_level()                             = 0;
};

}  // namespace api
}  // namespace fb
}  // namespace cffex

#endif
