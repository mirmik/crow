/** @file */

#include "client.h"
#include "crowker.h"

crowker_implementation::client::~client()
{
    for (auto &thm : thms)
        crow::crowker::instance()->unlink_theme_client(thm.first, this);
}