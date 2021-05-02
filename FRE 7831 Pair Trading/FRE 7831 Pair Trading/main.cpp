//
//  main.cpp
//  FRE 7831 Pair Trading
//
//  Created by Zetian Chen on 4/29/21.
//

#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include "Database.hpp"
#include "MarketData.hpp"
#include "PairTrading.hpp"

using namespace std;

int main() {
    const char *pairfile = "./PairTrading.txt";
    
    int ret = CreateDatabase();
    
    if (ret != SQLITE_OK)
    {
        cerr << "Failed to create database " << PairTradingDBPath << endl;
        return -1;
    }
    
    cout << "Created database " << PairTradingDBPath << endl;
    
    sqlite3 *db;
    
    if (OpenDatabase(db) != 0) { return -1; }
    if (CreateTables(db) != 0) { return -1; }
    
    int rc = 0;
    
    vector<pair<string,string>> PairVec;
    
    if ((rc = ReadPairsFromFile(pairfile, PairVec)) != 0)
    {
        cerr << "Failed to read from " << pairfile << endl;
        return -1;
    }
    
    string url_common_daily = "https://eodhistoricaldata.com/api/eod/";
    string api_token = "605f41c6a29c69.96773141";
    string url_request_daily1;
    string url_request_daily2;
    
    string start_date = "2021-01-01";
    string end_date = "2021-02-01";
    
    map<string,Stock> StockMap;
    
    for (const pair<string,string> &p : PairVec)
    {
        string symbol1 = p.first;
        string symbol2 = p.second;
        
        url_request_daily1 = url_common_daily + symbol1 + ".US?from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
        url_request_daily2 = url_common_daily + symbol2 + ".US?from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
        
        string read_buffer;
        if (PullMarketData(url_request_daily1, read_buffer) != 0)
        {
            cerr << "Failed to pull market data: " << symbol1 << endl;
            CloseDatabase(db);
            return -1;
        }
        
        if (PopulateStocks(read_buffer, symbol1, StockMap) != 0)
        {
            cerr << "Failed to insert into StockMap: " << symbol1 << endl;
            CloseDatabase(db);
            return -1;
        }
        
        read_buffer.clear();
        if (PullMarketData(url_request_daily2, read_buffer) != 0)
        {
            cerr << "Failed to pull market data: " << symbol2 << endl;
            CloseDatabase(db);
            return -1;
        }
        
        if (PopulateStocks(read_buffer, symbol2, StockMap) != 0)
        {
            cerr << "Failed to insert into StockMap: " << symbol2 << endl;
            CloseDatabase(db);
            return -1;
        }
    }
    
    if ((rc = InsertIndividualPrices(db, StockMap, PairVec)) != 0)
    {
        cerr << "Failed to insert data into PairOnePrices and PairTwoPrices" << endl;
        CloseDatabase(db);
        return -1;
    }
    
//    vector<string> DateVec;
//    if (GetDateVec(StockMap, DateVec) != 0)
//    {
//        cerr << "Failed to get DateVec" << endl;
//        CloseDatabase(db);
//        return -1;
//    }
    
    if ((rc = InsertPairPrices(db)) != 0)
    {
        cerr << "Failed to insert into PairPrices" << endl;
        CloseDatabase(db);
        return -1;
    }
    
    CloseDatabase(db);
    
    return 0;
}
