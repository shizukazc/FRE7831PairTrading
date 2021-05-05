//
//  PairTrading.hpp
//  FRE 7831 Pair Trading
//
//  Created by Zetian Chen on 4/29/21.
//

#ifndef PairTrading_hpp
#define PairTrading_hpp

#include <string>
#include <ostream>
#include <vector>
#include <utility>
#include <map>

class TradeData
{
private:
    std::string sDate;
    double dOpen;
    double dHigh;
    double dLow;
    double dClose;
    double dAdjClose;
    long lVolume;
    
public:
    // Default constructor
    TradeData(): sDate(""), dOpen(0), dHigh(0), dLow(0), dClose(0), dAdjClose(0), lVolume(0) {}
    
    // Parameterized constructor
    TradeData(std::string sDate_, double dOpen_, double dHigh_, double dLow_, double dClose_, double dAdjClose_, long lVolume_)
    {
        this->sDate = sDate_;
        this->dOpen = dOpen_;
        this->dHigh = dHigh_;
        this->dLow = dLow_;
        this->dClose = dClose_;
        this->dAdjClose = dAdjClose_;
        this->lVolume = lVolume_;
    }
    
    // Copy constructor
    TradeData(const TradeData &other)
    {
        this->sDate = other.sDate;
        this->dOpen = other.dOpen;
        this->dHigh = other.dHigh;
        this->dLow = other.dLow;
        this->dClose = other.dClose;
        this->dAdjClose = other.dAdjClose;
        this->lVolume = other.lVolume;
    }
    
    // Overload assignment operator
    TradeData operator=(const TradeData &other)
    {
        this->sDate = other.sDate;
        this->dOpen = other.dOpen;
        this->dHigh = other.dHigh;
        this->dLow = other.dLow;
        this->dClose = other.dClose;
        this->dAdjClose = other.dAdjClose;
        this->lVolume = other.lVolume;
        return *this;
    }
    
    // Getter
    std::string GetDate() const { return sDate; }
    double GetOpen() const { return dOpen; }
    double GetHigh() const { return dHigh; }
    double GetLow() const { return dLow; }
    double GetClose() const { return dClose; }
    double GetAdjClose() const { return dAdjClose; }
    long GetVolume() const { return lVolume; }
    
    // Friend
    friend std::ostream & operator<<(std::ostream &out, const TradeData &tradedata);
};

class Stock
{
private:
    std::string sSymbol;
    std::vector<TradeData> trades;
    
public:
    // Default constructor
    Stock(): sSymbol("") {}
    
    // Parameterized constructor
    Stock(std::string sSymbol_, const std::vector<TradeData> trades_)
    {
        this->sSymbol = sSymbol_;
        this->trades = trades_;
    }
    
    // Copy constructor
    Stock(const Stock &other)
    {
        this->sSymbol = other.sSymbol;
        this->trades = other.trades;
    }
    
    // Overload assignment operator
    Stock operator=(const Stock &other)
    {
        this->sSymbol = other.sSymbol;
        this->trades = other.trades;
        return *this;
    }
    
    // Getter
    std::string GetSymbol() const { return sSymbol; }
    
    const std::vector<TradeData> & GetTrades() const { return trades; }
    
    // Function
    void addTrade(const TradeData &tradedata)
    {
        trades.push_back(tradedata);
    }
    
    // Friend
    friend std::ostream & operator<<(std::ostream &out, const Stock &stock);
};

struct PairPrice
{
    double dOpen1;
    double dClose1;
    double dOpen2;
    double dClose2;
    double dProfitLoss;
    
    // Default constructor
    PairPrice(): dOpen1(0), dClose1(0), dOpen2(0), dClose2(0), dProfitLoss(0) {}
    
    // Parameterized constructor
    PairPrice(double dOpen1_, double dClose1_, double dOpen2_, double dClose2_)
    {
        this->dOpen1 = dOpen1_;
        this->dClose1 = dClose1_;
        this->dOpen2 = dOpen2_;
        this->dClose2 = dClose2_;
        this->dProfitLoss = 0.;
    }
    
    friend std::ostream & operator<<(std::ostream &out, const PairPrice &pp);
};

class StockPairPrices
{
private:
    std::pair<std::string,std::string> stockPair;
    double volatility;
    double k;
    std::map<std::string,PairPrice> dailyPairPrices;
    
public:
    // Default constructor
    StockPairPrices(): volatility(0), k(0) {}
    
    // Parameterized constructor
    StockPairPrices(std::pair<std::string,std::string> stockPair_)
    {
        this->stockPair = stockPair_;
        this->volatility = 0;
        this->k = 0;
    }
    
    // Getter
    const std::pair<std::string,std::string> & GetStockPair() const { return stockPair; }
    const std::map<std::string,PairPrice> & GetDailyPrices() const { return dailyPairPrices; }
    std::map<std::string,PairPrice> & GetDailyPrices() { return dailyPairPrices; }
    double GetVolatility() const { return volatility; }
    double GetK() const { return k; }
    
    // Calculate the total profit and loss
    double GetTotalPnL() const;
    
    // Setter
    void SetVolatility(double volatility_) { this->volatility = volatility_; }
    void SetK(double k_) { this->k = k_; }
    
    void SetDailyPairPrice(std::string sDate_, PairPrice pairPrice_)
    {
        dailyPairPrices.insert(std::pair<std::string,PairPrice>(sDate_, pairPrice_));
    }
    
    void UpdateProfitLoss(std::string sDate_, double dProfitLoss_)
    {
        dailyPairPrices[sDate_].dProfitLoss = dProfitLoss_;
    }
    
    friend std::ostream & operator<<(std::ostream &out, const StockPairPrices &spp);
};

#endif /* PairTrading_hpp */
