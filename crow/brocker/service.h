#ifndef CROW_BROCKER_SERVICE_H
#define CROW_BROCKER_SERVICE_H

#include <crow/brocker/client.h>
#include <memory>

namespace crowker_implementation
{
    class service
    {
        class record
        {
            std::shared_ptr<client> owner;
        }

        std::weak_ptr<client>
            owner;

        int16_t brocker_to_service_id_counter = 0;
        std::list<int16_t, >
    };
}

#endif