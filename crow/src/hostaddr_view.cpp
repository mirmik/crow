#include <crow/hostaddr_view.h>
#include <crow/hostaddr.h>
#include <cstring>

crow::hostaddr_view::hostaddr_view(const hostaddr &h) : addr(h.data()), alen(h.size())
{
}

crow::hostaddr_view::hostaddr_view(const std::string_view &v)
    : addr((uint8_t *)v.data()), alen(v.size())
{
}

crow::hostaddr_view::hostaddr_view(const uint8_t *addr, size_t alen) : addr(addr), alen(alen)
{
}

crow::hostaddr_view::hostaddr_view(const char *addr, size_t alen)
    : addr((uint8_t *)addr), alen(alen)
{
}

crow::hostaddr_view::hostaddr_view(const void *addr, size_t alen)
    : addr((uint8_t *)addr), alen(alen)
{
}

template <class Alloc>
crow::hostaddr_view::hostaddr_view(const std::vector<uint8_t, Alloc> &v)
    : addr(v.data()), alen(v.size())
{
}

const uint8_t *crow::hostaddr_view::data() const
{
    return addr;
}

size_t crow::hostaddr_view::size() const
{
    return alen;
}

bool crow::hostaddr_view::operator==(std::string_view buf) const
{
    return alen == buf.size() && memcmp(addr, buf.data(), alen) == 0;
}

bool crow::hostaddr_view::operator==(const hostaddr_view &buf) const
{
    return alen == buf.size() && memcmp(addr, buf.data(), alen) == 0;
}