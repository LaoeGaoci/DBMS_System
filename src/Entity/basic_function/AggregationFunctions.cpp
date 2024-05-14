//
// Created by ERQ on 2024/5/14.
//

#include "Entity/basic_function/AggregationFunctions.h"
// ��������
int AggregationFunctions::count(const std::vector<std::string>& data,bool have_all) {
    if(have_all){
        return data.size();
    }else{
        size_t count = 0;
        for (const auto& value : data) {
            if (!value.empty()) {
                ++count;
            }
        }
        return count;
    }
}
// ƽ��ֵ����
double AggregationFunctions::average(const std::vector<std::string>& data) {
    double sum = 0;
    int count = 0;
    for (const auto& value : data) {
        if (!value.empty()) {
            sum += std::stoi(value);
            count++;
        }
    }
    return count == 0 ? 0 : sum / count;
}

// ��ͺ���
int AggregationFunctions::sum(const std::vector<std::string>& data) {
    int sum = 0;
    for (const auto& value : data) {
        if (!value.empty()) {
            sum += std::stoi(value);
        }
    }
    return sum;
}

// ���ֵ����
int AggregationFunctions::max(const std::vector<std::string>& data) {
    int maxValue = std::numeric_limits<int>::min();
    for (const auto& value : data) {
        if (!value.empty()) {
            int intValue = std::stoi(value);
            if (intValue > maxValue) {
                maxValue = intValue;
            }
        }
    }
    return maxValue;
}

// ��Сֵ����
int AggregationFunctions::min(const std::vector<std::string>& data) {
    int minValue = std::numeric_limits<int>::max();
    for (const auto& value : data) {
        if (!value.empty()) {
            int intValue = std::stoi(value);
            if (intValue < minValue) {
                minValue = intValue;
            }
        }
    }
    return minValue;
}
