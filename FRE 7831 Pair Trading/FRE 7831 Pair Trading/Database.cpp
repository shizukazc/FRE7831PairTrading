//
//  Database.cpp
//  FRE 7831 Pair Trading
//
//  Created by Zetian Chen on 4/29/21.
//

#include "Database.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

int CreateDatabase()
{
    sqlite3 *pDb = NULL;
    int ret = 0;
    
    do  // Avoid nested if-statements
    {
        // Initialize engine
        if ((ret = sqlite3_initialize()) != SQLITE_OK)
        {
            std::cerr << "Failed to initialize library: " << ret << std::endl;
            break;
        }
        
        // Establish connection to a database
        if ((ret = sqlite3_open_v2(PairTradingDBPath, &pDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)))
        {
            std::cerr << "Failed to open connection: " << ret << std::endl;
            break;
        }
    } while (false);
    
    // Clean up
    if (NULL != pDb) { sqlite3_close(pDb); }
    
    sqlite3_shutdown();
    
    return ret;
}

int OpenDatabase(sqlite3 * &db)
{
    int rc = sqlite3_open(PairTradingDBPath, &db);
    
    if (rc)
    {
        std::cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << std::endl << std::endl;
        // Close connection
        sqlite3_close(db);
        return -1;
    }
    
    std::cout << "Opened database " << PairTradingDBPath << "." << std::endl << std::endl;
    
    return 0;
}

int CreateTables(sqlite3 * &db)
{
    int rc1 = 0;
    int rc2 = 0;
    int rc3 = 0;
    int rc4 = 0;
    
    char *error = NULL;
    
    std::vector<std::string> tables {
        std::string("PairPrices"),
        std::string("PairOnePrices"),
        std::string("PairTwoPrices"),
        std::string("StockPairs")
    };
    
    for (const std::string &table : tables)
    {
        std::string sql_droptable = std::string("DROP TABLE IF EXISTS ") + table + ";";
        
        if (sqlite3_exec(db, sql_droptable.c_str(), NULL, NULL, &error) != SQLITE_OK)
        {
            std::cerr << "SQLite3 cannot drop " << table << " table." << std::endl;
            sqlite3_free(error);
            return -1;
        }
    }
    
    // StockPairs
    
    std::cout << "Creating StockPairs table..." << std::endl;
    
    std::string sql_createtable = std::string("CREATE TABLE IF NOT EXISTS StockPairs ")
        + "(id INT NOT NULL,"
        + "symbol1 CHAR(20) NOT NULL,"
        + "symbol2 CHAR(20) NOT NULL,"
        + "volatility FLOAT NOT NULL,"
        + "profit_loss FLOAT NOT NULL,"
        + "PRIMARY KEY(symbol1, symbol2));";
    
    rc1 = sqlite3_exec(db, sql_createtable.c_str(), NULL, NULL, &error);

    if (rc1)
    {
        std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
        sqlite3_free(error);
    }
    else
    {
        std::cout << "Created StockPairs table." << std::endl << std::endl;
    }
    
    // PairOnePrices
    
    std::cout << "Creating PairOnePrices table..." << std::endl;
    
    sql_createtable = std::string("CREATE TABLE IF NOT EXISTS PairOnePrices ")
        + "(symbol CHAR(20) NOT NULL,"
        + "date CHAR(20) NOT NULL,"
        + "open REAL NOT NULL,"
        + "high REAL NOT NULL,"
        + "low REAL NOT NULL,"
        + "close REAL NOT NULL,"
        + "adjusted_close REAL NOT NULL,"
        + "volume INT NOT NULL,"
        + "PRIMARY KEY(symbol, date));";
    
    rc2 = sqlite3_exec(db, sql_createtable.c_str(), NULL, NULL, &error);

    if (rc2)
    {
        std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
        sqlite3_free(error);
    }
    else
    {
        std::cout << "Created PairOnePrices table." << std::endl << std::endl;
    }
    
    // PairTwoPrices
    
    std::cout << "Creating PairTwoPrices table..." << std::endl;
    
    sql_createtable = std::string("CREATE TABLE IF NOT EXISTS PairTwoPrices ")
        + "(symbol CHAR(20) NOT NULL,"
        + "date CHAR(20) NOT NULL,"
        + "open REAL NOT NULL,"
        + "high REAL NOT NULL,"
        + "low REAL NOT NULL,"
        + "close REAL NOT NULL,"
        + "adjusted_close REAL NOT NULL,"
        + "volume INT NOT NULL,"
        + "PRIMARY KEY(symbol, date));";
    
    rc3 = sqlite3_exec(db, sql_createtable.c_str(), NULL, NULL, &error);

    if (rc3)
    {
        std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
        sqlite3_free(error);
    }
    else
    {
        std::cout << "Created PairTwoPrices table." << std::endl << std::endl;
    }
    
    // PairPrices
    
    std::cout << "Creating PairPrices table..." << std::endl;
    
    sql_createtable = std::string("CREATE TABLE IF NOT EXISTS PairPrices ")
        + "(symbol1 CHAR(20) NOT NULL,"
        + "symbol2 CHAR(20) NOT NULL,"
        + "date CHAR(20) NOT NULL,"
        + "open1 REAL NOT NULL,"
        + "close1 REAL NOT NULL,"
        + "open2 REAL NOT NULL,"
        + "close2 REAL NOT NULL,"
        + "profit_loss REAL NOT NULL,"
        + "PRIMARY KEY(symbol1, symbol2, date),"
        + "FOREIGN KEY(symbol1, date) REFERENCES PairOnePrices(symbol, date) ON DELETE CASCADE ON UPDATE CASCADE,"
        + "FOREIGN KEY(symbol2, date) REFERENCES PairTwoPrices(symbol, date) ON DELETE CASCADE ON UPDATE CASCADE,"
        + "FOREIGN KEY(symbol1, symbol2) REFERENCES StockPairs(symbol1, symbol2) ON DELETE CASCADE ON UPDATE CASCADE);";
    
    rc4 = sqlite3_exec(db, sql_createtable.c_str(), NULL, NULL, &error);

    if (rc4)
    {
        std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
        sqlite3_free(error);
    }
    else
    {
        std::cout << "Created PairPrices table." << std::endl << std::endl;
    }
    
    return (rc1 || rc2 || rc3 || rc4) ? -1 : 0;
}

