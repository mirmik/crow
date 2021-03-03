#include "subscriber.h"
#include "crowker.h"

crowker_implementation::subscriber::~subscriber()
{
	for (auto& thm : thms)
		crow::crowker::instance()->unlink_theme_subscriber(thm.first, this);
}