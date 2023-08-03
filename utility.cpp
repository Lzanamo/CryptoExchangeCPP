#include "utility.hpp"
#include "useraccount.hpp"
#include <iostream>
#include <sstream>

bool Order::if_possible(Useraccount &user) const{
    if (side == "Buy"){
        for (auto &i : user.assets_){
            if (i.first == "USD" && i.second >= price * amount){
                return true;
            }
        }
    } else if (side == "Sell"){
        for (auto &i : user.assets_){
            if (i.first == asset && i.second >= amount){
                return true;
            }
        }
    }
    return false;
}

std::ostream& operator<<(std::ostream &oss, const Order &x){
    oss << x.side << " " << x.amount << " " << x.asset << " at " << x.price << " USD by " << x.username;
    return oss;
}

bool operator==(const Order &a, const Order &b){
    if (a.username == b.username && a.amount && b.amount && a.asset == b.asset && a.price == b.price && a.side == b.side){
        return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream &oss, const Trade &x){
    oss << x.buyer_username << " Bought " << x.amount << " of " << x.asset << " From " << x.seller_username << " for " << x.price << " USD" << std::endl;
    return oss;
}

// int main(){
//     Order o1 {"Nahum", "Buy", "BTC", 100, 60000};
//     Order o2(o1);
//     std::ostringstream oss;
//     oss << o1;
//     //CHECK(oss.str() == "Buy 100 BTC at 60000 USD by Nahum");
//     oss.str("");
//     oss << Order{"Dolson", "Sell", "ETH", 9, 888};
//     //CHECK(oss.str() == "Sell 9 ETH at 888 USD by Dolson");
//     return 0;
// }