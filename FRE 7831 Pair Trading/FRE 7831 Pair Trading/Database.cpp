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

const std::vector<std::string> tables {
    std::string("StockPairs"),
    std::string("PairOnePrices"),
    std::string("PairTwoPrices"),
    std::string("PairPrices")
};

int CreateDatabase(const char *dbfile)
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
        if ((ret = sqlite3_open_v2(dbfile, &pDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)))
        {
            std::cerr << "Failed to open connection: " << ret << std::endl;
            break;
        }
    } while (false);
    
    // Clean up
    if (pDb != NULL) { sqlite3_close(pDb); }
    
    sqlite3_shutdown();
    
    return ret;
}

int OpenDatabase(const char *dbfile, sqlite3 * &db)
{
    int rc = sqlite3_open(dbfile, &db);
    
    if (rc)
    {
        std::cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << std::endl << std::endl;
        // Close connection
        sqlite3_close(db);
        return -1;
    }
    
    std::cout << "Opened database " << dbfile << "." << std::endl << std::endl;
    
    return 0;
}

int CreateTables(sqlite3 * &db)
{
    char *error = NULL;
    int status = 0;
    
    std::string StockPairs_sql_create_table = std::string("CREATE TABLE IF NOT EXISTS StockPairs ")
        + "(id INT NOT NULL,"
        + "symbol1 CHAR(20) NOT NULL,"
        + "symbol2 CHAR(20) NOT NULL,"
        + "volatility FLOAT NOT NULL,"
        + "profit_loss FLOAT NOT NULL,"
        + "PRIMARY KEY(symbol1, symbol2));";
    
    std::string PairOnePrices_sql_create_table = std::string("CREATE TABLE IF NOT EXISTS PairOnePrices ")
        + "(symbol CHAR(20) NOT NULL,"
        + "date CHAR(20) NOT NULL,"
        + "open REAL NOT NULL,"
        + "high REAL NOT NULL,"
        + "low REAL NOT NULL,"
        + "close REAL NOT NULL,"
        + "adjusted_close REAL NOT NULL,"
        + "volume INT NOT NULL,"
        + "PRIMARY KEY(symbol, date));";
    
    std::string PairTwoPrices_sql_create_table = std::string("CREATE TABLE IF NOT EXISTS PairTwoPrices ")
        + "(symbol CHAR(20) NOT NULL,"
        + "date CHAR(20) NOT NULL,"
        + "open REAL NOT NULL,"
        + "high REAL NOT NULL,"
        + "low REAL NOT NULL,"
        + "close REAL NOT NULL,"
        + "adjusted_close REAL NOT NULL,"
        + "volume INT NOT NULL,"
        + "PRIMARY KEY(symbol, date));";
    
    std::string PairPrices_sql_create_table = std::string("CREATE TABLE IF NOT EXISTS PairPrices ")
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
    
    std::map<std::string,std::string> sql_create_table_map;
    sql_create_table_map.insert({ std::string("StockPairs"), StockPairs_sql_create_table });
    sql_create_table_map.insert({ std::string("PairOnePrices"), PairOnePrices_sql_create_table });
    sql_create_table_map.insert({ std::string("PairTwoPrices"), PairTwoPrices_sql_create_table });
    sql_create_table_map.insert({ std::string("PairPrices"), PairPrices_sql_create_table });
    
    // Drop the tables if they already exist in the database
    DropTables(db);
    
    // Create tables
    for (const std::pair<const std::string,std::string> &p : sql_create_table_map)
    {
        std::cout << "Creating table " << p.first << "..." << std::endl;
        if (sqlite3_exec(db, p.second.c_str(), NULL, NULL, &error))
        {
            std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
            sqlite3_free(error);
            status = -1;
        }
        else
        {
            std::cout << "Done." << std::endl << std::endl;
        }
    }
    
    return status;
}

int DropTables(sqlite3 * &db)
{
    char *error;
    
    for (std::vector<std::string>::const_reverse_iterator rit = tables.rbegin(); rit != tables.rend(); rit++)
    {
        const std::string &table = *rit;
        std::string sql_droptable = std::string("DROP TABLE IF EXISTS ") + table + ";";

        if (sqlite3_exec(db, sql_droptable.c_str(), NULL, NULL, &error) != SQLITE_OK)
        {
            std::cerr << "SQLite3 failed to drop " << table << " table." << std::endl;
            sqlite3_free(error);
            return -1;
        }
    }
    
    return 0;
}

void CloseDatabase(sqlite3 * &db)
{
    std::cout << "Closing database..." << std::endl;
    
    sqlite3_close(db);
    
    std::cout << "Done." << std::endl << std::endl;
}

int InsertPairKPrices(sqlite3 * &db, bool IsPairOne, const Stock &stock)
{
    int rc = 0;
    char *error;
    std::string date;
    double open, high, low, close, adjclose;
    long volume;
    
    const std::string table = IsPairOne ? "PairOnePrices" : "PairTwoPrices";
    const std::string &symbol = stock.GetSymbol();
    std::string sql_command;
    
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
    
    int count = 0;
    for (const TradeData &td : stock.GetTrades())
    {
        date = td.GetDate();
        open = td.GetOpen();
        high = td.GetHigh();
        low = td.GetLow();
        close = td.GetClose();
        adjclose = td.GetAdjClose();
        volume = td.GetVolume();
        
        std::ostringstream ss;
        ss << "INSERT INTO " << table << "(symbol, date, open, high, low, close, adjusted_close, volume) VALUES('"
            << symbol.c_str() << "','" << date.c_str() << "'," << open << "," << high << ","
            << low << "," << close << "," << adjclose << "," << volume << ");";
        sql_command = ss.str();
        
        rc = sqlite3_exec(db, sql_command.c_str(), NULL, NULL, &error);
        if (rc)
        {
            std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
            sqlite3_free(error);
        }
        else
        {
            count++;
        }
    }
    
    sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
    
    std::cout << "Inserted " << count << " entries for stock " << symbol << " into table " << table << std::endl << std::endl;
    
    return 0;
}

