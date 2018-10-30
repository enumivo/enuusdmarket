#include "ex.hpp"

#include <cmath>
#include <enulib/action.hpp>
#include <enulib/asset.hpp>
#include "enu.token.hpp"

using namespace enumivo;
using namespace std;

void ex::buy(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.usd.mm)).
	   get_balance(_self, enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  
  enu_balance = enu_balance/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  // get USD balance
  double usd_balance = enumivo::token(N(enu.usd.mm)).
	   get_balance(_self, enumivo::symbol_type(USD_SYMBOL).name()).amount;

  usd_balance = usd_balance/10000;

  //deduct fee
  received = received * 0.999;
  
  double product = usd_balance * enu_balance;

  double buy = usd_balance - (product / (received + enu_balance));

  auto to = transfer.from;

  auto quantity = asset(10000*buy, USD_SYMBOL);

  action(permission_level{_self, N(active)}, N(stable.coin), N(transfer),
         std::make_tuple(N(enu.usd.mm), to, quantity,
                         std::string("Buy USD with ENU")))
      .send();

  action(permission_level{_self, N(active)}, N(enu.token), N(transfer),
         std::make_tuple(_self, N(enu.usd.mm), transfer.quantity,
                         std::string("Buy USD with ENU")))
      .send();
}

void ex::sell(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get USD balance
  double usd_balance = enumivo::token(N(enu.usd.mm)).
	   get_balance(_self, enumivo::symbol_type(USD_SYMBOL).name()).amount;
  
  usd_balance = usd_balance/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.usd.mm)).
	   get_balance(_self, enumivo::symbol_type(ENU_SYMBOL).name()).amount;

  enu_balance = enu_balance/10000;

  //deduct fee
  received = received * 0.999;

  double product = enu_balance * usd_balance;

  double sell = enu_balance - (product / (received + usd_balance));

  auto to = transfer.from;

  auto quantity = asset(10000*sell, ENU_SYMBOL);

  /*
  action(permission_level{_self, N(active)}, N(enu.token), N(transfer),
         std::make_tuple(N(enu.usd.mm), to, quantity,
                         std::string("Sell USD for ENU")))
      .send();
  */    

  action(permission_level{_self, N(active)}, N(enu.usd.mm), N(transfer),
         std::make_tuple(_self, N(enu.usd.mm), transfer.quantity,
                         std::string("Sell USD for ENU")))
      .send();
}

void ex::apply(account_name contract, action_name act) {

  if (contract == N(enu.token) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ENU_SYMBOL,
                 "Must send ENU");
    buy(transfer);
    return;
  }

  if (contract == N(stable.coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == USD_SYMBOL,
                 "Must send USD");
    sell(transfer);
    return;
  }

  if (act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(false, "Must send USD or ENU");
    sell(transfer);
    return;
  }

  if (contract != _self) return;

}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  ex enuusd(receiver);
  enuusd.apply(code, action);
  enumivo_exit(0);
}
}
