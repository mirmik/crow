#ifndef CROW_SELECT_H
#define CROW_SELECT_H

namespace crow 
{
	void select_collect_fds();
	void add_select_fd(int fd);
	void select();
}

#endif