void CloseDatabase(sqlite3 * &db)
{
    std::cout << "Closing database " << PairTradingDBPath << "..." << std::endl;
    
    sqlite3_close(db);
    
    std::cout << "Closed database " << PairTradingDBPath << std::endl << std::endl;
}

int InsertIndividualPrices(sqlite3 * &db, std::map<std::string,Stock> &StockMap, const std::vector<std::pair<std::string,std::string>> &PairVec)
{
    int rc = 0;
    char *error;
    std::string date;
    double open;
    double high;
    double low;
    double close;
    double adjclose;
    long volume;
    
    std::string sql_command;
    
    for (const std::pair<std::string,std::string> &p : PairVec)
    {
        std::string symbol1 = p.first;
        std::string symbol2 = p.second;
        
        const Stock &stock1 = StockMap[symbol1];
        const Stock &stock2 = StockMap[symbol2];
        
        const std::vector<TradeData> &trades1 = stock1.GetTrades();
        const std::vector<TradeData> &trades2 = stock2.GetTrades();
        
        for (const TradeData &td : trades1)
        {
            date = td.GetDate();
            open = td.GetOpen();
            high = td.GetHigh();
            low = td.GetLow();
            close = td.GetClose();
            adjclose = td.GetAdjClose();
            volume = td.GetVolume();
            
            std::cout << "Inserting an entry into table PairOnePrices..." << std::endl;
            
            std::ostringstream ss;
            ss << "INSERT INTO PairOnePrices(symbol, date, open, high, low, close, adjusted_close, volume) VALUES('"
                << symbol1.c_str() << "','"
                << date.c_str() << "',"
                << open << ","
                << high << ","
                << low << ","
                << close << ","
                << adjclose << ","
                << volume << ")";
            sql_command = ss.str();
            
            rc = sqlite3_exec(db, sql_command.c_str(), NULL, NULL, &error);
            if (rc)
            {
                std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
                sqlite3_free(error);
            }
            else
            {
                std::cout << "Inserted an entry into table PairOnePrices." << std::endl << std::endl;
            }
        }
        
        for (const TradeData &td : trades2)
        {
            date = td.GetDate();
            open = td.GetOpen();
            high = td.GetHigh();
            low = td.GetLow();
            close = td.GetClose();
            adjclose = td.GetAdjClose();
            volume = td.GetVolume();
            
            std::cout << "Inserting an entry into table PairTwoPrices..." << std::endl;
            
            std::ostringstream ss;
            ss << "INSERT INTO PairTwoPrices(symbol, date, open, high, low, close, adjusted_close, volume) VALUES('"
                << symbol2.c_str() << "','"
                << date.c_str() << "',"
                << open << ","
                << high << ","
                << low << ","
                << close << ","
                << adjclose << ","
                << volume << ");";
            sql_command = ss.str();
            
            rc = sqlite3_exec(db, sql_command.c_str(), NULL, NULL, &error);
            if (rc)
            {
                std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
                sqlite3_free(error);
            }
            else
            {
                std::cout << "Inserted an entry into table PairTwoPrices." << std::endl << std::endl;
            }
        }
    }
    
    return 0;
}

