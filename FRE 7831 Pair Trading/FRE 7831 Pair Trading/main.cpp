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

const char *pairfile = "./PairTrading.txt";

int initialize(const vector<pair<string,string>> &PairVec)
{
    // Create database
    if (CreateDatabase() != SQLITE_OK)
    {
        cerr << "Failed to create database " << PairTradingDBPath << endl;
        return -1;
    }

    cout << "Created database " << PairTradingDBPath << endl;

    sqlite3 *db;

    // Open database
    if (OpenDatabase(db) != 0)
    {
        return -1;
    }

    // Create tables
    if (CreateTables(db) != 0)
    {
        CloseDatabase(db);
        return -1;
    }
    
    map<string,Stock> StockMap;
    
    string url_request_daily1;
    string url_request_daily2;

    // Model build date is from the start of 2011 to the end of 2020
    string start_date = "2019-01-01";
    string end_date = "2020-12-31";
    string bt_start_date = "2021-01-01";

    for (const pair<string,string> &p : PairVec)
    {
        string symbol1 = p.first;
        string symbol2 = p.second;

        url_request_daily1 = BuildDailyRequestURL(symbol1, start_date, end_date);
        url_request_daily2 = BuildDailyRequestURL(symbol2, start_date, end_date);

        string read_buffer;
        if (PullMarketData(url_request_daily1, read_buffer) != 0)
        {
            cerr << "ERROR: Failed to pull market data: " << symbol1 << endl;
            goto error_exit;
        }

        if (PopulateStocks(read_buffer, symbol1, StockMap) != 0)
        {
            cerr << "ERROR: Failed to insert into StockMap: " << symbol1 << endl;
            goto error_exit;
        }

        read_buffer.clear();
        if (PullMarketData(url_request_daily2, read_buffer) != 0)
        {
            cerr << "ERROR: Failed to pull market data: " << symbol2 << endl;
            goto error_exit;
        }

        if (PopulateStocks(read_buffer, symbol2, StockMap) != 0)
        {
            cerr << "ERROR: Failed to insert into StockMap: " << symbol2 << endl;
            goto error_exit;
        }
    }

    if (InsertIndividualPrices(db, StockMap, PairVec) != 0)
    {
        cerr << "ERROR: Failed to insert data into PairOnePrices and PairTwoPrices" << endl;
        goto error_exit;
    }
    
    if (InsertStockPairs(db, PairVec) != 0)
    {
        cerr << "ERROR: Failed to insert data into StockPairs" << endl;
        goto error_exit;
    }

    if (InsertPairPrices(db) != 0)
    {
        cerr << "ERROR: Failed to insert into PairPrices" << endl;
        goto error_exit;
    }
    
    if (UpdateStockPairsVolatility(db, bt_start_date) != 0)
    {
        cerr << "ERROR: Failed to update StockPairs volatility" << endl;
        goto error_exit;
    }

    CloseDatabase(db);
    return 0;
    
error_exit:
    CloseDatabase(db);
    return -1;
}

int main(int argc, char *args[]) {
    
    vector<pair<string,string>> PairVec;

    if (ReadPairsFromFile(pairfile, PairVec) != 0)
    {
        cerr << "ERROR: Failed to read from " << pairfile << endl;
        return -1;
    }
    
    bool init_env = (argc > 1 && std::string(args[1]) == "--init");
    
    if (init_env)
    {
        // Initialize the environment for the program to run
        if (initialize(PairVec) != 0)
        {
            cerr << "ERROR: initialize() failed" << endl;
            return -1;
        }
    }
    
    cout << "Here is the menu" << endl;
    
    return 0;
}
