/**
	@file
*/

#ifndef G0_ACTION_H
#define G0_ACTION_H

#include <crow/tower.h>
#include <igris/event/delegate.h>

namespace crow {
	struct action_node : public node {
		igris::delegate<void, crow::packet *> dlg;

		action_node(igris::delegate<void, crow::packet *> dlg) : dlg(dlg) {}

		void incoming_packet(crow::packet *pack) override { dlg(pack); }
	};

	static inline crow::action_node *
	create_action_node(int i, igris::delegate<void, crow::packet *> dlg) {
		action_node *n = new action_node(dlg);
		crow::link_node(n, i);
		return n;
	}
} // namespace crow

#endif