/** @file */

#ifndef CROW_HOSTADDR_H
#define CROW_HOSTADDR_H

#include <cstring>
#include <igris/buffer.h>
#include <vector>

namespace crow
{
    class hostaddr_view;

    class hostaddr
    {
        std::vector<uint8_t> _addr;

      public:
        hostaddr() = default;

        const uint8_t *data() const { return _addr.data(); }
        size_t size() const { return _addr.size(); }

        hostaddr(const std::vector<uint8_t> &addr) : _addr(addr) {}

        hostaddr(const hostaddr_view &addr);

        hostaddr(const hostaddr &addr) = default;
        hostaddr(hostaddr &&addr) : _addr(std::move(addr._addr)) {}

        hostaddr(std::vector<uint8_t> &&addr) : _addr(std::move(addr)) {}

        hostaddr(uint8_t *addr, size_t len) : _addr(addr, addr + len) {}

        hostaddr &operator=(std::vector<uint8_t> &&addr)
        {
            _addr = std::move(addr);
            return *this;
        }

        hostaddr &operator=(hostaddr &&addr)
        {
            _addr = std::move(addr._addr);
            return *this;
        }

        hostaddr &operator=(const hostaddr &addr) = default;

        hostaddr &operator=(const std::vector<uint8_t> &addr)
        {
            _addr = addr;
            return *this;
        }

        bool operator==(const hostaddr &buf) const
        {
            return _addr == buf._addr;
        }

        bool operator!=(const hostaddr &buf) const
        {
            return _addr != buf._addr;
        }

        bool operator<(const hostaddr &buf) const
        {
            return std::lexicographical_compare(
                _addr.begin(), _addr.end(), buf._addr.begin(), buf._addr.end());
        }

        hostaddr_view view();

        operator hostaddr_view();
    };
}

#endif
