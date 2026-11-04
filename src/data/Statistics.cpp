#include "Statistics.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace {
void ensureNotEmpty(const std::vector<double>& data) {
    if (data.empty()) {
        throw std::invalid_argument("Statistics cannot be calculated for empty data.");
    }
}
}

// The functions here calculate values used by the different graphs.
std::size_t Statistics::count(const std::vector<double>& data) {
    return data.size();
}

double Statistics::mean(const std::vector<double>& data) {
    ensureNotEmpty(data);
    double total = std::accumulate(data.begin(), data.end(), 0.0);
    return total / static_cast<double>(data.size());
}

double Statistics::minimum(const std::vector<double>& data) {
    ensureNotEmpty(data);
    return *std::min_element(data.begin(), data.end());
}

double Statistics::maximum(const std::vector<double>& data) {
    ensureNotEmpty(data);
    return *std::max_element(data.begin(), data.end());
}

double Statistics::median(std::vector<double> data) {
    ensureNotEmpty(data);
    // Sorting a copy means we do not change the caller's original values.
    std::sort(data.begin(), data.end());

    std::size_t middle = data.size() / 2;

    if (data.size() % 2 == 0) {
        return (data[middle - 1] + data[middle]) / 2.0;
    }

    return data[middle];
}

double Statistics::variance(const std::vector<double>& data) {
    ensureNotEmpty(data);

    double avg = mean(data);
    double totalSquaredDifference = 0.0;

    for (double value : data) {
        double difference = value - avg;
        totalSquaredDifference += difference * difference;
    }

    return totalSquaredDifference / static_cast<double>(data.size());
}

double Statistics::standardDeviation(const std::vector<double>& data) {
    return std::sqrt(variance(data));
}

namespace {
double percentile(std::vector<double> data,double p) {
    ensureNotEmpty(data);
    std::sort(data.begin(),data.end());
    double position=(data.size()-1)*p;
    std::size_t left=static_cast<std::size_t>(position);
    std::size_t right=std::min(data.size()-1,left+1);
    return data[left]+(position-left)*(data[right]-data[left]);
}
}
double Statistics::firstQuartile(std::vector<double> data) {return percentile(std::move(data),0.25);}
double Statistics::thirdQuartile(std::vector<double> data) {return percentile(std::move(data),0.75);}
double Statistics::interquartileRange(std::vector<double> data) {
    return thirdQuartile(data)-firstQuartile(data);
}
std::vector<double> Statistics::mode(std::vector<double> data) {
    ensureNotEmpty(data);std::sort(data.begin(),data.end());
    std::vector<double> result;std::size_t best=0;
    for(std::size_t i=0;i<data.size();) {
        std::size_t j=i+1;
        while(j<data.size()&&data[j]==data[i])++j;
        if(j-i>best){best=j-i;result.clear();}
        if(j-i==best)result.push_back(data[i]);
        i=j;
    }
    return result;
}
