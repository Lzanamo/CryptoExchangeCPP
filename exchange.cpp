#include "exchange.hpp"
#include "useraccount.hpp"
#include "utility.hpp"
#include <iostream>
using std::cout; using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <map>
using std::set;
#include <algorithm>
using std::sort;
using std::remove;
#include <sstream>

void Exchange::MakeDeposit(const std::string &username, const std::string &asset, int amount){
    int count = 0;
    for (auto &i : users){
        if (i.name_ == username){
            count += 1;
            if (i.assets_.count(asset) == 1){
                i.assets_[asset] += amount;
            } else{
                i.assets_[asset] = amount;
            }
        }
    }
    if (count == 0){
        Useraccount person{username, asset, amount};
        users.push_back(person);
    }
}

void Exchange::PrintUserPortfolios(std::ostream &os) const{
    auto temp = users;
    sort(temp.begin(), temp.end(), [](Useraccount a, Useraccount b){ return a.name_ < b.name_;});
    os << "User Portfolios (in alphabetical order):" << endl;
    for (auto &i : temp){
        os << i.name_ << "\'s Portfolio: ";
        for (auto &x : i.assets_){
            if (x.second != 0){
                os << x.second << " " << x.first << ", ";
            }
        }
        os << endl;
    }
}

bool Exchange::MakeWithdrawal(const std::string &username, const std::string &asset, int amount){
    for (auto &i : users){
        if (i.name_ == username){
            if (i.assets_.count(asset) == 1 && i.assets_[asset] >= amount){
                i.assets_[asset] -= amount;
                return true;
            }
        }
    }
    return false;  
}

bool Exchange::AddOrder(const Order &order){
    for (auto &i : users){
        if (i.name_ == order.username){
            if (order.if_possible(i) == false){return false;}
            if (order.side == "Buy"){
                int ind = get_buy_index(order);
                if (ind == -1){ 
                    buy_orders.push_back(order);
                    i.open_orders.push_back(order);
                    i.assets_["USD"] -= (order.amount * order.price);
                    return true;
                }
                process_buy(order, i, ind);
                return true;
            } else if (order.side == "Sell"){
                int ind = get_sell_index(order);
                if (ind == -1){ 
                    sell_orders.push_back(order);
                    i.open_orders.push_back(order);
                    i.assets_[order.asset] -= order.amount;
                    return true;
                }
                process_sell(order, i, ind);
                return true;
            }
        }
    }
    return false;
}

int Exchange::get_buy_index(const Order &order){
    int selling = -1;
    for (int i = 0; i < static_cast<int>(sell_orders.size()); ++i){
        if (order.asset == (sell_orders.at(i)).asset && order.price >= (sell_orders.at(i)).price){
            if (selling == -1 || (sell_orders.at(selling)).price > (sell_orders.at(i)).price){
                selling = i;
           }
        }
    }
    return selling;
}

int Exchange::get_sell_index(const Order &order){
    int buying = -1;
    for (int i = 0; i < static_cast<int>(buy_orders.size()); ++i){
        if (order.asset == (buy_orders.at(i)).asset && order.price <= (buy_orders.at(i)).price){
            if (buying == -1 || (buy_orders.at(buying)).price < (buy_orders.at(i)).price){
                buying = i;
           }
        }
    }
    return buying;
}

void Exchange::update_users_buy(const Order &order, Useraccount &i, int index, int amount){
    if (i.assets_.count(order.asset) == 1){
        i.assets_[order.asset] += amount;
    } else{
        i.assets_[order.asset] = amount;
    }
    i.assets_["USD"] -= (amount * order.price);
    for (auto &x : users){
        if (x.name_ == (sell_orders.at(index)).username){
            x.assets_["USD"] += (amount * order.price);
            all_trades.push_back(Trade{i.name_, x.name_, order.asset, amount, order.price});
        }
    }
}

void Exchange::update_users_sell(const Order &order, Useraccount &i, int index, int amount){
    //(buy_orders.at(index)).price
    i.assets_["USD"] += (amount * order.price);
    i.assets_[order.asset] -= amount;
    for (auto &x : users){
        if (x.name_ == (buy_orders.at(index)).username){
            if (x.assets_.count(order.asset) == 1){
                x.assets_[order.asset] += amount;
            } else{
                x.assets_[order.asset] = amount;
            }
            all_trades.push_back(Trade{x.name_, i.name_, order.asset, amount, order.price});
        }
    }
    
}

