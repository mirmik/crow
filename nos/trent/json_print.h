#ifndef NOS_JSON_PRINT_H
#define NOS_JSON_PRINT_H

#include <iostream>
#include <nos/io/std_adapter.h>
#include <nos/io/test_width_ostream.h>
#include <nos/trent/json.h>
#include <sstream>
#include <string>

namespace nos
{
    namespace json
    {
        namespace detail
        {
            inline void write_escaped_string(const std::string &str,
                                             std::ostream &os)
            {
                static const char hex[] = "0123456789ABCDEF";
                os.put('"');
                for (unsigned char c : str)
                {
                    switch (c)
                    {
                    case '"':
                        os << "\\\"";
                        break;
                    case '\\':
                        os << "\\\\";
                        break;
                    case '\b':
                        os << "\\b";
                        break;
                    case '\f':
                        os << "\\f";
                        break;
                    case '\n':
                        os << "\\n";
                        break;
                    case '\r':
                        os << "\\r";
                        break;
                    case '\t':
                        os << "\\t";
                        break;
                    default:
                        if (c < 0x20)
                        {
                            os << "\\u00" << hex[(c >> 4) & 0x0F]
                               << hex[c & 0x0F];
                        }
                        else
                        {
                            os.put(static_cast<char>(c));
                        }
                        break;
                    }
                }
                os.put('"');
            }
        }

        inline void
        print_to(const nos::trent &tr, std::ostream &os, bool pretty, int tab)
        {
            bool sep = false;
            bool havedict;

            switch (tr.get_type())
            {
            case trent::type::numer:
                os << tr.unsafe_numer_const();
                break;

            case trent::type::boolean:
                os << (tr.unsafe_bool_const() ? "true" : "false");
                break;

            case trent::type::string:
                detail::write_escaped_string(tr.unsafe_string_const(), os);
                break;

            case trent::type::list:
                havedict = false;

                for (const auto &m : tr.unsafe_list_const())
                {
                    if (m.get_type() == trent::type::dict)
                    {
                        havedict = true;
                        break;
                    }
                }

                os.put('[');

                if (havedict)
                    for (auto &v : tr.unsafe_list_const())
                    {
                        if (sep)
                        {
                            if (pretty)
                                os << ", ";
                            else
                                os.put(',');
                        }

                        json::print_to(v, os, pretty, tab + 1);
                        sep = true;
                    }
                else
                {
                    for (auto &v : tr.unsafe_list_const())
                    {
                        if (sep)
                            os.put(',');

                        if (pretty)
                        {
                            os << "\r\n";
                            for (int i = 0; i < tab + 1; i++)
                                os.put('\t');
                        }

                        if (pretty)
                        {
                            nos::test_width_ostream testwidth;
                            nos::std_ostream_adapter adapter =
                                nos::make_std_adapter(testwidth);
                            json::print_to(v, adapter, false, 0);
                            if (testwidth.result > 20)
                                json::print_to(v, os, true, tab + 1);
                            else
                                json::print_to(v, os, false, tab + 1);
                        }
                        else
                            json::print_to(v, os, pretty, tab + 1);

                        sep = true;
                    }

                    if (pretty)
                    {
                        os << "\r\n";

                        for (int i = 0; i < tab; i++)
                            os.put('\t');
                    }
                }

                os.put(']');
                break;

            case trent::type::dict:
                os.put('{');

                for (auto &p : tr.unsafe_dict_const())
                {
                    if (sep)
                        os.put(',');

                    if (pretty)
                    {
                        os.put('\n');

                        for (int i = 0; i < tab + 1; i++)
                            os.put('\t');
                    }

                    detail::write_escaped_string(p.first, os);
                    if (pretty)
                        os.write(": ", 2);
                    else
                        os.put(':');

                    if (pretty)
                    {
                        nos::test_width_ostream testwidth;
                        nos::std_ostream_adapter adapter =
                            nos::make_std_adapter(testwidth);
                        json::print_to(p.second, adapter, false, 0);
                        if (testwidth.result > 20)
                            json::print_to(p.second, os, true, tab + 1);
                        else
                            json::print_to(p.second, os, false, tab + 1);
                    }
                    else
                        json::print_to(p.second, os, pretty, tab + 1);
                    sep = true;
                }

                if (pretty)
                {
                    os.put('\n');
                    for (int i = 0; i < tab; i++)
                        os.put('\t');
                }

                os.put('}');
                break;

            case trent::type::nil:
                os.write("nil", 3);
                break;
            }

            if (pretty)
                if (tab == 0)
                    os << "\r\n";
        }

        inline std::string to_string(const nos::trent &tr, bool pretty = false)
        {
            std::stringstream ss;
            print_to(tr, ss, pretty, 0);
            return ss.str();
        }

        inline std::string to_pretty_string(const nos::trent &tr)
        {
            std::stringstream ss;
            print_to(tr, ss, true, 0);
            return ss.str();
        }

        inline void pretty_print_to(const nos::trent &tr, std::ostream &os)
        {
            print_to(tr, os, true, 0);
        }
    }
}

#endif
