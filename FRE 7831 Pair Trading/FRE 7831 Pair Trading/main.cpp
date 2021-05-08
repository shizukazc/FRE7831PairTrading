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
#include <iomanip>
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

const unsigned int STOCK_SYMBOL_LENGTH_MAX = 9;

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
        "User gets input start date and end date of the period on which to perform pairtrading. Results printed to console.")
    });
    menu.insert({ 'H', pair<string,string>(
        "Manual Test",
        "User gets to input a pair of stocks for which pairtrading is performed on 5/7/2021."
    ) });
    menu.insert({ 'I', pair<string,string>(
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
    string user_input;
    
    cin >> user_input;
    if (user_input.length() != 1) { return -1; }
    
    // Convert the selection to upper case
    transform(user_input.begin(), user_input.end(), user_input.begin(), ::toupper);
    
    selection = user_input[0];
    
    return 0;
}

int GetDate(string &date)
{
    string user_input;
    cin >> user_input;
    
    struct tm date_tm;
    char buf[32];
    
    if (strptime(user_input.c_str(), "%Y-%m-%d", &date_tm))
    {
        strftime(buf, 32, "%Y-%m-%d", &date_tm);
        date = string(buf);
        return 0;
    }
    
    return -1;
}

int GetK(double &K)
{
    string user_input;
    cin >> user_input;
    
    K = stod(user_input);
    
    return K > 0 ? 0 : -1;
}

int GetSymbol(string &symbol)
{
    string user_input;
    cin >> user_input;
    
    if (user_input.length() > STOCK_SYMBOL_LENGTH_MAX) return -1;
    
    symbol = user_input;
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
    vector<StockPairPrices> BTStockPairPricesVec;
    vector<StockPairPrices> PTStockPairPricesVec;
    
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
                BTStockPairPricesVec.clear();
                
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
                    
                    BTStockPairPricesVec.push_back(spp);
                }
                
                // Perform calculation
                PairTradePerform(BTStockPairPricesVec);
                
                for (const StockPairPrices &spp : BTStockPairPricesVec) { cout << spp << endl; }
                
                break;
            }

            case 'F':
            {
                if (UpdateBacktestPnL(db, BTStockPairPricesVec) != 0)
                {
                    cerr << "ERROR: UpdateBacktestPnL() failed" << endl;
                    CloseDatabase(db);
                    return -1;
                }
                
                break;
            }

            case 'G':
            {
                PTStockPairPricesVec.clear();
                
                string probtest_sdate;
                string probtest_edate;
                double K = 1.;
                int status = 0;
                
                do
                {
                    cout << "Enter probation test start date: ";
                    if (GetDate(probtest_sdate))
                    {
                        cerr << "Invalid date. Going back to menu." << endl;
                        status = -1;
                        break;
                    }
                    
                    if (probtest_sdate < BT_SDATE)
                    {
                        cerr << "Probation test start date cannot be earlier than " << BT_SDATE << "." << endl;
                        status = -1;
                        break;
                    }
                    
                    cout << "Enter probation test end date: ";
                    if (GetDate(probtest_edate))
                    {
                        cerr << "Invalid date. Going back to menu." << endl;
                        status = -1;
                        break;
                    }
                    
                    if (probtest_edate > BT_EDATE)
                    {
                        cerr << "Probation test end date cannot be later than " + BT_EDATE + "." << endl;
                        status = -1;
                        break;
                    }
                    
                    if (probtest_edate <= probtest_sdate)
                    {
                        cerr << "Probation test end date cannot be earlier than its start date." << endl;
                        cerr << "Going back to menu." << endl;
                        status = -1;
                        break;
                    }
                    
                    cout << "Enter K (K>0): ";
                    if (GetK(K))
                    {
                        cerr << "Invalid K value. Going back to menu." << endl;
                        status = -1;
                        break;
                    }
                } while (false);
                
                if (!status)
                {
                    for (const std::pair<std::string,std::string> &p : PairVec)
                    {
                        StockPairPrices spp(p);
                        
                        if (GetPairPriceMap(db, probtest_sdate, probtest_edate, spp) != 0)
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
                        spp.SetK(K);
                        
                        PTStockPairPricesVec.push_back(spp);
                    }
                    
                    // Perform calculation
                    PairTradePerform(PTStockPairPricesVec);
                    
                    for (const StockPairPrices &spp : PTStockPairPricesVec) { cout << spp << endl; }
                }
                
                break;
            }
                
            case 'H':
            {
                string symbol1;
                string symbol2;
                
                cout << "Enter Stock Symbol #1: ";
                if (GetSymbol(symbol1))
                {
                    cerr << "ERROR: Invalid stock symbol " << symbol1 << ". Going back to menu." << endl;
                    continue;
                }
                
                cout << "Enter Stock Symbol #2: ";
                if (GetSymbol(symbol2))
                {
                    cerr << "ERROR: Invalid stock symbol " << symbol2 << ". Going back to menu." << endl;
                    continue;
                }
                
                // See if the pair exists
                bool pairExists = false;
                for (const pair<string,string> &p : PairVec)
                {
                    if (p.first == symbol1 && p.second == symbol2)
                    {
                        pairExists = true;
                        break;
                    }
                }
                
                if (!pairExists)
                {
                    cerr << "ERROR: Stock pair (" << symbol1 << "," << symbol2 << ") does not exist. Going back to menu." << endl;
                    continue;
                }
                
                PairPrice pp56;
                PairPrice pp57;
                double K = 0.;
                double volatility = 0.;
                
                // Exists
                for (StockPairPrices &spp : BTStockPairPricesVec)
                {
                    const pair<string,string> &sp = spp.GetStockPair();
                    if (sp.first == symbol1 && sp.second == symbol2)
                    {
                        // found
                        map<string,PairPrice> &dpp = spp.GetDailyPrices();
                        pp56 = dpp[string("2021-05-06")];
                        pp57 = dpp[string("2021-05-07")];
                        K = spp.GetK();
                        volatility = spp.GetVolatility();
                        break;
                    }
                }
                
                pair<int,int> strategy = GetStrategy(pp56.dClose1, pp56.dClose2, pp57.dOpen1, pp57.dOpen2, pp57.dClose1, pp57.dClose2, K, volatility);
                int N1 = strategy.first;
                int N2 = strategy.second;
                
                cout << "Pair=(" << symbol1 << "," << symbol2 << "), K=" << K << ", Volatility=" << setprecision(20) << volatility << endl;
                cout << setprecision(2);
                cout << "Date=2021-05-07, close1d1=" << pp56.dClose1 << ", close2d1=" << pp56.dClose2
                    << ", open1d2=" << pp57.dOpen1 << ", open2d2=" << pp57.dOpen2 << ", close1d2=" << pp57.dClose1 << ", close2d2=" << pp57.dClose2 << endl;
                cout << "Strategy=" << (N1 > 0 ? "Long" : "Short") << ", N1=" << N1 << ", N2=" << N2 << ", P&L=" << pp57.dProfitLoss << endl;
                
                break;
            }

            case 'I':
            {
                if (DropTables(db) != 0)
                {
                    cerr << "ERROR: Failed to Drop tables." << endl;
                    CloseDatabase(db);
                    return -1;
                }
                cout << "All tables are dropped." << endl;
                
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
                    selection = 'M';  // Change it back
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