void Exchange::process_buy(const Order &order, Useraccount &i, int index){
    int left = (sell_orders.at(index)).amount - order.amount;
    if ( left == 0){
        update_users_buy(order, i, index, order.amount);
        for (auto &x : users){
            if (x.name_ == (sell_orders.at(index)).username){
                x.filled_orders.push_back(Order {(sell_orders.at(index)).username, (sell_orders.at(index)).side, order.asset, (sell_orders.at(index)).amount, order.price});
                x.open_orders.erase(remove(x.open_orders.begin(), x.open_orders.end(), sell_orders.at(index)));
            }
        }
        i.filled_orders.push_back(order);
        sell_orders.erase(sell_orders.begin()+index);
    } else if (left < 0){
        update_users_buy(order, i, index, (sell_orders.at(index)).amount);
        for (auto x = users.begin(); x != users.end(); ++x){
            if (x->name_ == (sell_orders.at(index)).username){
                x->filled_orders.push_back(Order {(sell_orders.at(index)).username, (sell_orders.at(index)).side, order.asset, (sell_orders.at(index)).amount, order.price});
                x->open_orders.erase(remove(x->open_orders.begin(), x->open_orders.end(), sell_orders.at(index)));
            }
        }
        i.filled_orders.push_back(Order {order.username, order.side, order.asset, (sell_orders.at(index)).amount, order.price});
        sell_orders.erase(sell_orders.begin()+index);
        AddOrder(Order {order.username, order.side, order.asset, left * -1, order.price});
    }else if (left > 0){
        update_users_buy(order, i, index, order.amount);
        for (auto x = users.begin(); x != users.end(); ++x){
            if (x->name_ == (sell_orders.at(index)).username){
                x->filled_orders.push_back(Order {(sell_orders.at(index)).username, (sell_orders.at(index)).side, order.asset, order.amount, order.price});
                for (auto &v : x->open_orders){
                    if (v == sell_orders.at(index)){v.amount = left;}
                }
            }
        }
        i.filled_orders.push_back(order);
        sell_orders.at(index).amount = left;
    }  
}

void Exchange::process_sell(const Order &order, Useraccount &i, int index){
    int left = order.amount - (buy_orders.at(index)).amount;
    if ( left == 0){
        update_users_sell(order, i, index, order.amount);
        for (auto &x : users){
            if (x.name_ == (buy_orders.at(index)).username){
                x.filled_orders.push_back(buy_orders.at(index));
                x.open_orders.erase(remove(x.open_orders.begin(), x.open_orders.end(), buy_orders.at(index)));
            }
        }
        i.filled_orders.push_back(order);
        buy_orders.erase(buy_orders.begin()+index);
    } else if (left < 0){
        update_users_sell(order, i, index, order.amount);
        i.filled_orders.push_back(order);
        for (auto &x : users){
            if (x.name_ == (buy_orders.at(index)).username){
                x.filled_orders.push_back(Order {(buy_orders.at(index)).username, (buy_orders.at(index)).side, order.asset, order.amount, order.price});
                x.open_orders.erase(remove(x.open_orders.begin(), x.open_orders.end(), buy_orders.at(index)));
                x.open_orders.push_back(Order {(buy_orders.at(index)).username, (buy_orders.at(index)).side, order.asset, left *-1, (buy_orders.at(index)).price});
            }
        }
        buy_orders.at(index).amount = left * -1;
    }else if (left > 0){
        update_users_sell(order, i, index, (buy_orders.at(index)).amount);
        for (auto &x : users){
            if (x.name_ == (buy_orders.at(index)).username){
                x.filled_orders.push_back(Order {(buy_orders.at(index)).username, (buy_orders.at(index)).side, order.asset, (buy_orders.at(index)).amount, order.price});
                x.open_orders.erase(remove(x.open_orders.begin(), x.open_orders.end(), buy_orders.at(index)));
            }
        }
        i.filled_orders.push_back(Order {order.username, order.side, order.asset, (buy_orders.at(index)).amount, order.price});
        buy_orders.erase(buy_orders.begin()+index);
        AddOrder(Order {order.username, order.side, order.asset, left, order.price});
    }  
}

