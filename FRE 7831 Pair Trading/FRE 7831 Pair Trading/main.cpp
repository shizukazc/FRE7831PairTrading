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
#include <ctime>
#include <unistd.h>
#include <unordered_set>
#include "Database.hpp"
#include "MarketData.hpp"
#include "PairTrading.hpp"

using namespace std;

const char *PAIR_FILE = "./PairTrading.txt";
const char *DB_FILE = "./PairTrading.db";

// Model build date is from the start of 2011 to the end of 2020
const string HIST_SDATE = "2011-01-01";
const string HIST_EDATE = "2020-12-31";

// The real backtest period starts from 2021-01-04 (the first trading day).
// In order for that day to perform pair trading, we move the start date
//  one trading day forward, which is 2020-12-31.
const string BT_SDATE = "2020-12-31";

map<char,pair<string,string>> CreateMenu()
{
    map<char,pair<string,string>> menu;
    
    menu.insert({ 'A', pair<string,string>(
        "Create Database & Tables",
        "Create database PairTrading.db and table StockPairs, PairOnePrices, PairTwoPrices, and PairPrices, in order.")
    });
    menu.insert({ 'B', pair<string,string>(
        "Populate StockPairs",
        "Read stock pairs from local file and populate them in table StockPairs.")
    });
    menu.insert({ 'C', pair<string,string>(
        "Retrieve & Populate for Each Stock",
        "Retrieve the historical data of each stock in table StockPairs and populate the data into table PairOnePrices, PairTwoPrices, PairPrices, in order.")
    });
    menu.insert({ 'D', pair<string,string>(
        "Update Volatility",
        "Based on the historical data in table PairPrices, calculate the volatility of each stock pair's close price ratio")
    });
    menu.insert({ 'E', pair<string,string>(
        "Backtest",
        "Perform pair trading for period from 2021-01-01 to today. P&L for each pair on each date is calculated. Results printed to console.")
    });
    menu.insert({ 'F', pair<string,string>(
        "Write Backtest Results",
        "Save the backtest result to database.")
    });
    menu.insert({ 'G', pair<string,string>(
        "Probation Test",
        "Manual testing. User gets input start date and end date of the period on which to perform pairtrading. Results printed to console.")
    });
    menu.insert({ 'H', pair<string,string>(
        "Drop All Tables",
        "Drop table PairPrices, PairOnePrices, PairTwoPrices, and StockPairs, in order.")
    });
    menu.insert({ 'M', pair<string,string>(
        "Describe Option",
        "Display the detailed explanation about a given option.")
    });
    menu.insert({ 'X', pair<string,string>(
        "Exit",
        "Terminate the program and exit.")
    });
    
    return menu;
}

void PrintMenu(const map<char,pair<string,string>> &menu)
{
    cout << "Menu" << endl;
    cout << "========================================" << endl;
    for (const pair<const char,pair<string,string>> &p : menu)
    {
        cout << p.first << " - " << p.second.first << endl;
    }
    cout << endl;
    cout << "Enter selection: ";
}

int GetSelection(char &selection)
{
    std::string user_input;
    
    cin >> user_input;
    if (user_input.length() != 1) { return -1; }
    
    // Convert the selection to upper case
    transform(user_input.begin(), user_input.end(), user_input.begin(), ::toupper);
    
    selection = user_input[0];
    
    return 0;
}

