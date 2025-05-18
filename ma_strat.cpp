#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <algorithm>
#include <numeric>

class MovingAverageStrategy {
private:
    std::vector<double> prices;
    std::vector<double> shortMA;
    std::vector<double> longMA;
    int shortPeriod;
    int longPeriod;

    double calculateMA(const std::vector<double>& data, int period, int currentIndex) {
        if (currentIndex < period - 1) return 0.0;
        
        double sum = 0.0;
        for (int i = 0; i < period; i++) {
            sum += data[currentIndex - i];
        }
        return sum / period;
    }

    double runSingleSimulation() {
        generatePriceData(252);
        calculateMovingAverages();
        
        double balance = 10000.0;
        int position = 0;

        for (size_t i = longPeriod; i < prices.size(); i++) {
            if (shortMA[i] > longMA[i] && position == 0) {
                position = 1;
            }
            else if (shortMA[i] < longMA[i] && position == 1) {
                position = 0;
            }

            if (position == 1) {
                balance = balance * (prices[i] / prices[i-1]);
            }
        }

        return ((balance - 10000.0) / 10000.0 * 100.0);
    }

public:
    MovingAverageStrategy(int shortP = 5, int longP = 20) 
        : shortPeriod(shortP), longPeriod(longP) {}

    void generatePriceData(int numDays, double initialPrice = 100.0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> d(0.0, 1.0);

        prices.clear();
        prices.push_back(initialPrice);

        for (int i = 1; i < numDays; i++) {
            double change = d(gen) * 2.0;
            prices.push_back(prices.back() * (1.0 + change/100.0));
        }
    }

    void calculateMovingAverages() {
        shortMA.clear();
        longMA.clear();

        for (size_t i = 0; i < prices.size(); i++) {
            shortMA.push_back(calculateMA(prices, shortPeriod, i));
            longMA.push_back(calculateMA(prices, longPeriod, i));
        }
    }

    void simulateStrategy() {
        double balance = 10000.0;
        int position = 0;
        std::vector<double> portfolioValue;

        for (size_t i = longPeriod; i < prices.size(); i++) {
            if (shortMA[i] > longMA[i] && position == 0) {
                position = 1;
                std::cout << "Day " << i << ": BUY at " << std::fixed << std::setprecision(2) 
                         << prices[i] << std::endl;
            }
            else if (shortMA[i] < longMA[i] && position == 1) {
                position = 0;
                std::cout << "Day " << i << ": SELL at " << std::fixed << std::setprecision(2) 
                         << prices[i] << std::endl;
            }

            double currentValue = balance;
            if (position == 1) {
                currentValue = balance * (prices[i] / prices[i-1]);
            }
            portfolioValue.push_back(currentValue);
            balance = currentValue;
        }

        std::cout << "\nFinal Results:" << std::endl;
        std::cout << "Initial Balance: $" << std::fixed << std::setprecision(2) << 10000.0 << std::endl;
        std::cout << "Final Balance: $" << std::fixed << std::setprecision(2) << balance << std::endl;
        std::cout << "Return: " << std::fixed << std::setprecision(2) 
                 << ((balance - 10000.0) / 10000.0 * 100.0) << "%" << std::endl;
    }

    void printData() {
        std::cout << "\nPrice Data and Moving Averages:" << std::endl;
        std::cout << std::setw(10) << "Day" << std::setw(15) << "Price" 
                 << std::setw(15) << "Short MA" << std::setw(15) << "Long MA" << std::endl;
        
        for (size_t i = 0; i < prices.size(); i++) {
            std::cout << std::setw(10) << i 
                     << std::setw(15) << std::fixed << std::setprecision(2) << prices[i];
            
            if (i >= shortPeriod - 1) {
                std::cout << std::setw(15) << std::fixed << std::setprecision(2) << shortMA[i];
            } else {
                std::cout << std::setw(15) << "N/A";
            }
            
            if (i >= longPeriod - 1) {
                std::cout << std::setw(15) << std::fixed << std::setprecision(2) << longMA[i];
            } else {
                std::cout << std::setw(15) << "N/A";
            }
            std::cout << std::endl;
        }
    }

    void runMonteCarloSimulation(int numSimulations = 100) {
        std::vector<double> returns;
        returns.reserve(numSimulations);

        std::cout << "\nRunning Monte Carlo Simulation (" << numSimulations << " simulations)..." << std::endl;
        
        for (int i = 0; i < numSimulations; i++) {
            double return_pct = runSingleSimulation();
            returns.push_back(return_pct);
            
            if ((i + 1) % 10 == 0) {
                std::cout << "Completed " << (i + 1) << " simulations..." << std::endl;
            }
        }

        std::sort(returns.begin(), returns.end());
        double avg_return = std::accumulate(returns.begin(), returns.end(), 0.0) / numSimulations;
        double median_return = returns[numSimulations / 2];
        double min_return = returns.front();
        double max_return = returns.back();
        
        int percentile_95_index = static_cast<int>(numSimulations * 0.95);
        int percentile_5_index = static_cast<int>(numSimulations * 0.05);
        double percentile_95 = returns[percentile_95_index];
        double percentile_5 = returns[percentile_5_index];

        std::cout << "\nMonte Carlo Simulation Results:" << std::endl;
        std::cout << "--------------------------------" << std::endl;
        std::cout << "Number of Simulations: " << numSimulations << std::endl;
        std::cout << "Average Return: " << std::fixed << std::setprecision(2) << avg_return << "%" << std::endl;
        std::cout << "Median Return: " << std::fixed << std::setprecision(2) << median_return << "%" << std::endl;
        std::cout << "Best Return: " << std::fixed << std::setprecision(2) << max_return << "%" << std::endl;
        std::cout << "Worst Return: " << std::fixed << std::setprecision(2) << min_return << "%" << std::endl;
        std::cout << "95th Percentile: " << std::fixed << std::setprecision(2) << percentile_95 << "%" << std::endl;
        std::cout << "5th Percentile: " << std::fixed << std::setprecision(2) << percentile_5 << "%" << std::endl;
    }
};

int main() {
    MovingAverageStrategy strategy(5, 20);
    
    strategy.runMonteCarloSimulation(100);
    
    return 0;
} 