void Exchange::PrintUsersOrders(std::ostream &os) const{
    auto temp = users;
    sort(temp.begin(), temp.end(), [](Useraccount a, Useraccount b){ return a.name_ < b.name_;});
    os << "Users Orders (in alphabetical order):" << endl;
    for (auto &i : temp){
        os << i.name_ << "\'s Open Orders (in chronological order):" << endl;
        for (auto &x : i.open_orders){
            os << x << endl;
        }
        os << i.name_ << "\'s Filled Orders (in chronological order):" << endl;
        for (auto &x : i.filled_orders){
            os << x << endl;
        }
    }
}

void Exchange::PrintTradeHistory(std::ostream &os) const{
    os << "Trade History (in chronological order):" << endl;
    for(auto &i : all_trades){
        os << i ;
    }
}

void Exchange::PrintBidAskSpread(std::ostream &os) const{
    os << "Asset Bid Ask Spread (in alphabetical order):" << endl;
    auto temp = buy_orders;
    temp.insert(temp.end(), sell_orders.begin(), sell_orders.end());
    set<string> all_assets;
    for (auto &i : temp){all_assets.insert(i.asset);}
    for (auto &i : all_assets){
        os << i << ": ";
        int buying = -1;
        for (int x = 0; x < static_cast<int>(buy_orders.size()); ++x){
            if (i == (buy_orders.at(x)).asset){
                if (buying == -1 || (buy_orders.at(buying)).price <= (buy_orders.at(x)).price){
                    buying = x;
                }
            }
        }
        if (buying == -1){
            os << "Highest Open Buy = NA USD and ";
        }else{
            os <<"Highest Open Buy = " << (buy_orders.at(buying)).price <<  " USD and ";
        }
        int selling = -1;
        for (int x = 0; x < static_cast<int>(sell_orders.size()); ++x){
            if (i == (sell_orders.at(x)).asset){
                if (selling == -1 || (sell_orders.at(selling)).price > (sell_orders.at(x)).price){
                    selling = x;
                }
            }
        }
        if (selling == -1){
            os << "Lowest Open Sell = NA USD" << endl;
        }else{
            os <<"Lowest Open Sell = " << (sell_orders.at(selling)).price <<  " USD" << endl;
        }
    }
}

