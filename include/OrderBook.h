#pragma once //since header file it also needed to be added once
#include "Order.h"//this includes order.h file in this file
#include <map> //for buy and selling list as we needed them sorted , high to low in buy list and low to high in sell list 
#include <list> //for multiple orders in same list if added with another hashmap it can cancel the order in o(1) time , basically it finds the position for O(1) deletion
#include <functional>//it gives std::greater<double> it basically reverses the hashmaps key :- required for buy list as high on top
#include <unordered_map>
class OrderBook{
    private:
        std::map<double,std::list<Order>,std::greater<double>>buyLevels;//key=double,value=std::list<Order>,comparator=std::greater<double>
        std::map<double,std::list<Order>>sellLevels;//std::list<Order> means a list of Order 
        std::unordered_map<uint64_t,std::list<Order>::iterator> orderIndex;
    public:
        void addOrder(const Order& order){
            if(order.side==Side::BUY){
                Order incoming=order;
                while(incoming.quantity>0 && !sellLevels.empty() && sellLevels.begin()->first <= incoming.price){
                    auto bestSellIt = sellLevels.begin();
                    std::list<Order>& ordersAtBestPrice = bestSellIt->second;
                    Order& restingOrder = ordersAtBestPrice.front(); 

                    double matchQty=std::min(incoming.quantity,restingOrder.quantity);

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

                if(incoming.quantity>0){
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

                    double matchQty=std::min(restingOrder.quantity,incoming.quantity);
                    
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
                if(incoming.quantity>0){
                    sellLevels[incoming.price].push_back(incoming);
                    orderIndex[incoming.orderId]=std::prev(sellLevels[incoming.price].end());
                }
            }
        }
        void cancelOrder(uint64_t orderId){
            auto found=orderIndex.find(orderId);
            if(found==orderIndex.end()){
                return;
            }
            std::list<Order>::iterator orderIt=found->second;
            Side side=orderIt->side;
            double price =orderIt->price;

            if(side==Side::BUY){
                buyLevels[price].erase(orderIt);
                if(buyLevels[price].empty()){
                    buyLevels.erase(price);
                }
            }
            else{
                sellLevels[price].erase(orderIt);
                if(sellLevels[price].empty()){
                    sellLevels.erase(price);
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
        double getBuyQuantityAtPrice(double price) const {
            auto it = buyLevels.find(price);
            if (it == buyLevels.end()) {
                return 0.0;
            }

            double total = 0.0;
            for (const Order& o : it->second) {
                total += o.quantity;
            }
            return total;
        }
        double getSellQuantityAtPrice(double price) const {
            auto it = sellLevels.find(price);
            if (it == sellLevels.end()) {
                return 0.0;
            }

            double total = 0.0;
            for (const Order& o : it->second) {
                total += o.quantity;
            }
            return total;
        }
};
/*
Order incoming=order;
while(incoming.price<=buyLevels.begin()->first && !buyLevels.empty() && incoming.quantity>0){
auto bestbuyit = buyLevels.begin();
std::list<Order> buyatbestprice=bestbuyit->second();
Order& restingorder=buyatbestprice.front();
}
*/