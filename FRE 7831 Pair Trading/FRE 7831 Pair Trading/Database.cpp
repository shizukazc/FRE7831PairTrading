//
//  Database.cpp
//  FRE 7831 Pair Trading
//
//  Created by DevonC on 4/29/21.
//

#include "Database.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

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
        + "adjusted REAL NOT NULL,"
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
        + "adjusted REAL NOT NULL,"
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
        + "FOREIGN KEY(symbol1) REFERENCES PairOnePrices(symbol) ON DELETE CASCADE ON UPDATE CASCADE,"
        + "FOREIGN KEY(date) REFERENCES PairOnePrices(date) ON DELETE CASCADE ON UPDATE CASCADE,"
        + "FOREIGN KEY(symbol2) REFERENCES PairTwoPrices(symbol) ON DELETE CASCADE ON UPDATE CASCADE,"
        + "FOREIGN KEY(date) REFERENCES PairTwoPrices(date) ON DELETE CASCADE ON UPDATE CASCADE,"
        + "FOREIGN KEY(symbol1) REFERENCES StockPairs(symbol1) ON DELETE CASCADE ON UPDATE CASCADE,"
        + "FOREIGN KEY(symbol2) REFERENCES StockPairs(symbol2) ON DELETE CASCADE ON UPDATE CASCADE);";
    
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
