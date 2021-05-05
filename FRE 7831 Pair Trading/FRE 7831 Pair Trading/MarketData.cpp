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
#include <cmath>
#include "json/json.h"

static const std::string API_TOKEN = "605f41c6a29c69.96773141";

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
        
        if (!symbol2.empty() && symbol2[symbol2.length() - 1] == '\r') symbol2.erase(symbol2.size() - 1);
        
        PairVec.push_back(std::pair<std::string,std::string> { symbol1, symbol2 });
    }
    
    return 0;
}

// Send GET request to EOD and store results into read_buffer
// Return 0 upon success; otherwise, -1
int PullMarketData(const std::string &url_request, std::string &read_buffer)
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

//int PopulateStocks(const std::string &read_buffer, std::string symbol, std::map<std::string,Stock> &StockMap)
//{
//    std::vector<TradeData> TradeDataVec;
//    std::vector<TradeData> BacktestVec;
//
//    if (ParseJson(read_buffer, bt_sdate, TradeDataVec, BacktestVec) != 0)
//    {
//        std::cerr << "PopulateStocks() failed" << std::endl;
//        return -1;
//    }
//
//    Stock stock(symbol, BacktestVec);
//    StockMap.insert({ symbol, stock });
//
//    return 0;
//}

std::string BuildDailyRequestURL(std::string symbol, std::string start_date, std::string end_date)
{
    std::string url_common_daily = "https://eodhistoricaldata.com/api/eod/";
    std::string url_request_daily = url_common_daily + symbol + ".US?from="
        + start_date + "&to=" + end_date + "&api_token=" + API_TOKEN + "&period=d&fmt=json";
    return url_request_daily;
}

// Backtest

int ParseJson(const std::string &read_buffer, std::vector<TradeData> &TradeDataVec)
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
    
    std::cout << "\nSuccess parsing JSON\n" << std::endl;
    
    std::string date;
    double open;
    double high;
    double low;
    double close;
    double adjclose;
    long volume;
    
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
        TradeDataVec.push_back(td);
    }
    
    return 0;
}

//int PopulateStockPairPrices(const std::vector<std::pair<std::string,std::string>> &PairVec,
//                            std::string start_date, std::string end_date, std::vector<StockPairPrices> &StockPairPricesVec)
//{
//    for (const std::pair<std::string,std::string> &p : PairVec)
//    {
//        StockPairPrices spp(p);
//
//        const std::string &symbol1 = p.first;
//        const std::string &symbol2 = p.second;
//
//        std::string read_buffer1;
//        std::string read_buffer2;
//
//        std::string url_request1 = BuildDailyRequestURL(symbol1, start_date, end_date);
//        std::string url_request2 = BuildDailyRequestURL(symbol2, start_date, end_date);
//
//        if (PullMarketData(url_request1, read_buffer1) != 0)
//        {
//            std::cerr << "Backtest: Failed to pull market data for " << symbol1 << std::endl;
//            return -1;
//        }
//
//        if (PullMarketData(url_request2, read_buffer2) != 0)
//        {
//            std::cerr << "Backtest: Failed to pull market data for " << symbol2 << std::endl;
//            return -1;
//        }
//
//        std::vector<TradeData> TradeDataVec1;
//        std::vector<TradeData> TradeDataVec2;
//
//        if (ParseJson(read_buffer1, TradeDataVec1) != 0)
//        {
//            std::cerr << "Backtest: Failed to parse json for " << symbol1 << std::endl;
//            return -1;
//        }
//
//        if (ParseJson(read_buffer2, TradeDataVec2) != 0)
//        {
//            std::cerr << "Backtest: Failed to parse json for " << symbol2 << std::endl;
//            return -1;
//        }
//
//        double open1 = 0.;
//        double open2 = 0.;
//        double close1 = 0.;
//        double close2 = 0.;
//
//        for (int i = 0; i < TradeDataVec1.size(); i++)
//        {
//            TradeData &td1 = TradeDataVec1[i];
//            TradeData &td2 = TradeDataVec2[i];
//
//            open1 = td1.GetOpen();
//            open2 = td2.GetOpen();
//            close1 = td1.GetClose();
//            close2 = td2.GetClose();
//
//            PairPrice pp(open1, close1, open2, close2);
//            spp.SetDailyPairPrice(td1.GetDate(), pp);
//        }
//
//        StockPairPricesVec.push_back(spp);
//    }
//
//    return 0;
//}

void PairTradePerform(std::vector<StockPairPrices> &StockPairPricesVec)
{
    double close1d1 = 0.;
    double close2d1 = 0.;
    double open1d2 = 0.;
    double open2d2 = 0.;
    double close1d2 = 0.;
    double close2d2 = 0.;
    
    std::string first_date;
    
    for (StockPairPrices &spp : StockPairPricesVec)
    {
        std::map<std::string,PairPrice> &dailyPairPrices = spp.GetDailyPrices();
        
        std::vector<std::string> DatesVec;
        for (const std::pair<const std::string,PairPrice> &dp : dailyPairPrices) { DatesVec.push_back(dp.first); }
        std::sort(DatesVec.begin(), DatesVec.end());
        
        for (std::vector<std::string>::const_iterator itr = DatesVec.begin(); itr != DatesVec.end(); itr++)
        {
            const PairPrice &pp = dailyPairPrices[*itr];
            
            if (itr == DatesVec.begin())
            {
                // First date
                first_date = *itr;
                close1d1 = pp.dClose1;
                close2d1 = pp.dClose2;
                continue;
            }
            
            open1d2 = pp.dOpen1;
            open2d2 = pp.dOpen2;
            close1d2 = pp.dClose1;
            close2d2 = pp.dClose2;
            
            double close_ratio = close1d1 / close2d1;
            double open_ratio = open1d2 / open2d2;
            
            int N1 = ((abs(close_ratio - open_ratio) > spp.GetK() * spp.GetVolatility()) ? -1 : 1) * 10000;
            int N2 = (int) std::round(-N1 * open_ratio);
            
            double pnl = N1 * (close1d2 - open1d2) + N2 * (close2d2 - open2d2);
            
            spp.UpdateProfitLoss(*itr, pnl);
            
            // Update
            close1d1 = close1d2;
            close2d1 = close2d2;
        }
        
        // Remove the first record because it does not have P&L
        dailyPairPrices.erase(first_date);
    }
}
