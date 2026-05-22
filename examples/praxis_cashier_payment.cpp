#include <PaymentInterface.h>
#include <PaymentServerInterface.h>

#include <ctime>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>

namespace {
std::string jsonString(const rapidjson::Value& obj, const char* key, const std::string& fallback = "") {
    if (!obj.IsObject() || !obj.HasMember(key) || !obj[key].IsString()) return fallback;
    return obj[key].GetString();
}

std::string configString(const std::string& config_json, const char* key, const std::string& fallback = "") {
    rapidjson::Document doc;
    if (config_json.empty() || doc.Parse(config_json.c_str()).HasParseError() || !doc.IsObject()) {
        return fallback;
    }
    return jsonString(doc, key, fallback);
}

std::string toJson(const rapidjson::Value& value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

std::string moneyToString(double amount) {
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(2);
    out << amount;
    return out.str();
}
}

class PraxisCashierPaymentProvider final : public PaymentInterface {
public:
    PraxisCashierPaymentProvider(PaymentServerInterface* server, const PaymentProviderConfigRecord& config)
        : server_(server), config_(config) {}

    std::string provider_code() const override { return "praxis"; }
    std::string provider_name() const override { return "Praxis Cashier"; }
    std::string provider_description() const override { return "Praxis-style hosted cashier redirect provider"; }
    std::string provider_payment_mode() const override { return "redirect"; }

    int CreateDeposit(const PaymentCreateRequestRecord& req, PaymentCreateResponseRecord& res) override {
        const std::string merchant_id = configString(config_.config_json, "merchant_id");
        const std::string application_key = configString(config_.config_json, "application_key");
        const std::string secret_key = configString(config_.config_json, "secret_key");
        if (merchant_id.empty() || application_key.empty() || secret_key.empty()) {
            res.status = PAYMENT_STATUS_FAILED;
            res.failure_reason = "PRAXIS_CONFIG_MISSING";
            return RET_ERR_PARAMS;
        }
        if (req.amount <= 0.0 || req.currency.empty()) {
            res.status = PAYMENT_STATUS_FAILED;
            res.failure_reason = "INVALID_PAYMENT_AMOUNT";
            return RET_ERR_PARAMS;
        }

        const std::string api_base = configString(config_.config_json, "api_base_url", config_.sandbox ? "https://sandbox.praxis.example" : "https://api.praxis.example");
        const std::string cashier_base = configString(config_.config_json, "cashier_base_url", config_.sandbox ? "https://cashier-sandbox.praxis.example" : "https://cashier.praxis.example");
        const std::string locale = configString(config_.config_json, "locale", "en");
        const std::string version = configString(config_.config_json, "api_version", "1.3");

        res.provider_payment_id = "px-" + std::to_string(req.transaction_id) + "-" + std::to_string(static_cast<long long>(std::time(nullptr)));

        rapidjson::Document payload;
        payload.SetObject();
        auto& alloc = payload.GetAllocator();
        payload.AddMember("merchant_id", rapidjson::Value(merchant_id.c_str(), alloc), alloc);
        payload.AddMember("application_key", rapidjson::Value(application_key.c_str(), alloc), alloc);
        payload.AddMember("intent", "deposit", alloc);
        payload.AddMember("version", rapidjson::Value(version.c_str(), alloc), alloc);
        payload.AddMember("trace_id", rapidjson::Value(res.provider_payment_id.c_str(), alloc), alloc);
        payload.AddMember("transaction_id", req.transaction_id, alloc);
        payload.AddMember("customer_id", req.customer_id, alloc);
        payload.AddMember("login", req.login, alloc);
        payload.AddMember("amount", rapidjson::Value(moneyToString(req.amount).c_str(), alloc), alloc);
        payload.AddMember("currency", rapidjson::Value(req.currency.c_str(), alloc), alloc);
        payload.AddMember("brand", rapidjson::Value(req.brand.c_str(), alloc), alloc);
        payload.AddMember("country", rapidjson::Value(req.country.c_str(), alloc), alloc);
        payload.AddMember("payment_method", rapidjson::Value(req.method.c_str(), alloc), alloc);
        payload.AddMember("locale", rapidjson::Value(locale.c_str(), alloc), alloc);
        payload.AddMember("success_url", rapidjson::Value(req.success_url.c_str(), alloc), alloc);
        payload.AddMember("failure_url", rapidjson::Value(req.failure_url.c_str(), alloc), alloc);
        payload.AddMember("idempotency_key", rapidjson::Value(req.idempotency_key.c_str(), alloc), alloc);

        rapidjson::Document metadata;
        if (!req.metadata_json.empty() && !metadata.Parse(req.metadata_json.c_str()).HasParseError() && metadata.IsObject()) {
            rapidjson::Value metadata_copy;
            metadata_copy.CopyFrom(metadata, alloc);
            payload.AddMember("metadata", metadata_copy, alloc);
        }

        const std::string payload_json = toJson(payload);

        if (server_) {
            server_->LogsOut("PAYMENT", "Praxis CreateDeposit payload prepared: " + payload_json);
        }

        // Real provider implementation should POST payload_json to api_base and use
        // the returned cashier session/redirect URL. The SDK example keeps network
        // transport out of the sample and shows the normalized server contract.
        res.redirect_url = cashier_base + "/checkout/" + res.provider_payment_id;
        res.instructions_json =
            "{\"type\":\"redirect\",\"provider\":\"praxis\",\"api_base_url\":\"" + api_base +
            "\",\"trace_id\":\"" + res.provider_payment_id + "\"}";
        res.raw_response_json =
            "{\"provider\":\"praxis\",\"mode\":\"sdk-example\",\"request\":" + payload_json +
            ",\"redirect_url\":\"" + res.redirect_url + "\"}";
        res.status = PAYMENT_STATUS_PENDING;
        return RET_OK;
    }

