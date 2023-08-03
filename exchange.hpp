#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
using std::set;

#include "useraccount.hpp"
#include "utility.hpp"

class Exchange {
  private:
    std::vector<Useraccount> users;
    //set<Useraccount> users;
    
    std::vector<Order> buy_orders;
    std::vector<Order> sell_orders;
    std::vector<Trade> all_trades;
  public:
    
    void update_users_buy(const Order &order, Useraccount &i, int index, int amount);
    void update_users_sell(const Order &order, Useraccount &i, int index, int amount);
    void process_buy(const Order &order, Useraccount &i, int index);
    void process_sell(const Order &order, Useraccount &i, int index);
    int get_buy_index(const Order &order);
    int get_sell_index(const Order &order);
    void check_sell_order(const Order &order);

    void MakeDeposit(const std::string &username, const std::string &asset,int amount);
    void PrintUserPortfolios(std::ostream &os) const;
    bool MakeWithdrawal(const std::string &username, const std::string &asset,int amount);
    bool AddOrder(const Order &order);
    void PrintUsersOrders(std::ostream &os) const;
    void PrintTradeHistory(std::ostream &os) const;
    void PrintBidAskSpread(std::ostream &os) const;
    std::ostream& operator<<(std::ostream &oss);
};
