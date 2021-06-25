/** @file */

#include <crow/hostaddr_view.h>
#include <crow/brocker/crowker.h>

#include <igris/util/dstring.h>
#include <nos/fprint.h>

void crow::crowker::publish(const std::string &theme, const std::string &data)
{
    int subs_count;

    auto *thm = get_theme(theme);
    subs_count = thm->count_subscribers();

    thm->publish(data);

    if (brocker_info || log_publish)
    {
        nos::fprintln("publish: t:{} s:{} d:{}", theme, subs_count,
                      igris::dstring(data));
    }
}

crowker_implementation::theme *crow::crowker::get_theme(const std::string &name)
{
    if (themes.count(name))
    {
        return &themes[name];
    }
    else
    {
        crowker_implementation::theme *thm = &themes[name];
        thm->name = name;
        return thm;
    }
}

void crow::crowker::crow_subscribe(const crow::hostaddr_view &addr,
                                   const std::string &theme, uint8_t qos,
                                   uint16_t ackquant)
{
    std::string saddr{(char *)addr.data(), (size_t)addr.size()};
    auto *thm = get_theme(theme);
    thm->timestamp_activity = crowker_eval_timestamp();

    auto *sub = crowker_implementation::crow_subscriber::get(saddr);

    // TODO: Перенести. Незачем перезаписывать адресс каждый раз.
    sub->addr = saddr;

    if (!thm->has_subscriber(sub))
    {
        thm->link_subscriber(sub);
        crowker_implementation::options *&opts = sub->thms[thm].opts;
        opts = new crowker_implementation::crow_options{qos, ackquant};

        if (brocker_info)
            nos::fprintln("new subscribe(crow): a:{} q:{} c:{} t:{}",
                          igris::dstring(addr.data(), addr.size()), qos,
                          ackquant, theme);
    }
    else
    {
        crowker_implementation::crow_options *opts =
            static_cast<crowker_implementation::crow_options *>(
                sub->thms[thm].opts);
        if (opts->qos != qos || opts->ackquant != ackquant)
        {
            opts->qos = qos;
            opts->ackquant = ackquant;
            nos::fprintln("change subscribe(crow): a:{} q:{} c:{} t:{}",
                          igris::dstring(addr.data(), addr.size()), qos,
                          ackquant, theme);
        }
    }
}

void crow::crowker::tcp_subscribe(const std::string &theme,
                                  nos::inet::tcp_socket *sock)
{

    auto *thm = get_theme(theme);
    auto *sub = crowker_implementation::tcp_subscriber::get(sock->getaddr());
    sub->sock = sock;

    if (!thm->has_subscriber(sub))
    {
        thm->link_subscriber(sub);

        if (brocker_info)
            nos::fprintln("subscribe(tcp): a:{}", sock->getaddr());
    }
}

void crow::crowker::unlink_theme_subscriber(
    crowker_implementation::theme *thm, crowker_implementation::subscriber *sub)
{
    thm->unlink_subscriber(sub);
    if (thm->count_subscribers() == 0)
    {
        themes.erase(thm->name);
    }
}