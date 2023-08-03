#pragma once
#include "utility.hpp"
#include <string>
#include <vector>
#include <map>

struct Order;

class Useraccount{
    public:
        std::string name_;
        std::map<std::string, int> assets_;
        std::vector<Order> filled_orders;
        std::vector<Order> open_orders;
        Useraccount(const std::string &username, const std::string &asset, int amount);
};