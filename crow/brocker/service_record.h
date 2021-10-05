/// @file
/// Список сервисов зарегистрированных в системе.

namespace crow
{
    class service_client
    {
    };

    class service_crow_client : public service_client
    {
        crow::hostaddr requestor_host;
        int requestor_node;
    }

    class service_request
    {
        service_record *record;
        int request_id;
        int reply_id;
        std::shared_ptr<service_client> client;
    }

    class service_record
    {
        std::string theme;
        std::unique_ptr<service_client> client;
        std::unordered_map<int, service_request> requests;
    };
}