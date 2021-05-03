//
//  Database.hpp
//  FRE 7831 Pair Trading
//
//  Created by Zetian Chen on 4/29/21.
//

#ifndef Database_hpp
#define Database_hpp

#include <sqlite3.h>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include "PairTrading.hpp"

#define PairTradingDBPath "./PairTrading.db"

int CreateDatabase();
int OpenDatabase(sqlite3 * &db);
int CreateTables(sqlite3 * &db);
void CloseDatabase(sqlite3 * &db);
int InsertIndividualPrices(sqlite3 * &db, std::map<std::string,Stock> &StockMap, const std::vector<std::pair<std::string,std::string>> &PairVec);

/* Special Purpose */
int InsertStockPairs(sqlite3 * &db, const std::vector<std::pair<std::string,std::string>> &PairVec);
int InsertPairPrices(sqlite3 * &db);
int UpdateStockPairsVolatility(sqlite3 * &db, std::string bt_date);

int SelectStockPairsVolatility(sqlite3 * &db, const std::pair<std::string,std::string> &StockPair, double &volatility);

#endif /* Database_hpp */