int InsertStockPairs(sqlite3 * &db, const std::vector<std::pair<std::string,std::string>> &PairVec)
{
    int rc = 0;
    char *error;
    std::string sql_command;
    
    int id = 0;
    
    for (const std::pair<std::string,std::string> &p : PairVec)
    {
        const std::string &symbol1 = p.first;
        const std::string &symbol2 = p.second;
        id++;
        
        std::ostringstream ss;
        ss << "INSERT INTO StockPairs(id, symbol1, symbol2, volatility, profit_loss) VALUES("
            << id << ",'"
            << symbol1 << "','"
            << symbol2 << "',0.0,0.0);";
        sql_command = ss.str();
        
        rc = sqlite3_exec(db, sql_command.c_str(), NULL, NULL, &error);
        if (rc)
        {
            std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
            sqlite3_free(error);
        }
        else
        {
            std::cout << "Inserted an entry into table StockPairs." << std::endl << std::endl;
        }
    }
    
    return 0;
}

int InsertPairPrices(sqlite3 * &db)
{
    int rc = 0;
    char *error;
    
    const char *sql_command =
        "INSERT INTO PairPrices "
        "SELECT StockPairs.symbol1 AS symbol1, "
            "StockPairs.symbol2 AS symbol2, "
            "PairOnePrices.date AS date, "
            "PairOnePrices.open AS open1, "
//            "PairOnePrices.close AS close1, "
            "PairOnePrices.adjusted_close AS close1, "  // Use adjusted_close for historical data
            "PairTwoPrices.open AS open2, "
            "PairTwoPrices.adjusted_close AS close2, "  // Use adjusted_close for historical data
//            "PairTwoPrices.close AS close2, "
            "0 AS profit_loss "
        "FROM StockPairs, PairOnePrices, PairTwoPrices "
        "WHERE (((StockPairs.symbol1 = PairOnePrices.symbol) "
            "AND (StockPairs.symbol2 = PairTwoPrices.symbol)) "
            "AND (PairOnePrices.date = PairTwoPrices.date)) "
        "ORDER BY symbol1, symbol2;";

    rc = sqlite3_exec(db, sql_command, NULL, NULL, &error);
    if (rc)
    {
        std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
        sqlite3_free(error);
    }
    else
    {
        std::cout << "Done PairPrices insertion." << std::endl << std::endl;
    }
    
    return rc ? -1 : 0;
}

int UpdateStockPairsVolatility(sqlite3 * &db, std::string bt_date)
{
    int rc = 0;
    char *error;
    
    std::string sql_command = std::string("UPDATE StockPairs SET volatility = ")
        + "(SELECT(AVG((close1/close2)*(close1/close2))-AVG(close1/close2)*AVG(close1/close2)) AS variance "
        + "FROM PairPrices "
        + "WHERE StockPairs.symbol1 = PairPrices.symbol1 AND StockPairs.symbol2 = PairPrices.symbol2 AND PairPrices.date <= \'"
        + bt_date + "\');";
    
    rc = sqlite3_exec(db, sql_command.c_str(), NULL, NULL, &error);
    if (rc)
    {
        std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
        sqlite3_free(error);
    }
    else
    {
        std::cout << "Done updating StockPairs volatility." << std::endl << std::endl;
    }
    
    return rc ? -1 : 0;
}

int SelectStockPairsVolatility(sqlite3 * &db, const std::pair<std::string,std::string> &StockPair, double &volatility)
{
    int rc = 0;
    char *error;
    
    char **result;
    int row;
    int col;
    
    const std::string &symbol1 = StockPair.first;
    const std::string &symbol2 = StockPair.second;
    
    std::string sql_command = "SELECT volatility FROM StockPairs WHERE symbol1='" + symbol1 + "' AND symbol2='" + symbol2 + "';";
    
    rc = sqlite3_get_table(db, sql_command.c_str(), &result, &row, &col, &error);
    if (rc)
    {
        std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
        sqlite3_free(error);
    }
    else
    {
        std::cout << "Done retrieving StockPair(" + symbol1 + "," + symbol2 + ") volatility." << std::endl << std::endl;
    }

    volatility = result[0] ? std::stod(std::string(result[1])) : 0.;
    
    sqlite3_free_table(result);
    return rc ? -1 : 0;
}