    int VerifyWebhook(const PaymentWebhookRequestRecord& req, PaymentWebhookResultRecord& res) override {
        rapidjson::Document payload;
        if (req.raw_body.empty() || payload.Parse(req.raw_body.c_str()).HasParseError() || !payload.IsObject()) {
            res.signature_status = PAYMENT_SIGNATURE_INVALID;
            res.failure_reason = "INVALID_WEBHOOK_PAYLOAD";
            return RET_INVALID_DATA;
        }

        // Praxis integrations normally validate headers/signature with webhook_secret.
        // This sample marks configured webhooks as valid and focuses on status mapping.
        const std::string webhook_secret = configString(config_.config_json, "webhook_secret");
        res.signature_status = webhook_secret.empty() ? PAYMENT_SIGNATURE_UNKNOWN : PAYMENT_SIGNATURE_VALID;
        res.event_id = jsonString(payload, "event_id", jsonString(payload, "trace_id"));
        res.provider_payment_id = jsonString(payload, "transaction_id", jsonString(payload, "payment_id"));
        res.raw_status = jsonString(payload, "status");
        if (payload.HasMember("amount") && payload["amount"].IsNumber()) res.amount = payload["amount"].GetDouble();
        res.currency = jsonString(payload, "currency");
        res.normalized_payload_json = req.raw_body;

        if (res.raw_status == "approved" || res.raw_status == "success") {
            res.status = PAYMENT_STATUS_CONFIRMED;
        } else if (res.raw_status == "declined" || res.raw_status == "failed") {
            res.status = PAYMENT_STATUS_FAILED;
        } else if (res.raw_status == "cancelled") {
            res.status = PAYMENT_STATUS_CANCELLED;
        } else if (res.raw_status == "refunded") {
            res.status = PAYMENT_STATUS_REFUNDED;
        } else if (res.raw_status == "chargeback") {
            res.status = PAYMENT_STATUS_CHARGEBACK;
        } else {
            res.status = PAYMENT_STATUS_PENDING;
        }
        return RET_OK;
    }

private:
    PaymentServerInterface* server_;
    PaymentProviderConfigRecord config_;
};

extern "C" int GetPaymentApiVersion() {
    return PaymentServerInterface::GetApiVersion();
}

extern "C" PaymentInterface* CreatePaymentProvider(PaymentServerInterface* server, const PaymentProviderConfigRecord& config) {
    const std::string merchant_id = configString(config.config_json, "merchant_id");
    const std::string application_key = configString(config.config_json, "application_key");
    const std::string secret_key = configString(config.config_json, "secret_key");
    if (merchant_id.empty() || application_key.empty() || secret_key.empty()) {
        return nullptr;
    }
    return new PraxisCashierPaymentProvider(server, config);
}

extern "C" void DestroyPaymentProvider(PaymentInterface* provider) {
    delete provider;
}
