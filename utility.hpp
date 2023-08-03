#pragma once
#include "useraccount.hpp"
#include <iostream>
#include <string>

class Useraccount;

struct Order {
  std::string username;
  std::string side;  // Can be "Buy" or "Sell"
  std::string asset;
  int amount;
  int price;
  bool if_possible(Useraccount &user) const;
  friend std::ostream& operator<<(std::ostream &oss, const Order &x);
};

bool operator==(const Order &a, const Order &b);

struct Trade {
  std::string buyer_username;
  std::string seller_username;
  std::string asset;
  int amount;
  int price;
};
std::ostream& operator<<(std::ostream &oss, const Trade &x);