#pragma once //since header file it also needed to be added once
#include "Order.h"//this includes order.h file in this file
#include <map> //for buy and selling list as we needed them sorted , high to low in buy list and low to high in sell list 
#include <list> //for multiple orders in same list if added with another hashmap it can cancel the order in o(1) time , basically it finds the position for O(1) deletion
#include <functional>//it gives std::greater<double> it basically reverses the hashmaps key :- required for buy list as high on top
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <cassert>
#include "Trade.h"//finally added this to record the trades
#include <vector>
#include <optional>//so to hold nothing 

class OrderBook{
    private:
        std::map<uint64_t,std::list<Order>,std::greater<uint64_t>>buyLevels;//key=double(price),value=std::list<Order>(because multiple order can exist at same time),comparator=std::greater<double>
        std::map<uint64_t,std::list<Order>>sellLevels;//std::list<Order> means a list of Order 
        std::unordered_map<uint64_t,std::list<Order>::iterator> orderIndex;//hashmap here to find order in o(1) time
        uint64_t nextOrderId=1;//this is for unique id
        std::vector<Trade> trades;
        uint64_t nextTradeId=1;
    public:
        uint64_t addOrder(Order order){
            if(order.quantity==0){
                throw std::invalid_argument("Order quantity cannot be zero");
            }
            if(order.side != Side::BUY && order.side != Side::SELL){
                throw std::invalid_argument("Invalid order side");
            }
            if(nextOrderId==0){
                throw std::overflow_error("Order ID space exhausted");
            }
            if(order.type==OrderType::LIMIT && order.price==0){
                throw std::invalid_argument("Limit order price cannot be zero");
            }
            if(order.type==OrderType::MARKET){
                order.price=(order.side==Side::BUY)?UINT64_MAX:0;
                if(order.timeInForce==TimeInForce::GTC){
                    order.timeInForce=TimeInForce::IOC;
                }
            }
            order.orderId=nextOrderId++;
            const uint64_t assignedOrderId=order.orderId;
            if(order.side==Side::BUY){//in buy incoming price is the max price a buyer is willing to pay
                Order incoming=order;//creating a copy here
                while(incoming.quantity>0 && !sellLevels.empty() && sellLevels.begin()->first <= incoming.price){
                    auto bestSellIt = sellLevels.begin();
                    std::list<Order>& ordersAtBestPrice = bestSellIt->second;
                    Order& restingOrder = ordersAtBestPrice.front(); 

                    uint64_t matchQty=std::min(incoming.quantity,restingOrder.quantity);

                    Trade trade{};
                    trade.tradeId = nextTradeId++;
                    trade.timestamp = incoming.timestamp;
                    trade.price = restingOrder.price;
                    trade.quantity = matchQty;
                    trade.buyOrderId = incoming.orderId;
                    trade.sellOrderId = restingOrder.orderId;
                    trade.aggressorSide = Side::BUY;
                    trades.push_back(trade);

                    incoming.quantity-=matchQty;
                    restingOrder.quantity-=matchQty;

                    if(restingOrder.quantity==0){
                        uint64_t filledOrderId=restingOrder.orderId;
                        ordersAtBestPrice.pop_front();
                        orderIndex.erase(filledOrderId);
                        if(ordersAtBestPrice.empty()){
                            sellLevels.erase(bestSellIt);
                        }
                    }
                }

                if(incoming.quantity>0 && incoming.timeInForce==TimeInForce::GTC){
                    buyLevels[incoming.price].push_back(incoming);
                    orderIndex[incoming.orderId]=std::prev(buyLevels[incoming.price].end());
                }
            }
            else{
                Order incoming=order;/*creating a copy here*/
                while(incoming.quantity>0 && !buyLevels.empty() && buyLevels.begin()->first>=incoming.price){
                    auto bestBuyIt=buyLevels.begin();
                    std::list<Order>& ordersAtBestPrice =bestBuyIt->second;
                    Order& restingOrder=ordersAtBestPrice.front();

                    uint64_t matchQty=std::min(restingOrder.quantity,incoming.quantity);

                    Trade trade{};
                    trade.tradeId = nextTradeId++;
                    trade.timestamp = incoming.timestamp;
                    trade.price = restingOrder.price;
                    trade.quantity = matchQty;
                    trade.buyOrderId = restingOrder.orderId;
                    trade.sellOrderId = incoming.orderId;
                    trade.aggressorSide = Side::SELL;
                    trades.push_back(trade);
                    
                    incoming.quantity-=matchQty;
                    restingOrder.quantity-=matchQty;

                    if(restingOrder.quantity==0){
                        uint64_t filledOrderId=restingOrder.orderId;
                        ordersAtBestPrice.pop_front();
                        orderIndex.erase(filledOrderId);
                        if(ordersAtBestPrice.empty()){
                            buyLevels.erase(bestBuyIt);
                        }
                    }
                }
                if(incoming.quantity>0 && incoming.timeInForce==TimeInForce::GTC){
                    sellLevels[incoming.price].push_back(incoming);
                    orderIndex[incoming.orderId]=std::prev(sellLevels[incoming.price].end());
                }
            }
            return assignedOrderId;
        }
        void cancelOrder(uint64_t orderId){
            auto found=orderIndex.find(orderId);
            if(found==orderIndex.end()){
                return;
            }
            std::list<Order>::iterator orderIt=found->second;
            Side side=orderIt->side;
            uint64_t price =orderIt->price;

            if(side==Side::BUY){
                auto levelIt=buyLevels.find(price);
                assert(levelIt!=buyLevels.end());//loud failure in debug if index and book disagree
                if(levelIt!=buyLevels.end()){
                    levelIt->second.erase(orderIt);
                    if(levelIt->second.empty()){
                        buyLevels.erase(levelIt);
                    }
                }
            }
            else{
                auto levelIt = sellLevels.find(price);
                assert(levelIt != sellLevels.end());//loud failure in debug if index and book disagree
                if(levelIt != sellLevels.end()){
                    levelIt->second.erase(orderIt);
                    if(levelIt->second.empty()){
                        sellLevels.erase(levelIt);
                    }
                }
            }
            orderIndex.erase(found);
        }
        bool isEmpty() const{
            return buyLevels.empty() && sellLevels.empty();
        }
        size_t buyLevelCount() const{
            return buyLevels.size();
        }
        size_t sellLevelCount() const{
            return sellLevels.size();
        }
        uint64_t getBuyQuantityAtPrice(uint64_t price) const {
            auto it = buyLevels.find(price);
            if (it == buyLevels.end()) {
                return 0;
            }

            uint64_t total = 0;
            for (const Order& o : it->second)/*iterates through all orders at that price and adds their quantites*/ {
                total += o.quantity;
            }
            return total;
        }
        uint64_t getSellQuantityAtPrice(uint64_t price) const {
            auto it = sellLevels.find(price);
            if (it == sellLevels.end()) {
                return 0;
            }

            uint64_t total = 0;
            for (const Order& o : it->second) {
                total += o.quantity;
            }
            return total;
        }
        //read-only getters
        const std::vector<Trade>& getTrades() const{//returing a reference here so no copy is made ,const make sure its read only and not modifiable
            return trades;
        }
        size_t getTradeCount() const{
            return trades.size();
        }
        uint64_t getTotalTradedVolume() const{
            uint64_t total=0;
            for(const Trade& t: trades){
                total+=t.quantity;
            }
            return total;
        }
        std::optional<uint64_t> getLastTradePrice() const{//std optional can either hold a value or nothing ,if i did return 0, it would mean that a trade happened at price 0
            if(trades.empty()){
                return std::nullopt;
            }
            return trades.back().price;
        }
};
