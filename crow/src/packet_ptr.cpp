#include <crow/packet_ptr.h>
#include <crow/tower_cls.h>

#include <assert.h>

crow::packet_ptr::~packet_ptr() { clear(); }

void crow::packet_ptr::clear()
{
    if (pack)
    {
        system_lock();
        pack->refs--;
        assert(pack->refs >= 0);
        bool should_release = (pack->refs == 0);
        system_unlock();

        if (should_release)
            crow::Tower::release(pack);

        pack = nullptr;
    }
}
