#ifndef RECORD_H
#define RECORD_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <filesystem>

class Table {
public:
    struct Column {
        std::string name;
        std::string type;
        int length;
        bool isPrimaryKey;
        bool isNullable;
        std::string defaultValue;
    };

    std::vector<Column> columns;

    void addColumn(const std::string& name, const std::string& type, int length, bool isPrimaryKey, bool isNullable, const std::string& defaultValue);
    void writeToDisk(std::ofstream& outFile) const;
};

#endif // DATABASE_MANAGER_H
