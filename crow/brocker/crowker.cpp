/** @file */

#include <crow/brocker/crowker.h>
#include <crow/hostaddr_view.h>

#include <igris/util/dstring.h>
#include <nos/fprint.h>

void crow::crowker::publish(const std::string &theme, const std::string &data)
{
    auto *thm = get_theme(theme);
    thm->publish(data);

    if (brocker_info || log_publish)
    {
        nos::fprintln("publish: t:{} s:{} d:{}", theme, thm->count_clients(),
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

void crow::crowker::subscribe(const std::string &theme, client *cl)
{
    auto *thm = get_theme(theme);
    thm->timestamp_activity = crowker_eval_timestamp();
    cl->thms[thm].opts = nullptr;
    cl->timestamp_activity = crowker_eval_timestamp();

    bool added = thm->link_client(cl);

    if (added && brocker_info)
        nos::fprintln("new subscribe(crow): t:{}", theme);
}

void crow::crowker::tcp_subscribe(const std::string &theme,
                                  nos::inet::tcp_socket *sock)
{

    auto *thm = get_theme(theme);
    auto *sub = crowker_implementation::tcp_client::get(sock->getaddr());
    sub->context = this;
    sub->sock = sock;

    if (!thm->has_client(sub))
    {
        thm->link_client(sub);

        if (brocker_info)
            nos::fprintln("subscribe(tcp): a:{}", sock->getaddr());
    }
}

void crow::crowker::unlink_theme_client(crowker_implementation::theme *thm,
                                        crowker_implementation::client *sub)
{
    thm->unlink_client(sub);
    if (thm->count_clients() == 0)
    {
        themes.erase(thm->name);
    }
}