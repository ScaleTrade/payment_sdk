# Payments SDK

Payments SDK is used to build payment provider modules for the Cashier
domain.

Provider modules normalize external payment systems into the server
contract. They must not credit trading balances directly. Balance changes
are owned by the server-side Cashier flow.

## Required Exports

Payment modules should export:

```cpp
extern "C" int GetPaymentApiVersion();
extern "C" PaymentInterface* CreatePaymentProvider(PaymentServerInterface* server);
extern "C" void DestroyPaymentProvider(PaymentInterface* provider);
```

`GetPaymentApiVersion()` must return `PaymentServerInterface::GetApiVersion()`.

For each enabled Cashier provider config, the server copies the original
provider `.so` to `var/sttrader/payments` under a unique runtime filename,
calls `CreatePaymentProvider`, verifies that `provider_code()` matches the
config `provider`, and then calls `InitPayment(config)`.

One provider module may therefore be initialized more than once with different
credentials or routing config. Each active provider config has its own module
copy, handle, and provider instance. Before destroying an instance, the server
calls `ShutdownPayment()`.

## Core Files

- `sdk/payments/include/PaymentInterface.h`
- `sdk/payments/include/PaymentServerInterface.h`
- `sdk/payments/include/model/PaymentStructures.hpp`
- `sdk/payments/examples/praxis_cashier_payment.cpp`

## Provider Responsibilities

- create provider-side deposit payment;
- validate and store config in `InitPayment`;
- release provider resources in `ShutdownPayment`;
- verify webhook signatures;
- normalize provider statuses;
- return redirect URLs or payment instructions;
- optionally cancel/refund provider payments.

Provider modules must not call trading balance operations directly.

## Praxis-Style Example

`examples/praxis_cashier_payment.cpp` is a realistic hosted cashier provider
sample. It uses a Praxis-like redirect flow:

1. Cashier resolves the active provider config for `provider`, `brand`,
   `method`, `currency`, and `country`.
2. Cashier passes routing fields and provider-specific `config_json` to the
   payment module through `PaymentCreateRequestRecord::config`.
3. The provider builds a hosted cashier payload and returns:
   - `provider_payment_id`;
   - `redirect_url`;
   - `instructions_json`;
   - `raw_response_json`;
   - normalized `PAYMENT_STATUS_PENDING`.

Example provider config:

```json
{
  "provider": "praxis",
  "brand": "ion4",
  "method": "card",
  "country": "",
  "currency": "USD",
  "config_json": "{\"merchant_id\":\"merchant-id\",\"application_key\":\"application-key\",\"secret_key\":\"merchant-secret\",\"webhook_secret\":\"webhook-secret\",\"api_base_url\":\"https://api.praxis.example\",\"cashier_base_url\":\"https://cashier.praxis.example\",\"api_version\":\"1.3\",\"locale\":\"en\"}",
  "sandbox": 0,
  "enabled": 1
}
```

The sample intentionally does not perform an HTTP request. A production
provider should POST the prepared payload to the provider API, validate the
provider response, and return the real hosted cashier URL.

Webhook normalization is also shown in the example. A production Praxis module
must validate the webhook signature with `webhook_secret` from `config_json`
before returning `PAYMENT_SIGNATURE_VALID`.