// int main(){
//     Exchange e;
//     std::ostringstream oss;
//     e.MakeDeposit("Nahum", "BTC", 1000);
//     e.MakeDeposit("Nahum", "USD", 100000);
//     e.MakeDeposit("Dolson", "USD", 555555);
//     e.MakeDeposit("Ofria", "ETH", 678);
//     e.MakeDeposit("Zaabar", "USD", 12121212);
//     e.MakeDeposit("Zaabar", "LTC", 4563);
//     // BTC rough price 1000
//     // ETH rough price 100
//     // LTC rough price 500
//     e.AddOrder({"Nahum", "Sell", "BTC", 5, 1100});
//     e.AddOrder({"Nahum", "Sell", "BTC", 100, 1200});
//     e.AddOrder({"Nahum", "Buy", "BTC", 7, 800});
//     e.AddOrder({"Dolson", "Buy", "BTC", 1, 950});
//     e.AddOrder({"Ofria", "Sell", "ETH", 12, 156});
//     e.AddOrder({"Ofria", "Sell", "ETH", 10, 160});
//     e.AddOrder({"Zaabar", "Sell", "LTC", 10, 550});
//     e.AddOrder({"Zaabar", "Buy", "LTC", 10, 450});
//     e.PrintUserPortfolios(std::cout);
//     // oss.str("");
//     // e.PrintUserPortfolios(oss);
//     // CHECK(oss.str() == "User Portfolios (in alphabetical order):\nDolson's Portfolio: 554605 USD, \nNahum's Portfolio: 895 BTC, 94400 USD, \nOfria's Portfolio: 656 ETH, \nZaabar's Portfolio: 4553 LTC, 12116712 USD, \n");
//     e.PrintUsersOrders(std::cout);
//     // oss.str("");
//     // e.PrintUsersOrders(oss);
//     // CHECK(oss.str() == "Users Orders (in alphabetical order):\nDolson's Open Orders (in chronological order):\nBuy 1 BTC at 950 USD by Dolson\nDolson's Filled Orders (in chronological order):\nNahum's Open Orders (in chronological order):\nSell 5 BTC at 1100 USD by Nahum\nSell 100 BTC at 1200 USD by Nahum\nBuy 7 BTC at 800 USD by Nahum\nNahum's Filled Orders (in chronological order):\nOfria's Open Orders (in chronological order):\nSell 12 ETH at 156 USD by Ofria\nSell 10 ETH at 160 USD by Ofria\nOfria's Filled Orders (in chronological order):\nZaabar's Open Orders (in chronological order):\nSell 10 LTC at 550 USD by Zaabar\nBuy 10 LTC at 450 USD by Zaabar\nZaabar's Filled Orders (in chronological order):\n");
//     std::cout << std::endl << std::endl;
//     e.AddOrder({"Nahum", "Buy", "LTC", 55, 600});
//     e.AddOrder({"Nahum", "Buy", "ETH", 30, 158});
//     e.AddOrder({"Ofria", "Sell", "ETH", 10, 140});
//     e.AddOrder({"Zaabar", "Buy", "BTC", 10, 1500});
//     e.AddOrder({"Zaabar", "Buy", "ETH", 200, 1200});
//     e.AddOrder({"Nahum", "Sell", "BTC", 30, 158});
//     e.AddOrder({"Dolson", "Buy", "BTC", 20, 2000});
//     e.PrintTradeHistory(std::cout);
//     // oss.str("");
//     // e.PrintTradeHistory(oss);
//     // CHECK(oss.str() == "Trade History (in chronological order):\nNahum Bought 10 of LTC From Zaabar for 600 USD\nNahum Bought 12 of ETH From Ofria for 158 USD\nNahum Bought 10 of ETH From Ofria for 140 USD\nZaabar Bought 5 of BTC From Nahum for 1500 USD\nZaabar Bought 5 of BTC From Nahum for 1500 USD\nZaabar Bought 10 of ETH From Ofria for 1200 USD\nDolson Bought 1 of BTC From Nahum for 158 USD\nNahum Bought 7 of BTC From Nahum for 158 USD\nDolson Bought 20 of BTC From Nahum for 2000 USD\n");
//     std::cout << std::endl;
//     e.PrintUserPortfolios(std::cout);
//     // oss.str("");
//     // e.PrintUserPortfolios(oss);
//     // CHECK(oss.str() == "User Portfolios (in alphabetical order):\nDolson's Portfolio: 21 BTC, 514605 USD, \nNahum's Portfolio: 872 BTC, 22 ETH, 10 LTC, 112924 USD, \nOfria's Portfolio: 646 ETH, 15296 USD, \nZaabar's Portfolio: 10 BTC, 10 ETH, 4553 LTC, 11867712 USD, \n");
//     std::cout << std::endl;
//     e.PrintUsersOrders(std::cout);
//     // oss.str("");
//     // e.PrintUsersOrders(oss);
//     // CHECK(oss.str() == "Users Orders (in alphabetical order):\nDolson's Open Orders (in chronological order):\nDolson's Filled Orders (in chronological order):\nBuy 1 BTC at 158 USD by Dolson\nBuy 20 BTC at 2000 USD by Dolson\nNahum's Open Orders (in chronological order):\nSell 95 BTC at 1200 USD by Nahum\nBuy 45 LTC at 600 USD by Nahum\nBuy 8 ETH at 158 USD by Nahum\nSell 2 BTC at 158 USD by Nahum\nNahum's Filled Orders (in chronological order):\nBuy 10 LTC at 600 USD by Nahum\nBuy 12 ETH at 158 USD by Nahum\nBuy 10 ETH at 140 USD by Nahum\nSell 5 BTC at 1500 USD by Nahum\nSell 5 BTC at 1500 USD by Nahum\nSell 1 BTC at 158 USD by Nahum\nBuy 7 BTC at 158 USD by Nahum\nSell 7 BTC at 158 USD by Nahum\nSell 20 BTC at 2000 USD by Nahum\nOfria's Open Orders (in chronological order):\nOfria's Filled Orders (in chronological order):\nSell 12 ETH at 158 USD by Ofria\nSell 10 ETH at 140 USD by Ofria\nSell 10 ETH at 1200 USD by Ofria\nZaabar's Open Orders (in chronological order):\nBuy 10 LTC at 450 USD by Zaabar\nBuy 190 ETH at 1200 USD by Zaabar\nZaabar's Filled Orders (in chronological order):\nSell 10 LTC at 600 USD by Zaabar\nBuy 5 BTC at 1500 USD by Zaabar\nBuy 5 BTC at 1500 USD by Zaabar\nBuy 10 ETH at 1200 USD by Zaabar\n");
//     return 0;
// }