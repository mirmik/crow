#ifndef G1_SELFGATE_H
#define G1_SELFGATE_H

#include <crow/gateway.h>
#include <crow/indexes.h>

namespace crow {
	struct selfgate : public gateway {
		void send(crow::packet* pack) override {
			pack->ingate = this;
			pack->revert_stage(G1_SELFGATE);
			crow::travel(pack);	
		}
	};
}

#endif