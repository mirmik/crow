/** @file */

#include "webui.h"
#include <crow/brocker/crow_client.h>
#include <crow/brocker/crowker.h>
#include <crow/brocker/tcp_client.h>
#include <crowhttp/app.h>
#include <crowhttp/json.h>
#include <igris/util/dstring.h>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

namespace crowker_webui
{
    static std::unique_ptr<crowhttp::SimpleApp> app;
    static std::thread server_thread;
    static std::mutex mtx;
    static bool running = false;
    static uint16_t current_port = 0;

    // Встроенная HTML страница
    static const char *INDEX_HTML = R"HTML(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Crowker Web UI</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: #1a1a2e;
            color: #eee;
            min-height: 100vh;
        }
        .header {
            background: linear-gradient(135deg, #16213e, #0f3460);
            padding: 20px 30px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.3);
        }
        .header h1 {
            font-size: 24px;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .header h1 .icon { font-size: 28px; }
        .status-badge {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 12px;
            margin-left: 15px;
            background: #4ade80;
            color: #000;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            padding: 20px;
        }
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .stat-card {
            background: #16213e;
            border-radius: 12px;
            padding: 20px;
            text-align: center;
            border: 1px solid #0f3460;
        }
        .stat-value { font-size: 36px; font-weight: bold; color: #60a5fa; }
        .stat-label { color: #888; margin-top: 5px; }
        .panel {
            background: #16213e;
            border-radius: 12px;
            margin-bottom: 20px;
            border: 1px solid #0f3460;
            overflow: hidden;
        }
        .panel-header {
            background: #0f3460;
            padding: 15px 20px;
            font-weight: bold;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .panel-body { padding: 20px; }
        table {
            width: 100%;
            border-collapse: collapse;
        }
        th, td {
            text-align: left;
            padding: 12px;
            border-bottom: 1px solid #0f3460;
        }
        th { color: #888; font-weight: 500; }
        tr:hover { background: rgba(255,255,255,0.02); }
        .theme-name {
            font-family: monospace;
            color: #fbbf24;
            cursor: pointer;
        }
        .theme-name:hover { text-decoration: underline; }
        .client-name {
            font-family: monospace;
            color: #a78bfa;
        }
        .btn {
            background: #3b82f6;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 6px;
            cursor: pointer;
            font-size: 14px;
            transition: background 0.2s;
        }
        .btn:hover { background: #2563eb; }
        .btn-sm { padding: 4px 10px; font-size: 12px; }
        .publish-form {
            display: grid;
            grid-template-columns: 1fr 2fr auto;
            gap: 15px;
            align-items: end;
        }
        .form-group label {
            display: block;
            margin-bottom: 5px;
            color: #888;
            font-size: 13px;
        }
        .form-group input, .form-group textarea {
            width: 100%;
            padding: 10px;
            border: 1px solid #0f3460;
            background: #1a1a2e;
            color: #eee;
            border-radius: 6px;
            font-family: monospace;
        }
        .form-group textarea { min-height: 80px; resize: vertical; }
        .messages-list {
            max-height: 400px;
            overflow-y: auto;
        }
        .message-item {
            background: #1a1a2e;
            border: 1px solid #0f3460;
            border-radius: 6px;
            padding: 10px 15px;
            margin-bottom: 10px;
            font-family: monospace;
            font-size: 13px;
            word-break: break-all;
        }
        .message-item:last-child { margin-bottom: 0; }
        .modal {
            display: none;
            position: fixed;
            top: 0; left: 0; right: 0; bottom: 0;
            background: rgba(0,0,0,0.7);
            z-index: 1000;
            align-items: center;
            justify-content: center;
        }
        .modal.active { display: flex; }
        .modal-content {
            background: #16213e;
            border-radius: 12px;
            width: 90%;
            max-width: 700px;
            max-height: 80vh;
            overflow: hidden;
            border: 1px solid #0f3460;
        }
        .modal-header {
            background: #0f3460;
            padding: 15px 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .modal-body { padding: 20px; overflow-y: auto; max-height: 60vh; }
        .close-btn {
            background: none;
            border: none;
            color: #888;
            font-size: 24px;
            cursor: pointer;
        }
        .close-btn:hover { color: #fff; }
        .refresh-btn {
            background: none;
            border: 1px solid #3b82f6;
            color: #3b82f6;
            padding: 6px 12px;
            border-radius: 6px;
            cursor: pointer;
            font-size: 13px;
        }
        .refresh-btn:hover { background: #3b82f6; color: white; }
        .no-data { color: #666; text-align: center; padding: 40px; }
        .toast {
            position: fixed;
            bottom: 20px;
            right: 20px;
            background: #22c55e;
            color: white;
            padding: 12px 20px;
            border-radius: 8px;
            display: none;
            z-index: 2000;
        }
        .toast.error { background: #ef4444; }
        .toast.show { display: block; animation: fadeIn 0.3s; }
        @keyframes fadeIn { from { opacity: 0; transform: translateY(10px); } to { opacity: 1; transform: translateY(0); } }
        .auto-refresh {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 13px;
            color: #888;
        }
        .auto-refresh input { width: 16px; height: 16px; }
    </style>
</head>
<body>
    <div class="header">
        <h1>
            <span class="icon">🐦</span> Crowker
            <span class="status-badge" id="statusBadge">Connecting...</span>
        </h1>
    </div>
    
    <div class="container">
        <div class="stats-grid">
            <div class="stat-card">
                <div class="stat-value" id="themesCount">-</div>
                <div class="stat-label">Тем</div>
            </div>
            <div class="stat-card">
                <div class="stat-value" id="clientsCount">-</div>
                <div class="stat-label">Клиентов</div>
            </div>
            <div class="stat-card">
                <div class="stat-value" id="crowClientsCount">-</div>
                <div class="stat-label">Crow клиентов</div>
            </div>
            <div class="stat-card">
                <div class="stat-value" id="tcpClientsCount">-</div>
                <div class="stat-label">TCP клиентов</div>
            </div>
        </div>

        <div class="panel">
            <div class="panel-header">
                <span>📤 Публикация сообщения</span>
            </div>
            <div class="panel-body">
                <div class="publish-form">
                    <div class="form-group">
                        <label for="pubTheme">Тема</label>
                        <input type="text" id="pubTheme" placeholder="/example/topic">
                    </div>
                    <div class="form-group">
                        <label for="pubData">Сообщение</label>
                        <input type="text" id="pubData" placeholder="Hello, world!">
                    </div>
                    <button class="btn" onclick="publishMessage()">Отправить</button>
                </div>
            </div>
        </div>

        <div class="panel">
            <div class="panel-header">
                <span>📚 Темы</span>
                <div style="display: flex; gap: 15px; align-items: center;">
                    <label class="auto-refresh">
                        <input type="checkbox" id="autoRefresh" checked>
                        Авто-обновление
                    </label>
                    <button class="refresh-btn" onclick="refresh()">🔄 Обновить</button>
                </div>
            </div>
            <div class="panel-body">
                <table id="themesTable">
                    <thead>
                        <tr>
                            <th>Имя темы</th>
                            <th>Подписчиков</th>
                            <th>Размер очереди</th>
                            <th>Последняя активность</th>
                            <th>Действия</th>
                        </tr>
                    </thead>
                    <tbody id="themesBody">
                        <tr><td colspan="5" class="no-data">Загрузка...</td></tr>
                    </tbody>
                </table>
            </div>
        </div>

        <div class="panel">
            <div class="panel-header">
                <span>👥 Клиенты</span>
            </div>
            <div class="panel-body">
                <table id="clientsTable">
                    <thead>
                        <tr>
                            <th>Имя / Адрес</th>
                            <th>Тип</th>
                            <th>Подтверждён</th>
                        </tr>
                    </thead>
                    <tbody id="clientsBody">
                        <tr><td colspan="3" class="no-data">Загрузка...</td></tr>
                    </tbody>
                </table>
            </div>
        </div>
    </div>

    <div class="modal" id="themeModal">
        <div class="modal-content">
            <div class="modal-header">
                <span id="modalTitle">Тема</span>
                <button class="close-btn" onclick="closeModal()">&times;</button>
            </div>
            <div class="modal-body">
                <h4 style="margin-bottom: 15px; color: #888;">Последние сообщения</h4>
                <div class="messages-list" id="messagesList">
                    <div class="no-data">Загрузка...</div>
                </div>
            </div>
        </div>
    </div>

    <div class="toast" id="toast"></div>

    <script>
        let refreshInterval = null;

        async function fetchJSON(url, options = {}) {
            try {
                const res = await fetch(url, options);
                return await res.json();
            } catch (e) {
                console.error('Fetch error:', e);
                return null;
            }
        }

        function showToast(msg, isError = false) {
            const toast = document.getElementById('toast');
            toast.textContent = msg;
            toast.className = 'toast show' + (isError ? ' error' : '');
            setTimeout(() => toast.className = 'toast', 3000);
        }

        function formatTime(timestamp) {
            if (!timestamp) return '-';
            const date = new Date(timestamp);
            return date.toLocaleTimeString();
        }

        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }

        async function refresh() {
            const status = await fetchJSON('/api/status');
            if (status) {
                document.getElementById('statusBadge').textContent = 'Online';
                document.getElementById('statusBadge').style.background = '#4ade80';
                document.getElementById('themesCount').textContent = status.themes_count;
                document.getElementById('clientsCount').textContent = status.clients_count;
                document.getElementById('crowClientsCount').textContent = status.crow_clients_count;
                document.getElementById('tcpClientsCount').textContent = status.tcp_clients_count;
            } else {
                document.getElementById('statusBadge').textContent = 'Offline';
                document.getElementById('statusBadge').style.background = '#ef4444';
            }

            const themes = await fetchJSON('/api/themes');
            const themesBody = document.getElementById('themesBody');
            if (themes && themes.themes && themes.themes.length > 0) {
                themesBody.innerHTML = themes.themes.map(t => `
                    <tr>
                        <td><span class="theme-name" onclick="showTheme('${escapeHtml(t.name)}')">${escapeHtml(t.name)}</span></td>
                        <td>${t.subscribers_count}</td>
                        <td>${t.queue_size}</td>
                        <td>${formatTime(t.activity_timestamp)}</td>
                        <td><button class="btn btn-sm" onclick="showTheme('${escapeHtml(t.name)}')">📨 Сообщения</button></td>
                    </tr>
                `).join('');
            } else {
                themesBody.innerHTML = '<tr><td colspan="5" class="no-data">Нет активных тем</td></tr>';
            }

            const clients = await fetchJSON('/api/clients');
            const clientsBody = document.getElementById('clientsBody');
            if (clients && clients.clients && clients.clients.length > 0) {
                clientsBody.innerHTML = clients.clients.map(c => `
                    <tr>
                        <td><span class="client-name">${escapeHtml(c.name || c.address || 'unnamed')}</span></td>
                        <td>${c.type}</td>
                        <td>${c.confirmed ? '✅' : '❌'}</td>
                    </tr>
                `).join('');
            } else {
                clientsBody.innerHTML = '<tr><td colspan="3" class="no-data">Нет подключённых клиентов</td></tr>';
            }
        }

        async function showTheme(name) {
            document.getElementById('themeModal').classList.add('active');
            document.getElementById('modalTitle').textContent = 'Тема: ' + name;
            document.getElementById('messagesList').innerHTML = '<div class="no-data">Загрузка...</div>';
            
            const data = await fetchJSON('/api/themes/' + encodeURIComponent(name));
            const list = document.getElementById('messagesList');
            
            if (data && data.messages && data.messages.length > 0) {
                list.innerHTML = data.messages.map(m => 
                    `<div class="message-item">${escapeHtml(m)}</div>`
                ).join('');
            } else {
                list.innerHTML = '<div class="no-data">Нет сообщений в очереди</div>';
            }
        }

        function closeModal() {
            document.getElementById('themeModal').classList.remove('active');
        }

        async function publishMessage() {
            const theme = document.getElementById('pubTheme').value.trim();
            const data = document.getElementById('pubData').value;
            
            if (!theme) {
                showToast('Введите имя темы', true);
                return;
            }
            
            const res = await fetchJSON('/api/publish', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ theme, data })
            });
            
            if (res && res.success) {
                showToast('Сообщение отправлено!');
                document.getElementById('pubData').value = '';
            } else {
                showToast('Ошибка отправки', true);
            }
        }

        document.getElementById('autoRefresh').addEventListener('change', function() {
            if (this.checked) {
                refreshInterval = setInterval(refresh, 2000);
            } else {
                clearInterval(refreshInterval);
            }
        });

        // Enter key support for publish form
        document.getElementById('pubData').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') publishMessage();
        });

        // Initial load
        refresh();
        refreshInterval = setInterval(refresh, 2000);
    </script>
</body>
</html>
)HTML";

    // JSON сериализация для тем
    static crowhttp::json::wvalue theme_to_json(const std::string &name,
                                                crowker_implementation::theme &theme)
    {
        crowhttp::json::wvalue j;
        j["name"] = name;
        j["subscribers_count"] = theme.count_clients();
        j["queue_size"] = theme.queue_size();
        j["publish_timestamp"] = theme.publish_timestamp();
        j["activity_timestamp"] = theme.activity_timestamp();
        return j;
    }

    bool start(uint16_t port, bool enable_log)
    {
        std::lock_guard<std::mutex> lock(mtx);

        if (running)
        {
            return false;
        }

        app = std::make_unique<crowhttp::SimpleApp>();

        if (!enable_log)
        {
            app->loglevel(crowhttp::LogLevel::Warning);
        }

        // Главная страница
        CROW_ROUTE((*app), "/")
        ([]() {
            crowhttp::response res;
            res.set_header("Content-Type", "text/html; charset=utf-8");
            res.body = INDEX_HTML;
            return res;
        });

        // API: Общий статус
        CROW_ROUTE((*app), "/api/status")
        ([]() {
            auto *crowker = crow::crowker::instance();
            crowhttp::json::wvalue j;

            j["themes_count"] = crowker->themes.size();

            // Подсчёт клиентов
            size_t crow_clients =
                crowker_implementation::crow_client::allsubs.size();
            size_t tcp_clients =
                crowker_implementation::tcp_client::allsubs.size();

            j["crow_clients_count"] = crow_clients;
            j["tcp_clients_count"] = tcp_clients;
            j["clients_count"] = crow_clients + tcp_clients;

            return j;
        });

        // API: Список тем
        CROW_ROUTE((*app), "/api/themes")
        ([]() {
            auto *crowker = crow::crowker::instance();
            crowhttp::json::wvalue result;
            std::vector<crowhttp::json::wvalue> themes_arr;

            for (auto &[name, theme] : crowker->themes)
            {
                themes_arr.push_back(theme_to_json(name, theme));
            }

            result["themes"] = std::move(themes_arr);
            result["count"] = crowker->themes.size();
            return result;
        });

        // API: Детали темы с сообщениями
        CROW_ROUTE((*app), "/api/themes/<string>")
        ([](const std::string &name) {
            auto *crowker = crow::crowker::instance();
            crowhttp::json::wvalue result;

            auto it = crowker->themes.find(name);
            if (it == crowker->themes.end())
            {
                result["error"] = "Theme not found";
                return crowhttp::response(404, result);
            }

            auto &theme = it->second;
            result["name"] = name;
            result["subscribers_count"] = theme.count_clients();
            result["queue_size"] = theme.queue_size();

            std::vector<crowhttp::json::wvalue> messages;
            {
                std::lock_guard<std::mutex> guard(theme.local_lock());
                auto &queue = theme.queue();
                for (size_t i = 0; i < queue.size(); ++i)
                {
                    if (queue[i])
                    {
                        // Используем dstring для читаемого представления
                        messages.push_back(
                            crowhttp::json::wvalue(igris::dstring(*queue[i])));
                    }
                }
            }
            result["messages"] = std::move(messages);

            return crowhttp::response(result);
        });

        // API: Список клиентов
        CROW_ROUTE((*app), "/api/clients")
        ([]() {
            crowhttp::json::wvalue result;
            std::vector<crowhttp::json::wvalue> clients_arr;

            // Crow клиенты
            for (auto &[addr, client] : crowker_implementation::crow_client::allsubs)
            {
                crowhttp::json::wvalue c;
                c["type"] = "crow";
                c["address"] = igris::dstring(addr);
                c["name"] = client.name();
                c["confirmed"] = client.is_confirmed();
                c["activity_timestamp"] = client.activity_timestamp();
                clients_arr.push_back(std::move(c));
            }

            // TCP клиенты
            for (auto &[addr, client] : crowker_implementation::tcp_client::allsubs)
            {
                crowhttp::json::wvalue c;
                c["type"] = "tcp";
                c["address"] = nos::format("{}:{}", addr.addr.to_string(), addr.port);
                c["name"] = client.name();
                c["confirmed"] = client.is_confirmed();
                c["activity_timestamp"] = client.activity_timestamp();
                clients_arr.push_back(std::move(c));
            }

            result["clients"] = std::move(clients_arr);
            result["count"] = clients_arr.size();
            return result;
        });

        // API: Публикация сообщения
        CROW_ROUTE((*app), "/api/publish")
            .methods(crowhttp::HTTPMethod::POST)([](const crowhttp::request &req) {
                crowhttp::json::wvalue result;

                auto body = crowhttp::json::load(req.body);
                if (!body)
                {
                    result["success"] = false;
                    result["error"] = "Invalid JSON";
                    return crowhttp::response(400, result);
                }

                std::string theme;
                std::string data;

                try
                {
                    theme = body["theme"].s();
                    data = body["data"].s();
                }
                catch (...)
                {
                    result["success"] = false;
                    result["error"] = "Missing theme or data fields";
                    return crowhttp::response(400, result);
                }

                if (theme.empty())
                {
                    result["success"] = false;
                    result["error"] = "Theme cannot be empty";
                    return crowhttp::response(400, result);
                }

                auto data_ptr = std::make_shared<std::string>(data);
                crow::crowker::instance()->publish(theme, data_ptr);

                result["success"] = true;
                result["theme"] = theme;
                return crowhttp::response(result);
            });

        // API: Установка размера очереди темы
        CROW_ROUTE((*app), "/api/themes/<string>/queue-size")
            .methods(crowhttp::HTTPMethod::POST)(
                [](const crowhttp::request &req, const std::string &name) {
                    crowhttp::json::wvalue result;

                    auto *crowker = crow::crowker::instance();
                    auto it = crowker->themes.find(name);
                    if (it == crowker->themes.end())
                    {
                        result["error"] = "Theme not found";
                        return crowhttp::response(404, result);
                    }

                    auto body = crowhttp::json::load(req.body);
                    if (!body)
                    {
                        result["error"] = "Invalid JSON";
                        return crowhttp::response(400, result);
                    }

                    size_t new_size = 0;
                    try
                    {
                        new_size = static_cast<size_t>(body["size"].i());
                    }
                    catch (...)
                    {
                        result["error"] = "Missing or invalid size field";
                        return crowhttp::response(400, result);
                    }

                    it->second.resize_queue(new_size);
                    result["success"] = true;
                    result["new_queue_size"] = new_size;
                    return crowhttp::response(result);
                });

        current_port = port;

        // Отключаем обработку сигналов в crowhttp, чтобы не блокировать
        // главный поток при Ctrl+C
        app->signal_clear();

        // Запуск сервера в отдельном потоке
        server_thread = std::thread([port]() {
            app->port(port).multithreaded().run();
        });

        running = true;
        return true;
    }

    void stop()
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (running && app)
        {
            app->stop();
            if (server_thread.joinable())
            {
                server_thread.join();
            }
            app.reset();
            running = false;
        }
    }

    bool is_running()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return running;
    }

    uint16_t get_port()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return current_port;
    }

} // namespace crowker_webui
