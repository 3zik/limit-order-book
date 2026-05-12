#include <iostream>
#include <memory>

#include "Order.h"
#include "Orderbook.h"

int main()
{
    Orderbook orderbook;

    orderbook.AddOrder(std::make_shared<Order>(
        OrderType::GoodTillCancel, 1, Side::Buy, 100, 10));
    std::cout << orderbook.Size() << '\n';

    orderbook.CancelOrder(1);
    std::cout << orderbook.Size() << '\n';

    return 0;
}
