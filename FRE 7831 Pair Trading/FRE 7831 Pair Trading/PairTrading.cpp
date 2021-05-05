//
//  PairTrading.cpp
//  FRE 7831 Pair Trading
//
//  Created by Zetian Chen on 4/29/21.
//

#include "PairTrading.hpp"
#include <iomanip>

std::ostream & operator<<(std::ostream &out, const TradeData &tradedata)
{
    out << "    sDate = " << tradedata.GetDate() << std::endl;
    out << "    dOpen = " << tradedata.GetOpen() << std::endl;
    out << "    dHigh = " << tradedata.GetHigh() << std::endl;
    out << "     dLow = " << tradedata.GetLow() << std::endl;
    out << "   dClose = " << tradedata.GetClose() << std::endl;
    out << "dAdjClose = " << tradedata.GetAdjClose() << std::endl;
    return out;
}

std::ostream & operator<<(std::ostream &out, const Stock &stock)
{
    out << "Symbol: " << stock.GetSymbol() << std::endl;
    for (const TradeData &tradedata : stock.GetTrades()) { out << tradedata; }
    return out;
}

std::ostream & operator<<(std::ostream &out, const PairPrice &pp)
{
    out << "open1=" << pp.dOpen1 << ", close1=" << pp.dClose1
        << ", open2=" << pp.dOpen2 << ", close2=" << pp.dClose2
        << ", P&L=" << pp.dProfitLoss;
    return out;
}

double StockPairPrices::GetTotalPnL() const
{
    double tot = 0.;
    for (const std::pair<const std::string,PairPrice> &dp : dailyPairPrices)
    {
        tot += dp.second.dProfitLoss;
    }
    
    return tot;
}

std::ostream & operator<<(std::ostream &out, const StockPairPrices &spp)
{
    out << std::setiosflags(std::ios::fixed) << std::setprecision(2);
    out << "(" << spp.GetStockPair().first << "," << spp.GetStockPair().second;
    
    out << "), vol=" << std::setprecision(20) << spp.GetVolatility();
    out << ", K=" << std::setprecision(2) << spp.GetK()
        << ", Total PnL=" << spp.GetTotalPnL() << std::endl;
    
    for (const std::pair<const std::string,PairPrice> &dp : spp.GetDailyPrices())
    {
        out << "Date=" << dp.first << ", " << dp.second << std::endl;
    }
    
    return out;
}