int main()
{
    if (access(DB_FILE, F_OK) != -1) { remove(DB_FILE); }
    
    map<char,pair<string,string>> menu = CreateMenu();
    char selection = NULL;
    
    sqlite3 *db = NULL;
    
    vector<pair<string,string>> PairVec;
    unordered_set<string> PairOneStocks;
    unordered_set<string> PairTwoStocks;
    map<string,Stock> StockMap;
    
    time_t t = std::time(0);
    tm *now = std::localtime(&t);
    string today = to_string(now->tm_year + 1900) + "-"
        + to_string(now->tm_mon + 1) + "-" + to_string(now->tm_mday);
    const string BT_EDATE = today;

    do
    {
        PrintMenu(menu);
        if (GetSelection(selection) != 0)
        {
            cerr << "Invalid selection. Choose again." << endl;
            continue;
        }
        
        switch (selection) {
            case 'A':
            {
                // Create database
                if (CreateDatabase(DB_FILE) != SQLITE_OK)
                {
                    cerr << "Failed to create database " << DB_FILE << endl;
                    return -1;
                }

                cout << "Created database " << DB_FILE << endl;

                // Open database
                if (OpenDatabase(DB_FILE, db) != 0) { return -1; }

                // Create tables
                if (CreateTables(db) != 0)
                {
                    CloseDatabase(db);
                    return -1;
                }
                
                break;
            }

            case 'B':
            {
                if (ReadPairsFromFile(PAIR_FILE, PairVec) != 0)
                {
                    cerr << "ERROR: Failed to read from " << PAIR_FILE << endl;
                    CloseDatabase(db);
                    return -1;
                }
                
                if (InsertStockPairs(db, PairVec) != 0)
                {
                    cerr << "ERROR: Failed to insert data into StockPairs" << endl;
                    CloseDatabase(db);
                    return -1;
                }
                
                break;
            }
                
            case 'C':
            {
                for (const pair<string,string> &p : PairVec)
                {
                    string symbol1 = p.first;
                    string symbol2 = p.second;

                    if (PairOneStocks.find(symbol1) == PairOneStocks.end())
                    {
                        string url_request = BuildDailyRequestURL(symbol1, HIST_SDATE, BT_EDATE);
                        string read_buffer;
                        vector<TradeData> TradeDataVec;
                        
                        if (PullMarketData(url_request, read_buffer) != 0)
                        {
                            cerr << "ERROR: Failed to pull market data: " << symbol1 << endl;
                            CloseDatabase(db);
                            return -1;
                        }
                        
                        if (ParseJson(read_buffer, TradeDataVec) != 0)
                        {
                            cerr << "ERROR: ParseJson() failed for " << symbol1 << std::endl;
                            CloseDatabase(db);
                            return -1;
                        }
                        
                        Stock stock(symbol1, TradeDataVec);
                        
                        if (InsertPairKPrices(db, true, stock) != 0)
                        {
                            cerr << "ERROR: Failed to insert data into PairOnePrices" << endl;
                        }
                        
                        // Add the stock into the set
                        PairOneStocks.insert(symbol1);
                    }
                    
                    if (PairTwoStocks.find(symbol2) == PairTwoStocks.end())
                    {
                        string url_request = BuildDailyRequestURL(symbol2, HIST_SDATE, BT_EDATE);
                        string read_buffer;
                        vector<TradeData> TradeDataVec;
                        
                        if (PullMarketData(url_request, read_buffer) != 0)
                        {
                            cerr << "ERROR: Failed to pull market data: " << symbol2 << endl;
                            CloseDatabase(db);
                            return -1;
                        }
                        
                        if (ParseJson(read_buffer, TradeDataVec) != 0)
                        {
                            cerr << "ERROR: ParseJson() failed for " << symbol2 << std::endl;
                            CloseDatabase(db);
                            return -1;
                        }
                        
                        Stock stock(symbol2, TradeDataVec);
                        
                        if (InsertPairKPrices(db, false, stock) != 0)
                        {
                            cerr << "ERROR: Failed to insert data into PairTwoPrices" << endl;
                        }
                        
                        // Add the stock into the set
                        PairTwoStocks.insert(symbol2);
                    }
                }
                
                if (InsertPairPrices(db) != 0)
                {
                    cerr << "ERROR: Failed to insert into PairPrices" << endl;
                    CloseDatabase(db);
                    return -1;
                }
                
                break;
            }

            case 'D':
            {
                if (UpdateStockPairsVolatility(db, BT_SDATE) != 0)
                {
                    cerr << "ERROR: Failed to update StockPairs volatility" << endl;
                    CloseDatabase(db);
                    return -1;
                }
                
                break;
            }

            case 'E':
            {
                vector<StockPairPrices> StockPairPricesVec;
                
                for (const std::pair<std::string,std::string> &p : PairVec)
                {
                    StockPairPrices spp(p);
                    
                    if (GetPairPriceMap(db, BT_SDATE, BT_EDATE, spp) != 0)
                    {
                        cerr << "ERROR: GetPairPriceMap() failed" << endl;
                        CloseDatabase(db);
                        return -1;
                    }
                    
                    double volatility = 0.;
                    if (SelectStockPairsVolatility(db, spp.GetStockPair(), volatility) != 0)
                    {
                        cerr << "ERROR: SelectStockPairsVolatility() failed" << endl;
                        CloseDatabase(db);
                        return -1;
                    }

                    spp.SetVolatility(volatility);
                    spp.SetK(1.);
                    
                    StockPairPricesVec.push_back(spp);
                }
                
                // Perform calculation
                PairTradePerform(StockPairPricesVec);
                
                for (const StockPairPrices &spp : StockPairPricesVec) { cout << spp << endl; }
                
                break;
            }

            case 'F':
            {
                break;
            }

            case 'G':
            {
                break;
            }

            case 'H':
            {
                break;
            }
                
            // Special purpose
            case 'M':
            {
                cout << "Selection: ";
                if (GetSelection(selection) != 0 || menu.find(selection) == menu.end())
                {
                    cerr << "Invalid selection. Going back to menu." << endl;
                }
                else
                {
                    cout << endl;
                    cout << "INFO: " << menu[selection].first << endl;
                    cout << menu[selection].second << endl;
                }
                break;
            }
                
            case 'X':
            {
                break;
            }

            default:
            {
                cerr << "Invalid selection. Choose again." << endl;
                break;
            }
        }
        
        cout << endl;

    } while (selection != 'X');

    CloseDatabase(db);
    cout << "Program terminated." << endl;
    return 0;
}

