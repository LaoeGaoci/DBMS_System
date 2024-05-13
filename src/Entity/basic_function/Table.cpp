#include "Entity/basic_function/Table.h"

//增加字段
void Table::addColumn(const std::string& name, const std::string& type, int length, bool isPrimaryKey, bool isNullable, const std::string& defaultValue) {
    // 检查是否已存在同名列
    for (const auto& column : columns) {
        if (column.name == name) {
            std::cerr << "Error: Column '" << name << "' already exists." << std::endl;
            return;
        }
    }
    columns.push_back({ name, type, length, isPrimaryKey, isNullable, defaultValue });
}
//删除字段
void Table::dropColumn(const std::string& name) {
    auto it = std::remove_if(columns.begin(), columns.end(),
                                                [&name](const Column& column) {
                                                    return column.name == name;
                                                });
    if (it != columns.end()) {
        // 如果找到了要删除的列，则从向量中删除
        columns.erase(it, columns.end());
    } else {
        // 如果未找到，输出错误信息
        std::cerr << "Error: Column '" << name << "' not found." << std::endl;
    }
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