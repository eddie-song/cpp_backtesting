#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>

struct StockData {
    std::string date;
    double price;
    double close;
    double high;
    double low;
    double open;
    int volume;
};

class Backtester {
private:
    std::vector<StockData> historicalData;
    std::vector<double> shortMA;
    std::vector<double> longMA;
    int shortPeriod;
    int longPeriod;

    double safeStod(const std::string& str) {
        try {
            std::string trimmed = str;
            trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), ::isspace), trimmed.end());
            if (trimmed.empty()) return 0.0;
            return std::stod(trimmed);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error converting value: '" << str << "'" << std::endl;
            return 0.0;
        }
    }

    int safeStoi(const std::string& str) {
        try {
            std::string trimmed = str;
            trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), ::isspace), trimmed.end());
            if (trimmed.empty()) return 0;
            return std::stoi(trimmed);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error converting value: '" << str << "'" << std::endl;
            return 0;
        }
    }

    double calculateMA(const std::vector<double>& data, int period, int currentIndex) {
        if (currentIndex < period - 1) return 0.0;
        
        double sum = 0.0;
        for (int i = 0; i < period; i++) {
            sum += data[currentIndex - i];
        }
        return sum / period;
    }

    void calculateMovingAverages() {
        shortMA.clear();
        longMA.clear();
        std::vector<double> prices;
        
        for (const auto& data : historicalData) {
            prices.push_back(data.close);
        }

        for (size_t i = 0; i < prices.size(); i++) {
            shortMA.push_back(calculateMA(prices, shortPeriod, i));
            longMA.push_back(calculateMA(prices, longPeriod, i));
        }
    }

public:
    Backtester(int shortP = 5, int longP = 20) 
        : shortPeriod(shortP), longPeriod(longP) {}

    bool loadData(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return false;
        }

        std::string line;
        for (int i = 0; i < 3; i++) {
            std::getline(file, line);
        }

        std::cout << "Reading data from CSV file..." << std::endl;

        int lineNumber = 4;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string value;
            StockData data;

            try {
                std::vector<std::string> values;
                while (std::getline(ss, value, ',')) {
                    values.push_back(value);
                }

                if (values.size() < 6) {
                    std::cerr << "Line " << lineNumber << " has insufficient columns. Skipping." << std::endl;
                    continue;
                }

                data.date = values[0];
                data.price = safeStod(values[0]);
                data.close = safeStod(values[1]);
                data.high = safeStod(values[2]);
                data.low = safeStod(values[3]);
                data.open = safeStod(values[4]);
                data.volume = safeStoi(values[5]);

                if (data.close > 0) {
                    historicalData.push_back(data);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing line " << lineNumber << ": " << e.what() << std::endl;
                std::cerr << "Line content: " << line << std::endl;
            }
            lineNumber++;
        }

        std::cout << "Successfully loaded " << historicalData.size() << " data points." << std::endl;
        return !historicalData.empty();
    }

    void runBacktest() {
        if (historicalData.empty()) {
            std::cout << "No data loaded. Please load data first." << std::endl;
            return;
        }

        calculateMovingAverages();
        
        double initialBalance = 10000.0;
        double balance = initialBalance;
        int position = 0;
        std::vector<double> portfolioValue;
        std::vector<double> returns;
        int trades = 0;

        double firstPrice = historicalData[0].close;
        double lastPrice = historicalData.back().close;
        double buyAndHoldReturn = ((lastPrice - firstPrice) / firstPrice) * 100.0;
        double buyAndHoldFinalBalance = initialBalance * (lastPrice / firstPrice);

        for (size_t i = longPeriod; i < historicalData.size(); i++) {
            if (shortMA[i] > longMA[i] && position == 0) {
                position = 1;
                trades++;
                std::cout << historicalData[i].date << ": BUY at " << std::fixed << std::setprecision(2) 
                         << historicalData[i].close << std::endl;
            }
            else if (shortMA[i] < longMA[i] && position == 1) {
                position = 0;
                trades++;
                std::cout << historicalData[i].date << ": SELL at " << std::fixed << std::setprecision(2) 
                         << historicalData[i].close << std::endl;
            }

            double currentValue = balance;
            if (position == 1) {
                currentValue = balance * (historicalData[i].close / historicalData[i-1].close);
            }
            portfolioValue.push_back(currentValue);
            balance = currentValue;

            if (i > longPeriod) {
                double dailyReturn = (currentValue / portfolioValue[portfolioValue.size() - 2]) - 1;
                returns.push_back(dailyReturn);
            }
        }

        double totalReturn = ((balance - initialBalance) / initialBalance) * 100.0;
        double avgReturn = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
        double volatility = 0.0;
        for (double r : returns) {
            volatility += (r - avgReturn) * (r - avgReturn);
        }
        volatility = std::sqrt(volatility / returns.size()) * std::sqrt(252);
        double sharpeRatio = (avgReturn * 252) / volatility;

        std::cout << "\nBacktest Results:" << std::endl;
        std::cout << "--------------------------------" << std::endl;
        std::cout << "Initial Balance: $" << std::fixed << std::setprecision(2) << initialBalance << std::endl;
        std::cout << "Final Balance: $" << std::fixed << std::setprecision(2) << balance << std::endl;
        std::cout << "Total Return: " << std::fixed << std::setprecision(2) << totalReturn << "%" << std::endl;
        std::cout << "Number of Trades: " << trades << std::endl;
        std::cout << "Average Daily Return: " << std::fixed << std::setprecision(4) << (avgReturn * 100) << "%" << std::endl;
        std::cout << "Annualized Volatility: " << std::fixed << std::setprecision(2) << (volatility * 100) << "%" << std::endl;
        std::cout << "Sharpe Ratio: " << std::fixed << std::setprecision(2) << sharpeRatio << std::endl;
        
        std::cout << "\nBuy and Hold Comparison:" << std::endl;
        std::cout << "--------------------------------" << std::endl;
        std::cout << "Buy and Hold Final Balance: $" << std::fixed << std::setprecision(2) << buyAndHoldFinalBalance << std::endl;
        std::cout << "Buy and Hold Return: " << std::fixed << std::setprecision(2) << buyAndHoldReturn << "%" << std::endl;
        std::cout << "Strategy vs Buy and Hold: " << std::fixed << std::setprecision(2) 
                 << (totalReturn - buyAndHoldReturn) << "%" << std::endl;
    }
};

int main() {
    Backtester backtester(5, 20);
    
    if (backtester.loadData("data/nvda_stock_data.csv")) {
        backtester.runBacktest();
    }
    
    return 0;
} 