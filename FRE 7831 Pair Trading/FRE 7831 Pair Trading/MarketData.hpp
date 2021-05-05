//
//  MarketData.hpp
//  FRE 7831 Pair Trading
//
//  Created by Zetian Chen on 4/29/21.
//

#ifndef MarketData_hpp
#define MarketData_hpp

#include <vector>
#include <utility>
#include <string>
#include <map>
#include "PairTrading.hpp"

int ReadPairsFromFile(const char *infile, std::vector<std::pair<std::string,std::string>> &PairVec);
int PullMarketData(const std::string &url_request, std::string &read_buffer);
// int PopulateStocks(const std::string &read_buffer, std::string symbol, std::string bt_sdate, std::map<std::string,Stock> &StockMap);
std::string BuildDailyRequestURL(std::string symbol, std::string start_date, std::string end_date);

// For backtest
int ParseJson(const std::string &read_buffer, std::vector<TradeData> &TradeDataVec);
// int PopulateStockPairPrices(const std::vector<std::pair<std::string,std::string>> &PairVec,
//                             std::string start_date, std::string end_date, std::vector<StockPairPrices> &StockPairPricesVec);
void PairTradePerform(std::vector<StockPairPrices> &StockPairPricesVec);

#endif /* MarketData_hpp */
