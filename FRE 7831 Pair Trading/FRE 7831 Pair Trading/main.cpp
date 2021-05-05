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
#include "Database.hpp"
#include "MarketData.hpp"
#include "PairTrading.hpp"

using namespace std;

const char *pairfile = "./PairTrading.txt";

// Model build date is from the start of 2011 to the end of 2020
string historical_start_date = "2011-01-01";
string historical_end_date = "2020-12-31";
string backtest_start_date = "2021-01-01";

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

    for (const pair<string,string> &p : PairVec)
    {
        string symbol1 = p.first;
        string symbol2 = p.second;

        url_request_daily1 = BuildDailyRequestURL(symbol1, historical_start_date, historical_end_date);
        url_request_daily2 = BuildDailyRequestURL(symbol2, historical_start_date, historical_end_date);

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
    
    if (UpdateStockPairsVolatility(db, backtest_start_date) != 0)
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

map<char,pair<string,string>> CreateMenu()
{
    map<char,pair<string,string>> menu;
    
    menu.insert({ 'A', pair<string,string>(
        "Create Tables",
        "Create table StockPairs, PairOnePrices, PairTwoPrices, and PairPrices, in order.")
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
    map<char,pair<string,string>> menu = CreateMenu();
    
    char selection = NULL;

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
                break;
            }

            case 'B':
            {
                break;
            }

            case 'C':
            {
                break;
            }

            case 'D':
            {
                break;
            }

            case 'E':
            {
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
