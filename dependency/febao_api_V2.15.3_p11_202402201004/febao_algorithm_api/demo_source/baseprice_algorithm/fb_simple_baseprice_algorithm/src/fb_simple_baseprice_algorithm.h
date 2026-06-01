/**
 * CFFEX Confidential.
 *
 * @Copyright 2018 CFFEX.  All rights reserved.
 *
 * The source code for this program is not published or otherwise
 * divested of its trade secrets, irrespective of what has been
 * deposited with the China Copyright Office.
 *
 * Author: zhr
 * Date: 2018-03-20
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "i_baseprice_algorithm.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*register_func)(void *handle, unsigned short id, const char *name);

void* create();
void destroy(void* p);
void identity(void* handle,register_func f);

#ifdef __cplusplus
}
#endif

namespace cffex {
namespace fb {
namespace plugin {

class fb_simple_baseprice_algorithm : public i_baseprice_algorithm {
public:
    fb_simple_baseprice_algorithm();
    virtual ~fb_simple_baseprice_algorithm();
    virtual int calculate(unsigned short algo_id, IN instrument_field* target_instrument, IN const instrument_md_field_container *mds, OUT double *baseprice);
    virtual int calculate(unsigned short algo_id, IN serial_field* target_serial, IN const instrument_md_field_container *mds, OUT double *baseprice) ;
    virtual int calculate(unsigned short algo_id, IN instrument_field *target_fund_instrument, IN const instrument_md_field_container *mds,
                          IN const component_md_field_container *component_mds, OUT double *baseprice);

private:
    int calculate_inner(unsigned short algo_id, IN const instrument_md_field_container *mds, double *baseprice);
    int calculate_fund_by_underlying(IN const instrument_md_field_container *instrument_mds, OUT double *baseprice);
    int calculate_fund_by_component(IN const component_md_field_container *component_mds, IN const instrument_field *fund_instrument, OUT double *baseprice);

    int calculate_arbi(unsigned short algo_id, IN const instrument_md_field_container *mds, OUT double *baseprice);

};

}
}
}


//g++ -g -O2 -o mm_middle_spot_algorithm.so ./src/mm_middle_spot_algorithm.cc  -I./src -I./interface -shared  -fPIC

