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

At initialization the server calls `CreatePaymentProvider`, reads
`provider_code`, `provider_name`, `provider_description`, and
`provider_payment_mode`, then keeps the initialized instance in the
Cashier provider registry.

## Core Files

- `sdk/payments/include/PaymentInterface.h`
- `sdk/payments/include/PaymentServerInterface.h`
- `sdk/payments/include/model/PaymentStructures.hpp`

## Provider Responsibilities

- create provider-side deposit payment;
- verify webhook signatures;
- normalize provider statuses;
- return redirect URLs or payment instructions;
- optionally cancel/refund provider payments.

Provider modules must not call trading balance operations directly.
