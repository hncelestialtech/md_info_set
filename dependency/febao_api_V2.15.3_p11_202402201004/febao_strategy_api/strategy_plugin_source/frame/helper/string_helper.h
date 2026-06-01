/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: --
 * Date: 2019-07-11
 */

#ifndef STRING_HELPER_H
#define STRING_HELPER_H

#include <vector>
#include <string.h>
#include <functional>
#include <string>



namespace cffex {
namespace strategy {

struct string_less_functor
{
    bool operator ()(const char *s1, const char *s2) const
    {
        return 0 < strcmp(s1, s2);
    }
};

class string_helper
{
 public:
    static inline void split(const std::string &line, char delim, std::vector<std::string > *ret)
    {
        std::string::size_type begin = 0, end = -1;
        while (std::string::npos != (end = line.find(delim, begin))) {
            ret->push_back(line.substr(begin, end - begin));
            begin = end + 1;
        }
        std::string temp_str = line.substr(begin);
        // ret->push_back(line.substr(begin));
        ret->push_back(temp_str);
    }

    static void split(const char *str, char delim, const std::function<bool (const char *)> &callback)
    {
        const char *begin = str;
        const char *end = str + strlen(str);
        const char *p = strchr(str, delim);
        while(begin < end && p != NULL && p < end) {
            char tmp[128] = {0};
            strncpy(tmp, begin, p - begin);
            if (! callback(tmp)) {
                return;
            }
            begin = p + 1;
            if (begin < end) {
                p = strchr(begin, delim);
            }
        }
        if (begin < end) {
            callback(begin);
        }
    }
};

}
}

#endif
