#pragma once

#include <rapidjson/document.h>
#include <string>

#include "Structures.h"

inline int PAYMENT_SERVER_API = 100;

class PaymentServerInterface {
public:
    virtual ~PaymentServerInterface() = default;

    static int GetApiVersion() { return PAYMENT_SERVER_API; }

    virtual int LogsOut(const std::string& type, const std::string& message) = 0;
    virtual int SendToManager(int manager_id, const rapidjson::Value& data) = 0;
    virtual int BroadcastToManagers(const rapidjson::Value& data) = 0;
};
