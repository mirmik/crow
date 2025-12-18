/** @file */

#ifndef CROWKER_WEBUI_H
#define CROWKER_WEBUI_H

#include <cstdint>
#include <functional>
#include <string>
#include <thread>

namespace crowker_webui
{
    /**
     * @brief Initialize and start the web UI server
     * @param port HTTP port to listen on (default: 8080)
     * @param enable_log Enable HTTP request logging
     * @return true if server started successfully
     */
    bool start(uint16_t port = 8080, bool enable_log = false);

    /**
     * @brief Stop the web UI server
     */
    void stop();

    /**
     * @brief Check if web UI server is running
     */
    bool is_running();

    /**
     * @brief Get the port the server is listening on
     */
    uint16_t get_port();

} // namespace crowker_webui

#endif // CROWKER_WEBUI_H
