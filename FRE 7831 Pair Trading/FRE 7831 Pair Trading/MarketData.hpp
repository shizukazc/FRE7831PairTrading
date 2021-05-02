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
//int GetDateVec(const std::map<std::string,Stock> &StockMap, std::vector<std::string> &DateVec);
int PullMarketData(const std::string& url_request, std::string& read_buffer);
int PopulateStocks(const std::string &read_buffer, std::string symbol, std::map<std::string,Stock> &StockMap);
//int PopulateStockPairPrices(const std::vector<std::pair<std::string,std::string>> &PairVec, const std::map<std::string,Stock> &StockMap, const std::vector<std::string> DateVec, std::vector<StockPairPrices> &StockPairPricesVec);

#endif /* MarketData_hpp */
