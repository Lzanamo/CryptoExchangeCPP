#include "useraccount.hpp"
#include <iostream>
#include <string>
using std::string;
#include <unordered_map>
using std::unordered_map;
using std::pair;


Useraccount::Useraccount(const std::string &username, const std::string &asset,int amount){
    name_ = username;
    assets_[asset] = amount;
}

// int main(){
//     Useraccount c{"Dolson" , "BTC", 17};
//     std::cout << c.name_ << " ";
//     for (auto &i : c.assets_){
//         std::cout << i.first << " " << i.second;
//     }
//     return 0;
// }