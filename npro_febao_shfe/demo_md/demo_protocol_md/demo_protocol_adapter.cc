#include "demo_protocol_adapter.h"

int demo_protocol_adapter::init()
{
    return 1;
}

void demo_protocol_adapter::start()
{
    // busyloop
    while (!stop_) {
        // receive data
        // handle data
    }
}

void demo_protocol_adapter::handle_message(void *data, size_t len)
{
    // callback to demo_protocol_md
}
