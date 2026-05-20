#pragma once

#include <ctime>
#include <string>

enum PaymentTransactionType {
    PAYMENT_TRANSACTION_DEPOSIT = 0,
    PAYMENT_TRANSACTION_WITHDRAWAL = 1,
};

enum PaymentNormalizedStatus {
    PAYMENT_STATUS_UNKNOWN = 0,
    PAYMENT_STATUS_PENDING = 1,
    PAYMENT_STATUS_CONFIRMED = 2,
    PAYMENT_STATUS_FAILED = 3,
    PAYMENT_STATUS_CANCELLED = 4,
    PAYMENT_STATUS_REFUNDED = 5,
    PAYMENT_STATUS_CHARGEBACK = 6,
};

enum PaymentSignatureStatus {
    PAYMENT_SIGNATURE_UNKNOWN = 0,
    PAYMENT_SIGNATURE_VALID = 1,
    PAYMENT_SIGNATURE_INVALID = 2,
};

struct PaymentProviderConfigRecord {
    std::string provider;
    std::string brand;
    std::string merchant_id;
    std::string public_key;
    std::string secret_key;
    std::string webhook_secret;
    std::string settings_json;
    int sandbox = 0;
};

struct PaymentCreateRequestRecord {
    int transaction_id = 0;
    int customer_id = 0;
    int login = 0;
    int type = PAYMENT_TRANSACTION_DEPOSIT;
    double amount = 0.0;
    std::string currency;
    std::string account_currency;
    std::string method;
    std::string brand;
    std::string country;
    std::string idempotency_key;
    std::string success_url;
    std::string failure_url;
    std::string metadata_json;
    PaymentProviderConfigRecord config;
};

struct PaymentCreateResponseRecord {
    std::string provider_payment_id;
    std::string redirect_url;
    std::string instructions_json;
    std::string raw_response_json;
    int status = PAYMENT_STATUS_PENDING;
    std::string failure_reason;
};

struct PaymentWebhookRequestRecord {
    std::string provider;
    std::string path;
    std::string method;
    std::string headers_json;
    std::string payload;
    PaymentProviderConfigRecord config;
};

struct PaymentWebhookResultRecord {
    std::string event_id;
    std::string provider_payment_id;
    int status = PAYMENT_STATUS_UNKNOWN;
    int signature_status = PAYMENT_SIGNATURE_UNKNOWN;
    double amount = 0.0;
    std::string currency;
    std::string raw_status;
    std::string normalized_payload_json;
    std::string failure_reason;
};

struct PaymentStatusRequestRecord {
    std::string provider_payment_id;
    PaymentProviderConfigRecord config;
};

struct PaymentStatusResponseRecord {
    int status = PAYMENT_STATUS_UNKNOWN;
    double amount = 0.0;
    std::string currency;
    std::string raw_status;
    std::string raw_response_json;
    std::string failure_reason;
};

struct PaymentCancelRequestRecord {
    std::string provider_payment_id;
    std::string reason;
    PaymentProviderConfigRecord config;
};

struct PaymentCancelResponseRecord {
    int status = PAYMENT_STATUS_CANCELLED;
    std::string raw_response_json;
    std::string failure_reason;
};

struct PaymentRefundRequestRecord {
    std::string provider_payment_id;
    double amount = 0.0;
    std::string currency;
    std::string reason;
    PaymentProviderConfigRecord config;
};

struct PaymentRefundResponseRecord {
    int status = PAYMENT_STATUS_REFUNDED;
    std::string provider_refund_id;
    std::string raw_response_json;
    std::string failure_reason;
};
