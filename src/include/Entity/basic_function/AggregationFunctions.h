//
// Created by ERQ on 2024/5/14.
//

#ifndef DBMS_AGGREGATIONFUNCTIONS_H
#define DBMS_AGGREGATIONFUNCTIONS_H


#include <iostream>
#include <vector>
#include <limits>
class AggregationFunctions {
public:
    static int count(const std::vector<std::string>& data,bool have_all);
    static double average(const std::vector<std::string>& data);
    static int sum(const std::vector<std::string>& data);
    static int max(const std::vector<std::string>& data);
    static int min(const std::vector<std::string>& data);
};
#endif //DBMS_AGGREGATIONFUNCTIONS_H
