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

int CreateDatabase(const char *dbfile);
int OpenDatabase(const char *dbfile, sqlite3 * &db);
int CreateTables(sqlite3 * &db);
int DropTables(sqlite3 * &db);
void CloseDatabase(sqlite3 * &db);

int InsertPairKPrices(sqlite3 * &db, bool IsPairOne, const Stock &stock);
int InsertStockPairs(sqlite3 * &db, const std::vector<std::pair<std::string,std::string>> &PairVec);
int InsertPairPrices(sqlite3 * &db);
int UpdateStockPairsVolatility(sqlite3 * &db, std::string bt_date);
int SelectStockPairsVolatility(sqlite3 * &db, const std::pair<std::string,std::string> &StockPair, double &volatility);
int GetPairPriceMap(sqlite3 * &db, std::string sdate, std::string edate, StockPairPrices &spp);
int UpdateBacktestPnL(sqlite3 * &db, std::vector<StockPairPrices> &StockPairPricesVec);

#endif /* Database_hpp */
