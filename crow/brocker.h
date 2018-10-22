/**
	@file
*/

#ifndef CROW_BROCKER_H
#define CROW_BROCKER_H

#include <crow/pubsub.h>

__BEGIN_DECLS

int brocker_publish(
	const void * thm, size_t thmsz, 
	const void * dat, size_t datsz
);

int crow_brocker_init();

__END_DECLS

#endif