//int main() {
//
//    vector<pair<string,string>> PairVec;
//
//    if (ReadPairsFromFile(pairfile, PairVec) != 0)
//    {
//        cerr << "ERROR: Failed to read from " << pairfile << endl;
//        return -1;
//    }
//
//    // TODO: This is just for getting bak file. Modify it once done.
//    bool init_env = true;
//    // bool init_env = (argc > 1 && std::string(args[1]) == "--init");
//
//    if (init_env)
//    {
//        // Initialize the environment for the program to run
//        if (initialize(PairVec) != 0)
//        {
//            cerr << "ERROR: initialize() failed" << endl;
//            return -1;
//        }
//    }
//
//    // TODO: This is just for getting bak file. Remove it once done.
//    return 0;
//
//    sqlite3 *db;
//
//    // Open database
//    if (OpenDatabase(db) != 0) { return -1; }
//
//    time_t t = std::time(0);
//    tm *now = std::localtime(&t);
//    string today = to_string(now->tm_year + 1900) + "-" + to_string(now->tm_mon + 1) + "-" + to_string(now->tm_mday);
//
//    string backtest_start_date = "2020-12-31";
//    string backtest_end_date = today;
//
//    std::vector<StockPairPrices> StockPairPricesVec;
//
//    if (PopulateStockPairPrices(PairVec, backtest_start_date, backtest_end_date, StockPairPricesVec) != 0)
//    {
//        cerr << "ERROR: PopulateStockPairPrices() failed" << endl;
//        return -1;
//    }
//
//    // Prepare for the calculation
//    for (StockPairPrices &spp : StockPairPricesVec)
//    {
//        double volatility = 0.;
//        if (SelectStockPairsVolatility(db, spp.GetStockPair(), volatility) != 0)
//        {
//            cerr << "ERROR: SelectStockPairsVolatility() failed" << endl;
//            CloseDatabase(db);
//            return -1;
//        }
//
//        spp.SetVolatility(volatility);
//        spp.SetK(1.);
//    }
//
//    // Perform calculation
//    PairTradePerform(StockPairPricesVec);
//
//    for (StockPairPrices &spp : StockPairPricesVec)
//    {
//        pair<string,string> StockPair = spp.GetStockPair();
//        cout << "Stock1=" << StockPair.first << ", Stock2=" << StockPair.second << endl;
//
//        map<std::string,PairPrice> &dailyPairPrices = spp.GetDailyPrices();
//        for (const std::pair<const std::string,PairPrice> &dp : dailyPairPrices)
//        {
//            cout << "  Date=" << dp.first << ", P&L=" << dp.second.dProfitLoss << endl;
//        }
//    }
//
//    cout << "Here is the menu" << endl;
//
//    return 0;
//}
