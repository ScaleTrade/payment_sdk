#include <PaymentInterface.h>
#include <PaymentServerInterface.h>

class MinimalPaymentProvider final : public PaymentInterface {
public:
    explicit MinimalPaymentProvider(PaymentServerInterface* server) : server_(server) {}

    std::string provider_code() const override { return "minimal"; }
    std::string provider_name() const override { return "Minimal payment provider"; }
    std::string provider_description() const override { return "Example payment provider"; }
    std::string provider_payment_mode() const override { return "redirect"; }

    int CreateDeposit(const PaymentCreateRequestRecord& req, PaymentCreateResponseRecord& res) override {
        if (server_) server_->LogsOut("PAYMENT", "CreateDeposit called");
        res.provider_payment_id = "demo-" + std::to_string(req.transaction_id);
        res.redirect_url = "https://example.com/pay/" + res.provider_payment_id;
        res.status = PAYMENT_STATUS_PENDING;
        return RET_OK;
    }

private:
    PaymentServerInterface* server_;
};

extern "C" int GetPaymentApiVersion() {
    return PaymentServerInterface::GetApiVersion();
}

extern "C" PaymentInterface* CreatePaymentProvider(PaymentServerInterface* server) {
    return new MinimalPaymentProvider(server);
}

extern "C" void DestroyPaymentProvider(PaymentInterface* provider) {
    delete provider;
}
