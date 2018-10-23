/**
	@file
*/

#ifndef CROW_HOST_H
#define CROW_HOST_H

#include <string.h>

/*namespace crow {
	struct host {
		uint8_t* data;
		size_t size;
		host() = default;
		host(const host& oth);
		host(const uint8_t* addr, size_t size);
		host(const char* str);
		host& operator=(const host& oth) { 
			data = (uint8_t*)malloc(oth.size);
			memcpy(data, oth.data, oth.size);
			size = oth.size;
			return *this;
		}

		bool operator!= (const host& oth) const {
			return size != oth.size || memcmp(data, oth.data, size) != 0;
		}

		bool operator== (const host& oth) const {
			return size == oth.size && memcmp(data, oth.data, size) == 0;
		}

		~host();
	};
}*/

#endif