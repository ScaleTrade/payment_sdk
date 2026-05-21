#pragma once

#include <rapidjson/document.h>
#include <string>

#include "Structures.h"
#include "model/PaymentStructures.hpp"

inline int PAYMENT_VERSION_API = 100;

class PaymentInterface {
public:
    virtual ~PaymentInterface() = default;

    virtual int getApiVersion() const { return PAYMENT_VERSION_API; }
    virtual std::string provider_code() const { return "demo"; }
    virtual std::string provider_name() const { return "Demo payment provider"; }
    virtual std::string provider_description() const { return "Demo payment provider"; }
    virtual std::string provider_payment_mode() const { return "redirect"; }

    virtual int InitPayment(const PaymentProviderConfigRecord& config) { (void)config; return RET_OK; }
    virtual int ShutdownPayment() { return RET_OK; }

    virtual int CreateDeposit(const PaymentCreateRequestRecord& req, PaymentCreateResponseRecord& res) { return RET_OK_NONE; }
    virtual int VerifyWebhook(const PaymentWebhookRequestRecord& req, PaymentWebhookResultRecord& res) { return RET_OK_NONE; }
    virtual int GetPaymentStatus(const PaymentStatusRequestRecord& req, PaymentStatusResponseRecord& res) { return RET_OK_NONE; }
    virtual int CancelPayment(const PaymentCancelRequestRecord& req, PaymentCancelResponseRecord& res) { return RET_OK_NONE; }
    virtual int RefundPayment(const PaymentRefundRequestRecord& req, PaymentRefundResponseRecord& res) { return RET_OK_NONE; }

    virtual int CustomAction(rapidjson::Value& req, rapidjson::Value& res, rapidjson::Document::AllocatorType& allocator) { return RET_OK_NONE; }
};
