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

void Table::dropColumn(const std::string& name) {
    auto it = std::remove_if(columns.begin(), columns.end(),
                             [&name](const Column& column) {
                                 return column.name == name;
                             });

    if (it != columns.end()) {
        // 如果找到了要删除的列，则从向量中删除
        columns.erase(it, columns.end());
    }
    else {
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

    int numForeignKeys = foreignKeys.size();
    outFile.write(reinterpret_cast<const char*>(&numForeignKeys), sizeof(numForeignKeys));
    for (const auto& fk : foreignKeys) {
        std::string columnName = fk.columnName.substr(0, std::min(fk.columnName.size(), size_t(32)));
        columnName.resize(32, '\0');
        outFile.write(columnName.c_str(), 32);

        std::string referenceTable = fk.referenceTable.substr(0, std::min(fk.referenceTable.size(), size_t(32)));
        referenceTable.resize(32, '\0');
        outFile.write(referenceTable.c_str(), 32);

        std::string referenceColumn = fk.referenceColumn.substr(0, std::min(fk.referenceColumn.size(), size_t(32)));
        referenceColumn.resize(32, '\0');
        outFile.write(referenceColumn.c_str(), 32);

        outFile.write(reinterpret_cast<const char*>(&fk.onDelete), sizeof(fk.onDelete));
        outFile.write(reinterpret_cast<const char*>(&fk.onUpdate), sizeof(fk.onUpdate));
    }
}

void Table::addForeignKey(const std::string& columnName, const std::string& referenceTable, const std::string& referenceColumn, ForeignKeyAction onDelete, ForeignKeyAction onUpdate) {
    foreignKeys.push_back({ columnName, referenceTable, referenceColumn, onDelete, onUpdate });
}

void Table::removeForeignKey(const std::string& columnName) {
    auto it = std::remove_if(foreignKeys.begin(), foreignKeys.end(), [&columnName](const ForeignKey& fk) {
        return fk.columnName == columnName;
    });
    if (it != foreignKeys.end()) {
        foreignKeys.erase(it, foreignKeys.end());
    }
}