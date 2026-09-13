#include <gtest/gtest.h>
#include "OrderBook.h"

TEST(OrderBookTest, SanityCheck) {
    EXPECT_EQ(1 + 1, 2);
}
TEST(OrderBookTest,FullMatchLeavesBookEmpty){
    OrderBook book;
    Order sellOrder{.timestamp=1,.orderId=1,.price=100,.quantity=5,.side=Side::SELL};
    Order buyOrder{.timestamp=1,.orderId=2,.price=100,.quantity=5,.side=Side::BUY};
    book.addOrder(sellOrder);
    book.addOrder(buyOrder);
/*timestamp,orderid,price,quantity,side*/
    EXPECT_TRUE(book.isEmpty());
}
TEST(OrderBookTest, PartialFillLeavesRemainderResting) {
    OrderBook book;
    Order sellOrder{.timestamp=1,.orderId=3,.price=100,.quantity=3,.side=Side::SELL};  // fill in: unique id, price 100, quantity 3, Side::SELL
    Order buyOrder{.timestamp=1,.orderId=4,.price=100,.quantity=5,.side=Side::BUY};   // fill in: unique id, price 100, quantity 5, Side::BUY

    book.addOrder(sellOrder);
    book.addOrder(buyOrder);

    EXPECT_TRUE(book.isEmpty() == false);       // something should still be resting
    EXPECT_EQ(book.getBuyQuantityAtPrice(100), 2.0);  // 5 - 3 = 2 left over
}