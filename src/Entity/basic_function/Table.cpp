#include "Entity/basic_function/Table.h"

void Table::addColumn(const std::string& name, const std::string& type, int length, bool isPrimaryKey, bool isNullable, const std::string& defaultValue) {
    columns.push_back({ name, type, length, isPrimaryKey, isNullable, defaultValue });
}

void Table::writeToDisk(std::ofstream& outFile) const {
    int numColumns = columns.size();
    outFile.write(reinterpret_cast<const char*>(&numColumns), sizeof(numColumns));
    for (const auto& column : columns) {
        std::string name = column.name.substr(0, std::min(column.name.size(), size_t(32)));
        name.resize(32, '\0');
        outFile.write(name.c_str(), 32);

        std::string type = column.type.substr(0, std::min(column.type.size(), size_t(32)));
        type.resize(32, '\0');
        outFile.write(type.c_str(), 32);

        outFile.write(reinterpret_cast<const char*>(&column.length), sizeof(column.length));

        outFile.write(reinterpret_cast<const char*>(&column.isPrimaryKey), sizeof(column.isPrimaryKey));

        outFile.write(reinterpret_cast<const char*>(&column.isNullable), sizeof(column.isNullable));

        std::string defaultValue = column.defaultValue.substr(0, std::min(column.defaultValue.size(), size_t(32)));
        defaultValue.resize(32, '\0');
        outFile.write(defaultValue.c_str(), 32);
    }
}
