#ifndef CROW_HOSTADDR_H
#define CROW_HOSTADDR_H

#include <cstring>
#include <string_view>
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

        hostaddr(std::vector<uint8_t> &&addr) : _addr(std::move(addr)) {}

        hostaddr(uint8_t *addr, size_t len) : _addr(addr, addr + len) {}

        hostaddr &operator=(std::vector<uint8_t> &&addr)
        {
            _addr = std::move(addr);
            return *this;
        }

        hostaddr &operator=(const std::vector<uint8_t> &addr)
        {
            _addr = addr;
            return *this;
        }

        hostaddr_view view();

        operator hostaddr_view();
    };

    class hostaddr_view
    {
      public:
        const uint8_t *addr;
        size_t alen;

        hostaddr_view() = default;
        hostaddr_view(const hostaddr &h) : addr(h.data()), alen(h.size()) {}
        hostaddr_view(const std::string_view &v)
            : addr((uint8_t *)v.data()), alen(v.size())
        {
        }
        hostaddr_view(const uint8_t *addr, size_t alen) : addr(addr), alen(alen)
        {
        }
        hostaddr_view(const char *addr, size_t alen)
            : addr((uint8_t *)addr), alen(alen)
        {
        }
        hostaddr_view(const void *addr, size_t alen)
            : addr((uint8_t *)addr), alen(alen)
        {
        }

        template <class Alloc>
        hostaddr_view(const std::vector<uint8_t, Alloc> &v)
            : addr(v.data()), alen(v.size())
        {
        }

        const uint8_t *data() const { return addr; }
        size_t size() const { return alen; }

        bool operator==(std::string_view buf) const
        {
            return alen == buf.size() && memcmp(addr, buf.data(), alen) == 0;
        }

        bool operator==(const hostaddr_view &buf) const
        {
            return alen == buf.size() && memcmp(addr, buf.data(), alen) == 0;
        }
    };
}

#endif