int InsertStockPairs(sqlite3 * &db, const std::vector<std::pair<std::string,std::string>> &PairVec)
{
    int rc = 0;
    char *error;
    std::string sql_command;
    
    int id = 0;
    
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
    
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
            std::cout << "Inserted (" << symbol1 << "," << symbol2 << ") into table StockPairs." << std::endl;
        }
    }
    
    std::cout << std::endl;
    
    sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
    
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
            "PairOnePrices.adjusted_close AS close1, "  // Use adjusted_close for historical data
            "PairTwoPrices.open AS open2, "
            "PairTwoPrices.adjusted_close AS close2, "  // Use adjusted_close for historical data
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
        volatility = std::stod(std::string(result[1]));
    }
    
    sqlite3_free_table(result);
    return rc ? -1 : 0;
}

int GetPairPriceMap(sqlite3 * &db, std::string sdate, std::string edate, StockPairPrices &spp)
{
    int rc = 0;
    char *error;
    
    char **result;
    int row;
    int col;
    
    const std::string &symbol1 = spp.GetStockPair().first;
    const std::string &symbol2 = spp.GetStockPair().second;
    
    std::string sql_command = std::string("SELECT ")
            + "StockPairs.symbol1 AS symbol1, "
            + "StockPairs.symbol2 AS symbol2, "
            + "PairOnePrices.date AS date, "
            + "PairOnePrices.open AS open1, "
            + "PairOnePrices.close AS close1, "
            + "PairTwoPrices.open AS open2, "
            + "PairTwoPrices.close AS close2 "
        + "FROM StockPairs, PairOnePrices, PairTwoPrices "
        + "WHERE (((PairOnePrices.symbol = StockPairs.symbol1) "
            + "AND (PairTwoPrices.symbol = StockPairs.symbol2)) "
            + "AND (PairOnePrices.date = PairTwoPrices.date)) "
            + "AND PairOnePrices.date >= '" + sdate + "' "
            + "AND PairOnePrices.date <= '" + edate + "' "
            + "AND StockPairs.symbol1 = '" + symbol1 + "' "
            + "AND StockPairs.symbol2 = '" + symbol2 + "' "
        + "ORDER BY symbol1, symbol2, date;";
    
    rc = sqlite3_get_table(db, sql_command.c_str(), &result, &row, &col, &error);
    if (rc)
    {
        std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
        sqlite3_free(error);
    }
    else
    {
        std::cout << "Done retrieving PairPrice-s from " << sdate << " to " << edate + "." << std::endl << std::endl;
        
        for (int r = 0; r < row; r++)  // Skip the header row
        {
            int base = (r + 1) * col;
            std::string symbol1 = std::string(result[base + 0]);
            std::string symbol2 = std::string(result[base + 1]);
            std::string date = std::string(result[base + 2]);
            double open1 = std::stod(std::string(result[base + 3]));
            double close1 = std::stod(std::string(result[base + 4]));
            double open2 = std::stod(std::string(result[base + 5]));
            double close2 = std::stod(std::string(result[base + 6]));
            
            PairPrice pp(open1, close1, open2, close2);
            spp.SetDailyPairPrice(date, pp);
        }
    }
    
    sqlite3_free_table(result);
    
    return rc ? -1 : 0;
}

int UpdateBacktestPnL(sqlite3 * &db, std::vector<StockPairPrices> &StockPairPricesVec)
{
    int rc, status = 0;
    char *error;
    std::string date;
    std::string symbol1;
    std::string symbol2;
    int id = 1;
    double profit_loss;
    
    std::string sql_command;
    
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
    
    for (const StockPairPrices &spp : StockPairPricesVec)
    {
        const std::map<std::string,PairPrice> &dailyPairPrices = spp.GetDailyPrices();
        symbol1 = spp.GetStockPair().first;
        symbol2 = spp.GetStockPair().second;
        
        int count = 0;
        double tot = 0.;
        
        for (const std::pair<const std::string,PairPrice> &dpp : dailyPairPrices)
        {
            date = dpp.first;
            profit_loss = dpp.second.dProfitLoss;
            tot += profit_loss;
            
            sql_command = std::string("UPDATE PairPrices SET profit_loss = ")
                + std::to_string(profit_loss)
                + " WHERE PairPrices.symbol1 = '" + symbol1
                + "' AND PairPrices.symbol2 = '" + symbol2
                + "' AND PairPRices.date = '" + date + "';";
            
            rc = sqlite3_exec(db, sql_command.c_str(), NULL, NULL, &error);
            if (rc)
            {
                status = -1;
                std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
                sqlite3_free(error);
            }
            {
                count++;
            }
        }
        
        std::cout << "Updated " << count << " entries for StockPair(" << symbol1 << "," << symbol2 << ")'s P&L in PairPrices. " << std::endl;
        
        sql_command = std::string("UPDATE StockPairs SET profit_loss = ")
            + std::to_string(tot) + " WHERE StockPairs.id = " + std::to_string(id) + ";";
        
        rc = sqlite3_exec(db, sql_command.c_str(), NULL, NULL, &error);
        if (rc)
        {
            status = -1;
            std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl << std::endl;
            sqlite3_free(error);
        }
        else
        {
            std::cout << "Updated P&L for StockPair(" << symbol1 << "," << symbol2 << ") in StockPairs."  << std::endl;
        }

        id += 1;
    }
    
    sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
    
    return status ? -1 : 0;
}
