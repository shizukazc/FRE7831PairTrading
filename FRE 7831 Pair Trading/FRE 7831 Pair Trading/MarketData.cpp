//
//  MarketData.cpp
//  FRE 7831 Pair Trading
//
//  Created by Zetian Chen on 4/29/21.
//

#include "MarketData.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <cstdio>
#include "json/json.h"

// Writing call back function for storing fetched values in memory
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *) userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

int ReadPairsFromFile(const char *infile, std::vector<std::pair<std::string,std::string>> &PairVec)
{
    std::ifstream fin(infile);
    
    if (!fin.is_open())
    {
        std::cerr << "ERROR: Unable to open " << infile << std::endl;
        return -1;
    }
    
    std::string line;
    while (getline(fin, line))
    {
        std::istringstream iss(line);
        std::string symbol1;
        std::string symbol2;
        
        getline(iss, symbol1, ',');
        getline(iss, symbol2, ',');
        symbol2.erase(symbol2.size() - 1);
        
        PairVec.push_back(std::pair<std::string,std::string> { symbol1, symbol2 });
    }
    
    return 0;
}

//int GetDateVec(const std::map<std::string,Stock> &StockMap, std::vector<std::string> &DateVec)
//{
//    const Stock &stock = StockMap.begin()->second;
//    const std::vector<TradeData> trades = stock.GetTrades();
//    
//    for (const TradeData &tradedata : trades)
//    {
//        DateVec.push_back(tradedata.GetDate());
//    }
//    
//    return 0;
//}

// Send GET request to EOD and store results into read_buffer
// Return 0 upon success; otherwise, -1
int PullMarketData(const std::string& url_request, std::string& read_buffer)
{
    curl_global_init(CURL_GLOBAL_ALL);
    
    CURL *handle;
    CURLcode result;
    
    handle = curl_easy_init();
    
    curl_easy_setopt(handle, CURLOPT_URL, url_request.c_str());
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0");
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &read_buffer);
    
    result = curl_easy_perform(handle);
    
    if (result != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
        return -1;
    }
    
    curl_easy_cleanup(handle);
    curl_global_cleanup();
    
    return 0;
}

int PopulateStocks(const std::string &read_buffer, std::string symbol, std::map<std::string,Stock> &StockMap)
{
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    Json::Value root;
    std::string errors;
    
    bool parsingSuccessful = reader->parse(read_buffer.c_str(), read_buffer.c_str() + read_buffer.size(), &root, &errors);
    if (!parsingSuccessful)
    {
        std::cout << "Failed to parse JSON " << std::endl << read_buffer << ": " << errors << std::endl;
        return -1;
    }
    
    std::cout << "\nSuccess parsing JSON\n" << root << std::endl;
    
    std::string date;
    double open;
    double high;
    double low;
    double close;
    double adjclose;
    long volume;
    
    Stock stock(symbol, std::vector<TradeData> {});
    
    for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
    {
        date = (*itr)["date"].asString();
        open = (*itr)["open"].asFloat();
        high = (*itr)["high"].asFloat();
        low = (*itr)["low"].asFloat();
        close = (*itr)["close"].asFloat();
        adjclose = (*itr)["adjusted_close"].asFloat();
        volume = (*itr)["volume"].asInt64();
        
        TradeData td(date, open, high, low, close, adjclose, volume);
        stock.addTrade(td);
    }
    
    StockMap.insert({ symbol, stock });
    
    return 0;
}

//int PopulateStockPairPrices(const std::vector<std::pair<std::string,std::string>> &PairVec, std::map<std::string,Stock> &StockMap, const std::vector<std::string> DateVec, std::vector<StockPairPrices> &StockPairPricesVec)
//{
//    for (const std::pair<std::string,std::string> &p : PairVec)
//    {
//        std::string symbol1 = p.first;
//        std::string symbol2 = p.second;
//        const Stock &stock1 = StockMap[symbol1];
//        const Stock &stock2 = StockMap[symbol2];
//
//        StockPairPrices spp(p);
//        std::vector<TradeData> td1 = stock1.GetTrades();
//        std::vector<TradeData> td2 = stock2.GetTrades();
//
//        for (int i = 0; i < DateVec.size(); i++)
//        {
//            PairPrice pair_price(td1[i].GetOpen(), td1[i].GetClose(), td2[i].GetOpen(), td2[i].GetClose());
//            spp.SetDailyPairPrice(DateVec[i], pair_price);
//        }
//
//        StockPairPricesVec.push_back(spp);
//    }
//
//    return 0;
//}
