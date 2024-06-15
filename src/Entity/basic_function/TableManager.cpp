#include "Entity/basic_function/TableManager.h"

void TableManager::createTable(const std::string& dbName,
                               const std::string& tableName,
                               const std::vector<std::string>& columnNames,
                               const std::vector<std::string>& columnTypes,
                               const std::vector<int>& columnLengths,
                               const std::vector<bool>& isPrimaryKeys,
                               const std::vector<bool>& isNullables,
                               const std::vector<std::string>& defaultValues,
                               const std::vector<Table::ForeignKey>& foreignKeys,
                               const std::string& createStatement) {
    // 构建数据库路径
    fs::path dbPath = fs::current_path() / "DB" / dbName;
    fs::create_directories(dbPath); // 确保数据库目录存在

    // 构建表文件夹路径
    fs::path tableDirPath = dbPath / tableName;
    fs::create_directories(tableDirPath);

    // 构建三个文件的路径
    fs::path schemaFilePath = tableDirPath / (tableName + ".tdf");
    fs::path dataFilePath = tableDirPath / (tableName + ".trd");
    fs::path constraintFilePath = tableDirPath / (tableName + ".tid");

    // 构建表头并添加约束
    Table table;
    for (size_t i = 0; i < columnNames.size(); i++) {
        table.addColumn(columnNames[i], columnTypes[i], columnLengths[i], isPrimaryKeys[i], isNullables[i], defaultValues[i]);
    }

    for (const auto& fk : foreignKeys) {
        table.addForeignKey(fk.columnName, fk.referenceTable, fk.referenceColumn, fk.onDelete, fk.onUpdate);
    }

    // 将表结构和约束写入到.tdf文件
    std::ofstream schemaFile(schemaFilePath, std::ios::binary);
    table.writeToDisk(schemaFile);
    schemaFile.close();

    // 创建数据文件和约束文件
    std::ofstream dataFile(dataFilePath, std::ios::binary);
    std::ofstream constraintFile(constraintFilePath, std::ios::binary);
    dataFile.close();
    constraintFile.close();
    // 创建备份表文件
    std::filesystem::path redbPath = std::filesystem::current_path() / "Recover" / dbName;
    std::filesystem::create_directories(redbPath); // 确保备份数据库文件夹存在

    std::filesystem::path tableFilePath = redbPath / (tableName + ".sql");
    // 保存建表语句到文件
    std::ofstream tableFile(tableFilePath);
    if (tableFile.is_open()) {
        tableFile << createStatement;
        tableFile.close();
    }
    else {
        std::cerr << "Failed to open file to write create statement for table '" << tableName << "'." << std::endl;
    }
}

void TableManager::deleteTable(const std::string& dbName, const std::string& tableName) {
    // 构建表路径
    fs::path tableDirPath = fs::current_path() / "DB" / dbName / tableName;

    // 尝试删除表文件夹及其所有文件
    std::error_code ec; // 使用error_code避免异常
    if (fs::remove_all(tableDirPath, ec) == 0u) {
        std::cerr << "Failed to delete table. Error: " << ec.message() << std::endl;
    }
    else {
        std::cout << "Table '" << tableName << "' deleted successfully." << std::endl;
    }
}

bool TableManager::loadTableSchema(const std::string& dbName, const std::string& tableName, Table& table) {
    fs::path schemaFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".tdf");
    std::ifstream schemaFile(schemaFilePath, std::ios::binary);
    if (!schemaFile) {
        std::cerr << "Failed to open schema file." << std::endl;
        return false;
    }

    int numColumns;
    schemaFile.read(reinterpret_cast<char*>(&numColumns), sizeof(numColumns));

    table.columns.clear();
    for (int i = 0; i < numColumns; i++) {
        std::string name(32, '\0');
        schemaFile.read(&name[0], 32);
        name.erase(std::find(name.begin(), name.end(), '\0'), name.end());

        std::string type(32, '\0');
        schemaFile.read(&type[0], 32);
        type.erase(std::find(type.begin(), type.end(), '\0'), type.end());

        int length;
        schemaFile.read(reinterpret_cast<char*>(&length), sizeof(length));

        bool isPrimaryKey;
        schemaFile.read(reinterpret_cast<char*>(&isPrimaryKey), sizeof(isPrimaryKey));

        bool isNullable;
        schemaFile.read(reinterpret_cast<char*>(&isNullable), sizeof(isNullable));

        std::string defaultValue(32, '\0');
        schemaFile.read(&defaultValue[0], 32);
        defaultValue.erase(std::find(defaultValue.begin(), defaultValue.end(), '\0'), defaultValue.end());

        table.addColumn(name, type, length, isPrimaryKey, isNullable, defaultValue);
    }

    int numForeignKeys;
    schemaFile.read(reinterpret_cast<char*>(&numForeignKeys), sizeof(numForeignKeys));

    table.foreignKeys.clear();
    for (int i = 0; i < numForeignKeys; i++) {
        std::string columnName(32, '\0');
        schemaFile.read(&columnName[0], 32);
        columnName.erase(std::find(columnName.begin(), columnName.end(), '\0'), columnName.end());

        std::string referenceTable(32, '\0');
        schemaFile.read(&referenceTable[0], 32);
        referenceTable.erase(std::find(referenceTable.begin(), referenceTable.end(), '\0'), referenceTable.end());

        std::string referenceColumn(32, '\0');
        schemaFile.read(&referenceColumn[0], 32);
        referenceColumn.erase(std::find(referenceColumn.begin(), referenceColumn.end(), '\0'), referenceColumn.end());

        Table::ForeignKeyAction onDelete;
        schemaFile.read(reinterpret_cast<char*>(&onDelete), sizeof(onDelete));

        Table::ForeignKeyAction onUpdate;
        schemaFile.read(reinterpret_cast<char*>(&onUpdate), sizeof(onUpdate));

        table.addForeignKey(columnName, referenceTable, referenceColumn, onDelete, onUpdate);
    }

    schemaFile.close();
    return true;
}

void TableManager::insertRecord(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& recordData) {
    // 构建表结构文件路径
    fs::path schemaFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".tdf");
    fs::path dataFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".trd");

    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Error loading table schema. Insert operation aborted." << std::endl;
        return;
    }

    if (recordData.size() != table.columns.size()) {
        std::cerr << "Error: Record data does not match the number of table columns." << std::endl;
        return;
    }

    // 检查外键约束
    for (const auto& fk : table.foreignKeys) {
        int colIdx = -1;
        for (size_t i = 0; i < table.columns.size(); ++i) {
            if (table.columns[i].name == fk.columnName) {
                colIdx = i;
                break;
            }
        }
        if (colIdx == -1) {
            std::cerr << "Foreign key column '" << fk.columnName << "' not found in table schema." << std::endl;
            return;
        }

        // 检查引用的表中是否存在对应的外键值
        if (!checkForeignKeyConstraint(dbName, fk.referenceTable, fk.referenceColumn, recordData[colIdx], table.columns[colIdx].type)) {
            std::cerr << "Foreign key constraint violation: value '" << recordData[colIdx] << "' for column '" << fk.columnName << "' does not exist in reference table '" << fk.referenceTable << "'." << std::endl;
            return;
        }
    }

    std::vector<std::string> effectiveData = recordData;
    std::fstream dataFile(dataFilePath, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
    if (!dataFile) {
        std::cerr << "Failed to open data file for writing." << std::endl;
        return;
    }

    for (size_t i = 0; i < table.columns.size(); i++) {
        if (effectiveData[i].empty() && !table.columns[i].isNullable) {
            if (!table.columns[i].defaultValue.empty()) {
                effectiveData[i] = table.columns[i].defaultValue;
            }
            else {
                std::cerr << "Non-nullable column '" << table.columns[i].name << "' must have a value." << std::endl;
                return;
            }
        }

        if (table.columns[i].type == "integer") {
            int value = std::stoi(effectiveData[i]);
            dataFile.write(reinterpret_cast<char*>(&value), sizeof(value));
        }
        else if (table.columns[i].type == "str") {
            effectiveData[i].resize(table.columns[i].length, '\0');
            dataFile.write(effectiveData[i].c_str(), table.columns[i].length);
        }
        else if (table.columns[i].type == "number") {
            float value = std::stof(effectiveData[i]);
            dataFile.write(reinterpret_cast<char*>(&value), sizeof(value));
        }
        else if (table.columns[i].type == "bool") {
            bool value = (effectiveData[i] == "true" || effectiveData[i] == "1");
            dataFile.write(reinterpret_cast<char*>(&value), sizeof(value));
        }
    }

    dataFile.close();
}

bool TableManager::checkForeignKeyConstraint(const std::string& dbName, const std::string& referenceTable, const std::string& referenceColumn, const std::string& value, const std::string& columnType) {
    Table refTable;
    if (!loadTableSchema(dbName, referenceTable, refTable)) {
        std::cerr << "Error loading reference table schema: " << referenceTable << std::endl;
        return false;
    }

    int colIdx = -1;
    for (size_t i = 0; i < refTable.columns.size(); ++i) {
        if (refTable.columns[i].name == referenceColumn) {
            colIdx = i;
            break;
        }
    }
    if (colIdx == -1) {
        std::cerr << "Reference column '" << referenceColumn << "' not found in reference table schema." << std::endl;
        return false;
    }

    fs::path dataFilePath = fs::current_path() / "DB" / dbName / referenceTable / (referenceTable + ".trd");
    std::ifstream dataFile(dataFilePath, std::ios::binary);
    if (!dataFile) {
        std::cerr << "Failed to open reference data file for reading." << std::endl;
        return false;
    }

    int rowWidth = 0;
    std::vector<int> fieldOffsets;
    for (const auto& col : refTable.columns) {
        fieldOffsets.push_back(rowWidth);
        rowWidth += col.length;
    }

    std::vector<char> rowBuffer(rowWidth);
    while (dataFile.read(rowBuffer.data(), rowWidth)) {
        std::string fieldValue(rowBuffer.data() + fieldOffsets[colIdx], refTable.columns[colIdx].length);
        fieldValue.erase(std::remove(fieldValue.begin(), fieldValue.end(), '\0'), fieldValue.end());

        bool match = false;
        if (columnType == "integer") {
            int intValue;
            memcpy(&intValue, rowBuffer.data() + fieldOffsets[colIdx], sizeof(intValue));
            match = (std::to_string(intValue) == value);
        }
        else if (columnType == "number") {
            float floatValue;
            memcpy(&floatValue, rowBuffer.data() + fieldOffsets[colIdx], sizeof(floatValue));
            match = (std::to_string(floatValue) == value);
        }
        else {
            match = (fieldValue == value);
        }

        if (match) {
            dataFile.close();
            return true;
        }
    }

    dataFile.close();
    return false;
}

void TableManager::readTableData(const std::string& dbName, const std::string& tableName) {
    // 构建表结构文件路径和数据文件路径
    fs::path schemaFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".tdf");
    fs::path dataFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".trd");

    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    std::ifstream dataFile(dataFilePath, std::ios::binary);
    if (!dataFile) {
        std::cerr << "Failed to open data file for reading." << std::endl;
        return;
    }

    std::cout << std::endl;

    // 输出表头，即列名
    for (const auto& col : table.columns) {
        std::cout << col.name << "\t";
    }
    std::cout << std::endl;

    // 读取数据
    while (dataFile && dataFile.peek() != EOF) {
        for (const auto& col : table.columns) {
            if (col.type == "integer") {
                int value;
                dataFile.read(reinterpret_cast<char*>(&value), sizeof(value));
                if (!dataFile) break;
                std::cout << value << "\t";
            }
            else if (col.type == "str") {
                std::string value(col.length, '\0');
                dataFile.read(&value[0], col.length);
                if (!dataFile) break;
                value.erase(std::remove(value.begin(), value.end(), '\0'), value.end());
                std::cout << value << "\t";
            }
            else if (col.type == "str") {
                float value;
                dataFile.read(reinterpret_cast<char*>(&value), sizeof(value));
                std::cout << value << "\t";
            }
            else if (col.type == "str") {
                bool value;
                dataFile.read(reinterpret_cast<char*>(&value), sizeof(value));
                std::cout << (value ? "true" : "false") << "\t";
            }
        }
        std::cout << std::endl;
    }

    dataFile.close();
}

void TableManager::readRecords(const std::string& dbName, const std::string& tableName,
                               const std::vector<std::string>& fieldNames,
                               const std::vector<std::string>& conditionColumn,
                               const std::vector<std::string>& operation,
                               const std::vector<std::string>& conditionValue) {
    // 构建表结构文件路径和数据文件路径
    fs::path schemaFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".tdf");
    fs::path dataFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".trd");

    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    std::ifstream dataFile(dataFilePath, std::ios::binary);
    if (!dataFile) {
        std::cerr << "Failed to open data file for reading." << std::endl;
        return;
    }

    int rowWidth = 0;
    std::vector<int> fieldOffsets;
    for (const auto& col : table.columns) {
        fieldOffsets.push_back(rowWidth);
        rowWidth += col.length;
    }

    std::vector<char> rowBuffer(rowWidth);
    std::string fieldValue;

    // 输出列名作为表头
    std::cout << std::endl;
    for (const auto& fieldName : fieldNames) {
        std::cout << fieldName << "\t";
    }
    std::cout << std::endl;

    // 创建列名到索引的映射
    std::map<std::string, int> columnMap;
    for (int i = 0; i < table.columns.size(); ++i) {
        columnMap[table.columns[i].name] = i;
    }

    // 现在可以使用columnMap来获取正确的索引
    while (dataFile.read(rowBuffer.data(), rowWidth)) {
        bool matches = true;
        for (size_t i = 0; i < conditionColumn.size(); ++i) {
            const auto& colName = conditionColumn[i];
            const auto& op = operation[i];
            const auto& value = conditionValue[i];

            if (columnMap.find(colName) != columnMap.end()) {
                int columnIndex = columnMap[colName];
                fieldValue.assign(rowBuffer.data() + fieldOffsets[columnIndex], table.columns[columnIndex].length);
                fieldValue.erase(std::remove(fieldValue.begin(), fieldValue.end(), '\0'), fieldValue.end());
                matches &= checkCondition(fieldValue, op, value);
            }
        }

        if (matches) {
            for (const auto& col : table.columns) {
                if (std::find(fieldNames.begin(), fieldNames.end(), col.name) != fieldNames.end()) {
                    int columnIndex = columnMap[col.name];
                    fieldValue.assign(rowBuffer.data() + fieldOffsets[columnIndex], col.length);
                    fieldValue.erase(std::remove(fieldValue.begin(), fieldValue.end(), '\0'), fieldValue.end());
                    if (col.type == "integer") {
                        int intValue = static_cast<unsigned char>(fieldValue[0]);
                        fieldValue = std::to_string(intValue);
                        std::cout << fieldValue << "\t";
                    }
                    else if (col.type == "number") {
                        float value;
                        dataFile.read(reinterpret_cast<char*>(&value), sizeof(value));
                        std::cout << value << "\t";
                    }
                    else {
                        std::cout << fieldValue << "\t";
                    }
                }
            }
            std::cout << std::endl;
        }
    }

    dataFile.close();
}

void TableManager::deleteRecords(const std::string& dbName, const std::string& tableName,
                                 const std::vector<std::string>& conditionColumn,
                                 const std::vector<std::string>& operation,
                                 const std::vector<std::string>& conditionValue) {
    fs::path dataFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".trd");
    fs::path tempPath = dataFilePath;
    tempPath += ".tmp";

    std::ifstream inFile(dataFilePath, std::ios::binary);
    std::ofstream outFile(tempPath, std::ios::binary);

    if (!inFile || !outFile) {
        std::cerr << "Failed to open files for processing." << std::endl;
        return;
    }

    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    int rowWidth = 0;
    std::vector<int> fieldOffsets;
    for (const auto& col : table.columns) {
        fieldOffsets.push_back(rowWidth);
        rowWidth += col.length;
    }

    std::vector<char> rowBuffer(rowWidth);
    std::map<std::string, int> columnMap;
    for (int i = 0; i < table.columns.size(); ++i) {
        columnMap[table.columns[i].name] = i;
    }

    bool violationDetected = false;

    while (inFile.read(rowBuffer.data(), rowWidth)) {
        bool matches = true;
        for (size_t i = 0; i < conditionColumn.size(); ++i) {
            if (columnMap.find(conditionColumn[i]) != columnMap.end()) {
                int columnIndex = columnMap[conditionColumn[i]];
                std::string fieldValue(rowBuffer.data() + fieldOffsets[columnIndex], table.columns[columnIndex].length);
                fieldValue.erase(std::remove(fieldValue.begin(), fieldValue.end(), '\0'), fieldValue.end());
                matches &= checkCondition(fieldValue, operation[i], conditionValue[i]);
            }
        }

        if (matches) {
            for (const auto& fk : table.foreignKeys) {
                std::string value(rowBuffer.data() + fieldOffsets[columnMap[fk.columnName]], table.columns[columnMap[fk.columnName]].length);
                value.erase(std::remove(value.begin(), value.end(), '\0'), value.end());
                if (!handleForeignKeyAction(dbName, table, fk.columnName, value, fk.onDelete)) {
                    violationDetected = true;
                    break;
                }
            }

            if (!violationDetected) {
                continue; // Skip writing this row to the new file
            }
        }

        outFile.write(rowBuffer.data(), rowWidth);
    }

    inFile.close();
    outFile.close();

    if (violationDetected) {
        std::cerr << "Foreign key constraint violation. Deletion aborted." << std::endl;
        fs::remove(tempPath); // Remove temporary file
    }
    else {
        fs::remove(dataFilePath);
        fs::rename(tempPath, dataFilePath);
    }
}

bool TableManager::checkCondition(const std::string& fieldValue, const std::string& op, const std::string& value) {
    if (op == "=") return fieldValue == value;
    if (op == "!=") return fieldValue != value;
    if (op == "<" && isNumber(fieldValue)) return std::stod(fieldValue) < std::stod(value);
    if (op == ">" && isNumber(fieldValue)) return std::stod(fieldValue) > std::stod(value);
    if (op == "<=" && isNumber(fieldValue)) return std::stod(fieldValue) <= std::stod(value);
    if (op == ">=" && isNumber(fieldValue)) return std::stod(fieldValue) >= std::stod(value);
    return false;
}

bool TableManager::isNumber(const std::string& str) {
    return !str.empty() && str.find_first_not_of("0123456789.-") == std::string::npos;
}

void TableManager::updateTable(const std::string& dbName,
                               const std::string& tableName,
                               const std::vector<std::string>& conditionColumn,
                               const std::vector<std::string>& operation,
                               const std::vector<std::string>& conditionValue,
                               const std::vector<std::string>& updateColumn,
                               const std::vector<std::string>& updateValue) {
    fs::path dataFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".trd");
    fs::path tempPath = dataFilePath;
    tempPath += ".tmp";

    std::ifstream inFile(dataFilePath, std::ios::binary);
    std::ofstream outFile(tempPath, std::ios::binary);

    if (!inFile || !outFile) {
        std::cerr << "Failed to open files for processing." << std::endl;
        return;
    }

    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    int rowWidth = 0;
    std::vector<int> fieldOffsets;
    for (const auto& col : table.columns) {
        fieldOffsets.push_back(rowWidth);
        rowWidth += col.length;
    }

    std::vector<char> rowBuffer(rowWidth);
    std::map<std::string, int> columnMap;
    for (int i = 0; i < table.columns.size(); ++i) {
        columnMap[table.columns[i].name] = i;
    }

    bool violationDetected = false;

    while (inFile.read(rowBuffer.data(), rowWidth)) {
        bool matches = true;
        for (size_t i = 0; i < conditionColumn.size(); ++i) {
            if (columnMap.find(conditionColumn[i]) != columnMap.end()) {
                int columnIndex = columnMap[conditionColumn[i]];
                std::string fieldValue(rowBuffer.data() + fieldOffsets[columnIndex], table.columns[columnIndex].length);
                fieldValue.erase(std::remove(fieldValue.begin(), fieldValue.end(), '\0'), fieldValue.end());
                matches = checkCondition(fieldValue, operation[i], conditionValue[i]);
                if (!matches) break;
            }
        }

        if (matches) {
            // 检查并处理外键约束
            for (size_t i = 0; i < updateColumn.size(); ++i) {
                if (columnMap.find(updateColumn[i]) != columnMap.end()) {
                    int updateIndex = columnMap[updateColumn[i]];
                    std::string newValue = updateValue[i];

                    for (const auto& fk : table.foreignKeys) {
                        if (fk.columnName == updateColumn[i]) {
                            std::string oldValue(rowBuffer.data() + fieldOffsets[updateIndex], table.columns[updateIndex].length);
                            oldValue.erase(std::remove(oldValue.begin(), oldValue.end(), '\0'), oldValue.end());

                            // 执行外键更新动作
                            if (!handleForeignKeyAction(dbName, table, updateColumn[i], oldValue, fk.onUpdate)) {
                                std::cerr << "Foreign key constraint violation on column '" << updateColumn[i] << "'." << std::endl;
                                violationDetected = true;
                                break; // 外键约束违规，退出循环
                            }
                        }
                    }

                    if (violationDetected) {
                        break; // 外键约束违规，退出循环
                    }

                    // 更新主键检查
                    if (table.columns[updateIndex].isPrimaryKey) {
                        if (updateValue[i].empty()) {
                            std::cerr << "Attempt to set primary key '" << updateColumn[i] << "' to an empty value denied." << std::endl;
                            continue; // 跳过这个更新，继续处理其他更新
                        }
                    }

                    newValue.resize(table.columns[updateIndex].length, '\0');
                    std::memcpy(rowBuffer.data() + fieldOffsets[updateIndex], newValue.data(), newValue.size());
                }
            }

            if (violationDetected) {
                break; // 外键约束违规，退出循环
            }
        }

        outFile.write(rowBuffer.data(), rowWidth);
    }

    inFile.close();
    outFile.close();

    if (violationDetected) {
        std::cerr << "Foreign key constraint violation. Update aborted." << std::endl;
        fs::remove(tempPath); // 删除临时文件
    } else {
        fs::remove(dataFilePath);
        fs::rename(tempPath, dataFilePath);
    }
}





void TableManager::orderByRecord(const std::string& dbName,
                                 const std::string& tableName,
                                 const std::vector<std::string>& sortColumn,
                                 const std::vector<std::string>& orders,
                                 const std::vector<std::string>& fieldNames) {
    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    // 创建列索引映射
    std::map<std::string, int> columnIndex;
    for (size_t i = 0; i < table.columns.size(); ++i) {
        columnIndex[table.columns[i].name] = i;
    }

    // 读取并排序数据
    std::vector<std::vector<std::string>> records = readSortTableData(dbName, tableName, table);

    // 排序数据
    std::sort(records.begin(), records.end(), [&columnIndex, &sortColumn, &orders](const std::vector<std::string>& a, const std::vector<std::string>& b) {
        for (size_t i = 0; i < sortColumn.size(); ++i) {
            int colIndex = columnIndex[sortColumn[i]];
            bool asc = orders[i] == "ASC";
            if (a[colIndex] != b[colIndex])
                return asc ? a[colIndex] < b[colIndex] : a[colIndex] > b[colIndex];
        }

        return false;
    });

    std::cout << std::endl;

    // 输出表头
    for (const auto& fieldName : fieldNames) {
        std::cout << fieldName << "\t";
    }

    std::cout << std::endl;

    // 打印排序后的数据
    for (const auto& row : records) {
        for (const auto& fieldName : fieldNames) {
            int colIndex = columnIndex[fieldName];
            std::cout << row[colIndex] << "\t";
        }

        std::cout << std::endl;
    }
}

std::vector<std::vector<std::string>> TableManager::readSortTableData(const std::string& dbName,
                                                                      const std::string& tableName,
                                                                      const Table& table) {
    std::vector<std::vector<std::string>> data;
    fs::path dataFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".trd");
    std::ifstream dataFile(dataFilePath, std::ios::binary);

    if (!dataFile) {
        std::cerr << "Failed to open data file for reading." << std::endl;
        return data;
    }

    while (dataFile.peek() != EOF) {
        std::vector<std::string> record;
        for (const auto& col : table.columns) {
            std::string value;
            if (col.type == "integer") {
                int intVal;
                dataFile.read(reinterpret_cast<char*>(&intVal), sizeof(intVal));
                value = std::to_string(intVal);
            }
            else if (col.type == "str") {
                char buffer[256];
                dataFile.read(buffer, col.length);
                buffer[col.length] = '\0';
                value = std::string(buffer);
            }
            else if (col.type == "number") {
                float numVal;
                dataFile.read(reinterpret_cast<char*>(&numVal), sizeof(numVal));
                value = std::to_string(numVal);
            }
            else if (col.type == "bool") {
                bool boolVal;
                dataFile.read(reinterpret_cast<char*>(&boolVal), sizeof(boolVal));
                value = boolVal ? "true" : "false";
            }

            record.push_back(value);
        }

        data.push_back(record);
    }

    dataFile.close();
    return data;
}

int calculateRowWidth(const std::vector<Table::Column>& columns) {
    int width = 0;
    for (const auto& col : columns) {
        width += col.length;
    }
    return width;
}

void TableManager::alter_addColumnToTable(const std::string& dbName,
                                          const std::string& tableName,
                                          const std::vector<std::string>& columnNames,
                                          const std::vector<std::string>& columnTypes,
                                          const std::vector<int>& columnLengths,
                                          const std::vector<bool>& isPrimaryKeys,
                                          const std::vector<bool>& isNullables,
                                          const std::vector<std::string>& defaultValues) {
    fs::path tableDirPath = fs::current_path() / "DB" / dbName / tableName;
    fs::path dataFilePath = tableDirPath / (tableName + ".trd");
    fs::path tempFilePath = dataFilePath;
    tempFilePath += ".tmp";
    fs::path schemaFilePath = tableDirPath / (tableName + ".tdf");

    std::ifstream inFile(dataFilePath, std::ios::binary);
    std::ofstream outFile(tempFilePath, std::ios::binary);

    if (!inFile || !outFile) {
        std::cerr << "Failed to open files for processing." << std::endl;
        return;
    }

    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    int originalRowWidth = calculateRowWidth(table.columns);
    std::vector<char> rowBuffer(originalRowWidth);

    for (size_t i = 0; i < columnNames.size(); ++i) {
        table.addColumn(columnNames[i], columnTypes[i], columnLengths[i], isPrimaryKeys[i], isNullables[i], defaultValues[i]);
    }

    int newRowWidth = calculateRowWidth(table.columns);
    std::vector<char> newRowBuffer(newRowWidth);


    std::ofstream schemaOutFile(schemaFilePath, std::ios::binary | std::ios::trunc);
    table.writeToDisk(schemaOutFile);
    schemaOutFile.close();

    while (inFile.read(rowBuffer.data(), originalRowWidth)) {
        std::memcpy(newRowBuffer.data(), rowBuffer.data(), originalRowWidth);

        int startPosition = originalRowWidth;
        for (size_t i = 0; i < columnNames.size(); ++i) {
            std::string defaultValue = defaultValues[i];
            defaultValue.resize(columnLengths[i], '\0');  // Ensure fixed-length columns
            std::memcpy(newRowBuffer.data() + startPosition, defaultValue.data(), columnLengths[i]);
            startPosition += columnLengths[i];
        }

        outFile.write(newRowBuffer.data(), newRowWidth);
    }

    inFile.close();
    outFile.close();

    fs::remove(dataFilePath);
    fs::rename(tempFilePath, dataFilePath);

    std::cout << "Columns added successfully and data file updated." << std::endl;
}

void TableManager::alter_deleteColumns(const std::string& dbName,
                                       const std::string& tableName,
                                       const std::vector<std::string>& columnsToDelete) {
    fs::path tableDirPath = fs::current_path() / "DB" / dbName / tableName;
    fs::path dataFilePath = tableDirPath / (tableName + ".trd");
    fs::path tempFilePath = dataFilePath;
    tempFilePath += ".tmp";
    fs::path schemaFilePath = tableDirPath / (tableName + ".tdf");

    std::ifstream inFile(dataFilePath, std::ios::binary);
    std::ofstream outFile(tempFilePath, std::ios::binary);

    if (!inFile || !outFile) {
        std::cerr << "Failed to open files for processing." << std::endl;
        return;
    }

    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    int rowWidth = 0;
    std::vector<int> fieldOffsets;
    for (const auto& col : table.columns) {
        fieldOffsets.push_back(rowWidth);
        rowWidth += col.length;
    }

    std::vector<char> rowBuffer(rowWidth);
    std::map<std::string, int> columnMap;
    for (int i = 0; i < table.columns.size(); ++i) {
        columnMap[table.columns[i].name] = i;
    }

    std::set<int> columnsToSkip;
    for (const auto& colName : columnsToDelete) {
        if (columnMap.find(colName) != columnMap.end()) {
            columnsToSkip.insert(columnMap[colName]);
        }
    }

    while (inFile.read(rowBuffer.data(), rowWidth)) {
        for (int i = 0; i < table.columns.size(); ++i) {
            if (columnsToSkip.find(i) == columnsToSkip.end()) {
                outFile.write(rowBuffer.data() + fieldOffsets[i], table.columns[i].length);
            }
        }
    }

    inFile.close();
    outFile.close();

    std::vector<Table::Column> updatedColumns;
    for (int i = 0; i < table.columns.size(); ++i) {
        if (columnsToSkip.find(i) == columnsToSkip.end()) {
            updatedColumns.push_back(table.columns[i]);
        }
    }
    table.columns = updatedColumns; // Update the column list

    std::ofstream schemaOutFile(schemaFilePath, std::ios::binary | std::ios::trunc);
    if (!schemaOutFile) {
        std::cerr << "Failed to open schema file for writing." << std::endl;
        return;
    }
    table.writeToDisk(schemaOutFile);
    schemaOutFile.close();

    fs::remove(dataFilePath);
    fs::rename(tempFilePath, dataFilePath);

    std::cout << "Specified columns have been successfully deleted from the file and the schema updated." << std::endl;
}

bool TableManager::renameTable(const std::string& dbName,
                               const std::string& oldTableName,
                               const std::string& newTableName) {
    fs::path dbPath = fs::current_path() / "DB" / dbName;
    fs::path recoverPath = fs::current_path() / "Recover" / dbName;
    fs::path oldTablePath = dbPath / oldTableName;
    fs::path newTablePath = dbPath / newTableName;

    // 检查新表名是否已经存在
    if (fs::exists(newTablePath)) {
        std::cerr << "A table with the name '" << newTableName << "' already exists." << std::endl;
        return false;
    }

    // 重命名表文件夹
    try {
        fs::rename(oldTablePath, newTablePath);
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to rename table directory: " << e.what() << std::endl;
        return false;
    }

    // 更新文件夹中所有相关文件的名称
    std::vector<std::string> fileExtensions = { ".tdf", ".trd", ".tid" };
    for (const auto& ext : fileExtensions) {
        fs::path oldFilePath = newTablePath / (oldTableName + ext);
        fs::path newFilePath = newTablePath / (newTableName + ext);
        if (fs::exists(oldFilePath)) {
            try {
                fs::rename(oldFilePath, newFilePath);
            }
            catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename file '" << oldFilePath << "' to '" << newFilePath << "': " << e.what() << std::endl;
                return false;
            }
        }
    }

    // 更新恢复文件夹中的相关文件名
    fs::path oldRecoverFilePath = recoverPath / (oldTableName + ".sql");
    fs::path newRecoverFilePath = recoverPath / (newTableName + ".sql");
    if (fs::exists(oldRecoverFilePath)) {
        try {
            fs::rename(oldRecoverFilePath, newRecoverFilePath);
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename recover file '" << oldRecoverFilePath << "' to '" << newRecoverFilePath << "': " << e.what() << std::endl;
            return false;
        }
    }

    std::cout << "Table and all associated files renamed successfully from '" << oldTableName << "' to '" << newTableName << "'." << std::endl;
    return true;
}

std::vector<std::string> TableManager::listAllDatabases() const {
    std::cout << std::endl;
    std::cout << "List All Databases:" << std::endl;
    std::vector<std::string> databases;
    fs::path dbRootPath = fs::current_path() / "DB";

    if (!fs::exists(dbRootPath) || !fs::is_directory(dbRootPath)) {
        std::cerr << "Database directory does not exist." << std::endl;
        return databases;
    }

    // 遍历 databases 目录下的所有文件夹
    for (const auto& entry : fs::directory_iterator(dbRootPath)) {
        if (entry.is_directory()) {
            databases.push_back(entry.path().filename().string()); // 添加文件夹名到列表，即数据库名
        }
    }

    return databases;
}

void TableManager::useDatabase(const std::string& dbName) {
    std::cout << std::endl;
    currentDatabase = dbName;
    std::cout << "Database changed to: " << dbName << std::endl;
}

std::string TableManager::selectDatabase() const {
    return currentDatabase.empty() ? "No database selected." : currentDatabase;
}

void TableManager::showTables() {
    if (currentDatabase.empty()) {
        std::cout << "No database selected. Please select a database first." << std::endl;
        return;
    }

    fs::path dbPath = fs::current_path() / "DB" / currentDatabase;
    if (!fs::exists(dbPath) || !fs::is_directory(dbPath)) {
        std::cout << "The selected database does not exist or is not accessible." << std::endl;
        return;
    }

    std::cout << std::endl;
    std::cout << "Tables in '" << currentDatabase << "':" << std::endl;
    for (const auto& entry : fs::directory_iterator(dbPath)) {
        if (fs::is_directory(entry)) {
            std::cout << "  - " << entry.path().filename().string() << std::endl;
        }
    }
}

void TableManager::describeTable(const std::string& tableName) {
    if (currentDatabase.empty()) {
        std::cout << "No database selected. Please select a database first." << std::endl;
        return;
    }

    fs::path schemaFilePath = fs::current_path() / "DB" / currentDatabase / tableName / (tableName + ".tdf");
    if (!fs::exists(schemaFilePath)) {
        std::cout << "Table '" << tableName << "' does not exist." << std::endl;
        return;
    }

    Table table;
    if (!loadTableSchema(currentDatabase, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    std::cout << "Description of '" << tableName << "':" << std::endl;

    // 设置表头的宽度
    std::cout << std::left
              << std::setw(15) << "Column"
              << std::setw(10) << "Type"
              << std::setw(10) << "Length"
              << std::setw(15) << "Primary Key"
              << std::setw(10) << "Nullable"
              << std::setw(15) << "Default"
              << std::endl;

    // 打印每列的信息
    for (const auto& column : table.columns) {
        std::cout << std::left
                  << std::setw(15) << column.name
                  << std::setw(10) << column.type
                  << std::setw(10) << column.length
                  << std::setw(15) << (column.isPrimaryKey ? "True" : "False")
                  << std::setw(10) << (column.isNullable ? "True" : "False")
                  << std::setw(15) << column.defaultValue
                  << std::endl;
    }

    // 打印外键信息
    if (!table.foreignKeys.empty()) {
        std::cout << "\nForeign Keys:\n";
        std::cout << std::left
                  << std::setw(15) << "Column"
                  << std::setw(20) << "References Table"
                  << std::setw(20) << "References Column"
                  << std::setw(10) << "On Delete"
                  << std::setw(10) << "On Update"
                  << std::endl;
        for (const auto& foreignKey : table.foreignKeys) {
            std::string columnName = foreignKey.columnName;
            columnName.erase(std::remove(columnName.begin(), columnName.end(), '\0'), columnName.end());
            std::string referenceTable = foreignKey.referenceTable;
            referenceTable.erase(std::remove(referenceTable.begin(), referenceTable.end(), '\0'), referenceTable.end());
            std::string referenceColumn = foreignKey.referenceColumn;
            referenceColumn.erase(std::remove(referenceColumn.begin(), referenceColumn.end(), '\0'), referenceColumn.end());
            std::cout << std::left
                      << std::setw(15) << columnName
                      << std::setw(20) << referenceTable
                      << std::setw(20) << referenceColumn
                      << std::setw(10) << foreignKeyActionToString(foreignKey.onDelete)
                      << std::setw(10) << foreignKeyActionToString(foreignKey.onUpdate)
                      << std::endl;
        }
    }
}

std::string TableManager::foreignKeyActionToString(Table::ForeignKeyAction action) {
    switch (action) {
        case Table::ForeignKeyAction::RESTRICT:
            return "RESTRICT";
        case Table::ForeignKeyAction::NOACTION:
            return "NOACTION";
        case Table::ForeignKeyAction::CASCADE:
            return "CASCADE";
        case Table::ForeignKeyAction::SET_NULL:
            return "SET_NULL";
        case Table::ForeignKeyAction::SET_DEFAULT:
            return "SET_DEFAULT";
        default:
            return "UNKNOWN";
    }
}

void TableManager::showCreateTable(const std::string& dbName, const std::string& tableName) {
    std::filesystem::path tableFilePath = std::filesystem::current_path() / "Recover" / dbName / (tableName + ".sql");

    if (!std::filesystem::exists(tableFilePath)) {
        std::cout << "Table '" << tableName << "' does not exist or create statement is not saved." << std::endl;
        return;
    }

    std::ifstream tableFile(tableFilePath);
    std::string line, createStatement;

    if (tableFile.is_open()) {
        while (std::getline(tableFile, line)) {
            createStatement += line + "\n";
        }
        tableFile.close();

        std::cout << "CREATE TABLE statement for '" << tableName << "':\n" << createStatement << std::endl;
    }
    else {
        std::cerr << "Failed to open create statement file for table '" << tableName << "'." << std::endl;
    }
}

void TableManager::truncateTable(const std::string& dbName, const std::string& tableName) {
    // 数据文件路径
    fs::path dataFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".trd");

    // 打开数据文件并清空内容
    std::ofstream dataFile(dataFilePath, std::ios::trunc);
    if (!dataFile.is_open()) {
        std::cerr << "Failed to truncate table. Unable to open data file." << std::endl;
    }
    else {
        std::cout << "Table '" << tableName << "' truncated successfully." << std::endl;
        dataFile.close();  // 关闭文件
    }
}

void TableManager::innerJoin(const std::string& dbName,
                             const std::string& table1,
                             const std::string& table2,
                             const std::string& column1,
                             const std::string& column2,
                             const std::vector<std::string>& selectColumns) {
    Table tableA, tableB;
    if (!loadTableSchema(dbName, table1, tableA) || !loadTableSchema(dbName, table2, tableB)) {
        std::cerr << "Error loading table schema for one or both tables." << std::endl;
        return;
    }

    fs::path dataFilePath1 = fs::current_path() / "DB" / dbName / table1 / (table1 + ".trd");
    fs::path dataFilePath2 = fs::current_path() / "DB" / dbName / table2 / (table2 + ".trd");

    std::ifstream dataFile1(dataFilePath1, std::ios::binary);
    std::ifstream dataFile2(dataFilePath2, std::ios::binary);

    if (!dataFile1 || !dataFile2) {
        std::cerr << "Failed to open data files for one or both tables." << std::endl;
        return;
    }

    // 获取连接列的索引
    int colIdx1 = -1, colIdx2 = -1;
    for (size_t i = 0; i < tableA.columns.size(); ++i) {
        if (tableA.columns[i].name == column1) {
            colIdx1 = i;
            break;
        }
    }
    for (size_t i = 0; i < tableB.columns.size(); ++i) {
        if (tableB.columns[i].name == column2) {
            colIdx2 = i;
            break;
        }
    }

    if (colIdx1 == -1 || colIdx2 == -1) {
        std::cerr << "One or both columns for join not found." << std::endl;
        return;
    }

    std::cout << std::endl;

    // 输出选择的列作为表头
    for (const auto& col : selectColumns) {
        std::cout << col << "\t";
    }
    std::cout << std::endl;

    // 计算每行的宽度和列的偏移量
    int rowWidth1 = 0, rowWidth2 = 0;
    std::vector<int> fieldOffsets1, fieldOffsets2;
    for (const auto& col : tableA.columns) {
        fieldOffsets1.push_back(rowWidth1);
        rowWidth1 += col.length;
    }
    for (const auto& col : tableB.columns) {
        fieldOffsets2.push_back(rowWidth2);
        rowWidth2 += col.length;
    }

    // 创建列名到索引的映射
    std::map<std::string, int> columnMapA, columnMapB;
    for (int i = 0; i < tableA.columns.size(); ++i) {
        columnMapA[tableA.columns[i].name] = i;
    }
    for (int i = 0; i < tableB.columns.size(); ++i) {
        columnMapB[tableB.columns[i].name] = i;
    }

    // 读取并匹配行数据
    std::vector<char> rowBuffer1(rowWidth1);
    std::vector<char> rowBuffer2(rowWidth2);

    while (dataFile1.read(rowBuffer1.data(), rowWidth1)) {
        std::string value1(rowBuffer1.data() + fieldOffsets1[colIdx1], tableA.columns[colIdx1].length);
        value1.erase(std::remove(value1.begin(), value1.end(), '\0'), value1.end());

        dataFile2.clear(); // 重置EOF标志
        dataFile2.seekg(0, std::ios::beg); // 重置文件指针

        while (dataFile2.read(rowBuffer2.data(), rowWidth2)) {
            std::string value2(rowBuffer2.data() + fieldOffsets2[colIdx2], tableB.columns[colIdx2].length);
            value2.erase(std::remove(value2.begin(), value2.end(), '\0'), value2.end());

            bool match = false;
            if (tableA.columns[colIdx1].type == "integer" || tableA.columns[colIdx1].type == "number") {
                if (tableA.columns[colIdx1].type == "integer") {
                    int intValue1, intValue2;
                    memcpy(&intValue1, rowBuffer1.data() + fieldOffsets1[colIdx1], sizeof(intValue1));
                    memcpy(&intValue2, rowBuffer2.data() + fieldOffsets2[colIdx2], sizeof(intValue2));
                    match = (intValue1 == intValue2);
                }
                else if (tableA.columns[colIdx1].type == "number") {
                    float floatValue1, floatValue2;
                    memcpy(&floatValue1, rowBuffer1.data() + fieldOffsets1[colIdx1], sizeof(floatValue1));
                    memcpy(&floatValue2, rowBuffer2.data() + fieldOffsets2[colIdx2], sizeof(floatValue2));
                    match = (floatValue1 == floatValue2);
                }
            }
            else {
                match = (value1 == value2);
            }

            if (match) {
                for (const auto& col : selectColumns) {
                    bool found = false;
                    for (const auto& aCol : tableA.columns) {
                        if (aCol.name == col) {
                            found = true;
                            std::string value(rowBuffer1.data() + fieldOffsets1[columnMapA[aCol.name]], aCol.length);
                            if (aCol.type == "integer") {
                                int intValue;
                                memcpy(&intValue, value.data(), sizeof(intValue));
                                value = std::to_string(intValue);
                            }
                            else if (aCol.type == "number") {
                                float floatValue;
                                memcpy(&floatValue, value.data(), sizeof(floatValue));
                                value = std::to_string(floatValue);
                            }
                            else {
                                value.erase(std::remove(value.begin(), value.end(), '\0'), value.end());
                            }

                            std::cout << value << "\t";
                            break;
                        }
                    }
                    if (!found) {
                        for (const auto& bCol : tableB.columns) {
                            if (bCol.name == col) {
                                std::string value(rowBuffer2.data() + fieldOffsets2[columnMapB[bCol.name]], bCol.length);
                                if (bCol.type == "integer") {
                                    int intValue;
                                    memcpy(&intValue, value.data(), sizeof(intValue));
                                    value = std::to_string(intValue);
                                }
                                else if (bCol.type == "number") {
                                    float floatValue;
                                    memcpy(&floatValue, value.data(), sizeof(floatValue));
                                    value = std::to_string(floatValue);
                                }
                                else {
                                    value.erase(std::remove(value.begin(), value.end(), '\0'), value.end());
                                }

                                std::cout << value << "\t";
                                break;
                            }
                        }
                    }
                }
                std::cout << std::endl;
            }
        }
    }

    dataFile1.close();
    dataFile2.close();
}

void TableManager::alter_addForeignKey(const std::string& dbName,
                                       const std::string& tableName,
                                       const std::string& columnName,
                                       const std::string& referenceTable,
                                       const std::string& referenceColumn,
                                       Table::ForeignKeyAction onDelete,
                                       Table::ForeignKeyAction onUpdate) {
    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Error loading table schema." << std::endl;
        return;
    }

    table.addForeignKey(columnName, referenceTable, referenceColumn, onDelete, onUpdate);

    // 构建表结构文件路径
    fs::path schemaFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".tdf");
    std::ofstream schemaFile(schemaFilePath, std::ios::binary);
    table.writeToDisk(schemaFile);
    schemaFile.close();

    std::cout << "Foreign key added successfully." << std::endl;
}

void TableManager::alter_deleteForeignKey(const std::string& dbName, const std::string& tableName, const std::string& columnName) {
    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Error loading table schema." << std::endl;
        return;
    }

    table.removeForeignKey(columnName);

    // 构建表结构文件路径
    fs::path schemaFilePath = fs::current_path() / "DB" / dbName / tableName / (tableName + ".tdf");
    std::ofstream schemaFile(schemaFilePath, std::ios::binary);
    table.writeToDisk(schemaFile);
    schemaFile.close();

    std::cout << "Foreign key deleted successfully." << std::endl;
}

bool TableManager::handleForeignKeyAction(const std::string& dbName,
                                          const Table& table,
                                          const std::string& columnName,
                                          const std::string& value,
                                          Table::ForeignKeyAction action) {
    for (const auto& fk : table.foreignKeys) {
        if (fk.columnName == columnName) {
            Table childTable;
            if (!loadTableSchema(dbName, fk.referenceTable, childTable)) {
                std::cerr << "Failed to load schema for child table: " << fk.referenceTable << std::endl;
                continue;
            }

            fs::path dataFilePath = fs::current_path() / "DB" / dbName / fk.referenceTable / (fk.referenceTable + ".trd");
            fs::path tempPath = dataFilePath;
            tempPath += ".tmp";

            std::ifstream inFile(dataFilePath, std::ios::binary);
            std::ofstream outFile(tempPath, std::ios::binary);

            if (!inFile || !outFile) {
                std::cerr << "Failed to open files for processing." << std::endl;
                continue;
            }

            int rowWidth = 0;
            std::vector<int> fieldOffsets;
            for (const auto& col : childTable.columns) {
                fieldOffsets.push_back(rowWidth);
                rowWidth += col.length;
            }

            std::vector<char> rowBuffer(rowWidth);
            std::map<std::string, int> columnMap;
            for (int i = 0; i < childTable.columns.size(); ++i) {
                columnMap[childTable.columns[i].name] = i;
            }

            bool foreignKeyFound = false;

            while (inFile.read(rowBuffer.data(), rowWidth)) {
                std::string fieldValue(rowBuffer.data() + fieldOffsets[columnMap[fk.referenceColumn]], childTable.columns[columnMap[fk.referenceColumn]].length);
                fieldValue.erase(std::remove(fieldValue.begin(), fieldValue.end(), '\0'), fieldValue.end());

                if (fieldValue == value) {
                    foreignKeyFound = true;
                    if (action == Table::ForeignKeyAction::RESTRICT || action == Table::ForeignKeyAction::NOACTION) {
                        inFile.close();
                        outFile.close();
                        fs::remove(tempPath);
                        return false;
                    }
                    else if (action == Table::ForeignKeyAction::CASCADE) {
                        continue;
                    }
                    else if (action == Table::ForeignKeyAction::SET_NULL) {
                        std::memset(rowBuffer.data() + fieldOffsets[columnMap[fk.referenceColumn]], '\0', childTable.columns[columnMap[fk.referenceColumn]].length);
                    }
                    else if (action == Table::ForeignKeyAction::SET_DEFAULT) {
                        std::string defaultValue = childTable.columns[columnMap[fk.referenceColumn]].defaultValue;
                        std::memcpy(rowBuffer.data() + fieldOffsets[columnMap[fk.referenceColumn]], defaultValue.c_str(), childTable.columns[columnMap[fk.referenceColumn]].length);
                    }
                }

                outFile.write(rowBuffer.data(), rowWidth);
            }

            inFile.close();
            outFile.close();

            if (action == Table::ForeignKeyAction::CASCADE && foreignKeyFound) {
                fs::remove(dataFilePath);
                fs::rename(tempPath, dataFilePath);
            }
            else if (foreignKeyFound && (action == Table::ForeignKeyAction::SET_NULL || action == Table::ForeignKeyAction::SET_DEFAULT)) {
                fs::remove(dataFilePath);
                fs::rename(tempPath, dataFilePath);
            }
            else {
                fs::remove(tempPath);
            }
        }
    }
    return true;
}

std::vector<std::string>TableManager::readColumnData(const std::string& dbName, const std::string& tableName, const std::string& columnName) {
    fs::path dataFilePath = fs::current_path() / dbName / tableName / (tableName + ".trd");
    std::vector<std::string> columnData;

    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return columnData;
    }

    std::ifstream dataFile(dataFilePath, std::ios::binary);
    if (!dataFile) {
        std::cerr << "Failed to open data file for reading." << std::endl;
        return columnData;
    }

    int rowWidth = 0;
    std::vector<int> fieldOffsets;
    for (const auto& col : table.columns) {
        fieldOffsets.push_back(rowWidth);
        rowWidth += col.length;
    }

    std::vector<char> rowBuffer(rowWidth);
    std::string fieldValue;

    std::map<std::string, int> columnMap;
    for (int i = 0; i < table.columns.size(); ++i) {
        columnMap[table.columns[i].name] = i;
    }

    if (columnMap.find(columnName) == columnMap.end()) {
        std::cerr << "Column not found." << std::endl;
        return columnData;
    }

    int columnIndex = columnMap[columnName];
    const auto& colType = table.columns[columnIndex].type;

    while (dataFile.read(rowBuffer.data(), rowWidth)) {
        if (colType == "integer") {
            // Assumes integer is stored in binary format
            int intValue;
            memcpy(&intValue, rowBuffer.data() + fieldOffsets[columnIndex], sizeof(int));
            columnData.push_back(std::to_string(intValue));
        } else if (colType == "number") {
            // Assumes number is stored as a floating point number
            float floatValue;
            memcpy(&floatValue, rowBuffer.data() + fieldOffsets[columnIndex], sizeof(float));
            columnData.push_back(std::to_string(floatValue));
        } else {
            // Default case: treat as string
            fieldValue.assign(rowBuffer.data() + fieldOffsets[columnIndex], table.columns[columnIndex].length);
            fieldValue.erase(std::remove(fieldValue.begin(), fieldValue.end(), '\0'), fieldValue.end());
            columnData.push_back(fieldValue);
        }
    }

    dataFile.close();
    return columnData;
}

void TableManager::changeColumn(const std::string& dbName,
                                const std::string& tableName,
                                const std::string& oldName,
                                const std::string& newName,
                                const std::string& newType,
                                int newLength, bool newIsPrimaryKey, bool newIsNullable,
                                const std::string& newDefaultValue) {
    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    auto it = std::find_if(table.columns.begin(), table.columns.end(), [&](const Table::Column& col) { return col.name == oldName; });
    if (it == table.columns.end()) {
        std::cerr << "Column not found" << std::endl;
        return;
    }

    it->name = newName;
    it->type = newType;
    it->length = newLength;
    it->isPrimaryKey = newIsPrimaryKey;
    it->isNullable = newIsNullable;
    it->defaultValue = newDefaultValue;

    // Prepare paths for the original and temporary schema files
    fs::path tableDirPath = fs::current_path() / dbName / tableName;
    fs::path tempSchemaFilePath = tableDirPath / (tableName + "_temp.tdf");
    fs::path schemaFilePath = tableDirPath / (tableName + ".tdf");

    // Open the temporary schema file for writing
    std::ofstream schemaFile(tempSchemaFilePath, std::ios::binary);
    if (!schemaFile) {
        std::cerr << "Failed to open temporary schema file for writing." << std::endl;
        return;
    }

    // Write the updated table schema to the temporary file
    table.writeToDisk(schemaFile);
    schemaFile.close();

    // Check for file write errors
    if (!schemaFile.good()) {
        fs::remove(tempSchemaFilePath);  // Cleanup temporary file on failure
        std::cerr << "Failed to write to the temporary schema file correctly." << std::endl;
        return;
    }

    // Replace the original schema file with the updated temporary file
    try {
        fs::rename(tempSchemaFilePath, schemaFilePath);
        std::cout << "Table schema successfully updated." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to replace the old schema file: " << e.what() << std::endl;
    }
}

void TableManager::modifyColumn(const std::string& dbName, const std::string& tableName, const std::string& name, const std::string& newType, int newLength, bool newIsPrimaryKey, bool newIsNullable, const std::string& newDefaultValue) {
    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema." << std::endl;
        return;
    }

    auto it = std::find_if(table.columns.begin(), table.columns.end(), [&](const Table::Column& col) { return col.name == name; });
    if (it == table.columns.end()) {
        std::cerr << "Column not found" << std::endl;
        return;
    }

    it->type = newType;
    it->length = newLength;
    it->isPrimaryKey = newIsPrimaryKey; // 修改主键属性
    it->isNullable = newIsNullable;
    it->defaultValue = newDefaultValue;

    // 准备临时和原始模式文件的路径
    fs::path tableDirPath = fs::current_path() / dbName / tableName;
    fs::path tempSchemaFilePath = tableDirPath / (tableName + "_temp.tdf");
    fs::path schemaFilePath = tableDirPath / (tableName + ".tdf");

    // 打开临时模式文件进行写入
    std::ofstream schemaFile(tempSchemaFilePath, std::ios::binary);
    if (!schemaFile) {
        std::cerr << "Failed to open temporary schema file for writing." << std::endl;
        return;
    }

    // 将更新后的表模式写入临时文件
    table.writeToDisk(schemaFile);
    schemaFile.close();

    // 检查文件写入错误
    if (!schemaFile.good()) {
        fs::remove(tempSchemaFilePath); // 失败时清理临时文件
        std::cerr << "Failed to write to the temporary schema file correctly." << std::endl;
        return;
    }

    // 使用更新后的临时文件替换原始文件
    try {
        fs::rename(tempSchemaFilePath, schemaFilePath);
        std::cout << "Table schema successfully updated with new primary key settings." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to replace the old schema file: " << e.what() << std::endl;
    }
}


//void TableManager::printTree(struct sqlNode* node) {
//    if (node == NULL) {
//        return;
//    }
//
//    printf("Operation: %s\n", node->operation);
//
//    if (strcmp(node->operation, "create") == 0) {
//        struct createNode* createNode = (struct createNode*)node->subNodes;
//        printf("Create operation: %s\n", createNode->operation);
//
//        if (strcmp(createNode->operation, "createDatabase") == 0) {
//            struct createDatabaseNode* createDatabaseNode = (struct createDatabaseNode*)createNode->subNodes;
//            printf("Database name: %s\n", createDatabaseNode->databaseName);
//        } else if (strcmp(createNode->operation, "createTable") == 0) {
//            struct createTableNode* createTableNode = (struct createTableNode*)createNode->subNodes;
//            printf("Table name: %s\n", createTableNode->tableName);
//
//            printf("Attributes:\n");
//            struct attributeNode* attribute = createTableNode->attribute;
//            while (attribute != NULL) {
//                printf("  Field: %s, Type: %s, Constraint: %s\n", attribute->field, attribute->type, attribute->constraint);
//                attribute = attribute->next;
//            }
//        } else if (strcmp(createNode->operation, "createIndex") == 0) {
//            struct createIndexNode* createIndexNode = (struct createIndexNode*)createNode->subNodes;
//            printf("Index name: %s\n", createIndexNode->indexName);
//            printf("Table name: %s\n", createIndexNode->tableName);
//            printf("Fields: ");
//            for (int i = 0; i < createIndexNode->numFields; i++) {
//                printf("%s  ", createIndexNode->fields[i]);
//            }
//            printf("\n");
//        }
//    } else if (strcmp(node->operation, "select") == 0) {
//        struct selectNode* selectNode = (struct selectNode*)node->subNodes;
//        printf("Select operation: %s\n", selectNode->operation);
//
//        if (strcmp(selectNode->operation, "selectTable") == 0) {
//            struct selectTableNode* selectTableNode = (struct selectTableNode*)selectNode->subNodes;
//
//            printf("Fields: ");
//            for (int i = 0; i < selectTableNode->numFields; i++) {
//                printf("%s  ", selectTableNode->fields[i]);
//            }
//            printf("\n");
//
//            printf("Tables: ");
//            for (int i = 0; i < selectTableNode->numTables; i++) {
//                printf("%s  ", selectTableNode->tables[i]);
//            }
//            printf("\n");
//
//            printf("Conditions:\n");
//            struct conditionNode* condition = selectTableNode->conditions;
//            while (condition != NULL) {
//                printf("  Field: %s, Value: %s\n", condition->field, condition->value);
//                condition = condition->next;
//            }
//
//            printf("Order by: ");
//            for (int i = 0; i < selectTableNode->numOrderBy; i++) {
//                printf("%s  ", selectTableNode->orderBy[i]);
//            }
//            printf("\n");
//        } else if (strcmp(selectNode->operation, "selectDatabase") == 0) {
//            printf("Select Database\n");
//        }
//    } else if (strcmp(node->operation, "insert") == 0) {
//        struct insertNode* insertNode = (struct insertNode*)node->subNodes;
//        printf("Insert operation: %s\n", insertNode->operation);
//        if (strcmp(insertNode->operation, "insertAll") == 0) {
//            struct insertAllNode* insertAllNode = (struct insertAllNode*)insertNode->subNodes;
//
//            printf("Table name: %s\n", insertAllNode->tableName);
//
//            printf("Values: ");
//            for (int i = 0; i < insertAllNode->numValues; i++) {
//                printf("%s  ", insertAllNode->values[i]);
//            }
//            printf("\n");
//        } else if (strcmp(insertNode->operation, "insertAssign") == 0) {
//            struct insertAssignNode* insertAssignNode = (struct insertAssignNode*)insertNode->subNodes;
//
//            printf("Table name: %s\n", insertAssignNode->tableName);
//
//            printf("Fields: ");
//            for (int i = 0; i < insertAssignNode->numFields; i++) {
//                printf("%s  ", insertAssignNode->fields[i]);
//            }
//            printf("\n");
//
//            printf("Values: ");
//            for (int i = 0; i < insertAssignNode->numValues; i++) {
//                printf("%s  ", insertAssignNode->values[i]);
//            }
//            printf("\n");
//        }
//    } else if (strcmp(node->operation, "update") == 0) {
//        struct updateNode* updateNode = (struct updateNode*)node->subNodes;
//
//        printf("Table name: %s\n", updateNode->tableName);
//
//        printf("Field-Value pairs:\n");
//        struct conditionNode* fieldValue = updateNode->field_values;
//        while (fieldValue != NULL) {
//            printf("  Field: %s, Value: %s\n", fieldValue->field, fieldValue->value);
//            fieldValue = fieldValue->next;
//        }
//
//        printf("Conditions:\n");
//        struct conditionNode* condition = updateNode->conditions;
//        while (condition != NULL) {
//            printf("  Field: %s, Value: %s\n", condition->field, condition->value);
//            condition = condition->next;
//        }
//    } else if (strcmp(node->operation, "delete") == 0) {
//        struct deleteNode* deleteNode = (struct deleteNode*)node->subNodes;
//
//        printf("Table name: %s\n", deleteNode->tableName);
//
//        printf("Conditions:\n");
//        struct conditionNode* condition = deleteNode->conditions;
//        while (condition != NULL) {
//            printf("  Field: %s, Value: %s\n", condition->field, condition->value);
//            condition = condition->next;
//        }
//    } else if (strcmp(node->operation, "alter") == 0) {
//        struct alterNode* alterNode = (struct alterNode*)node->subNodes;
//        printf("Alter operation: %s\n", alterNode->operation);
//        printf("Table name: %s\n", alterNode->tableName);
//
//        if (strcmp(alterNode->operation, "add") == 0) {
//            struct alterAddNode* alterAddNode = (struct alterAddNode*)alterNode->subNodes;
//            printf("Add operation: %s\n", alterAddNode->operation);
//            if (strcmp(alterAddNode->operation, "alterAddAttribute") == 0) {
//                struct alterAddAttributeNode* addAttributeNode = (struct alterAddAttributeNode*)alterAddNode->subNodes;
//                printf("Attribute: Field: %s, Type: %s, Constraint: %s\n", addAttributeNode->field, addAttributeNode->type, addAttributeNode->constraint);
//            } else if (strcmp(alterAddNode->operation, "alterAddPrimary") == 0) {
//                struct alterAddPrimaryNode* addPrimaryNode = (struct alterAddPrimaryNode*)alterAddNode->subNodes;
//                printf("Adding primary key to fields: ");
//                for (int i = 0; i < addPrimaryNode->numFields; i++) {
//                    printf("%s  ", addPrimaryNode->fields[i]);
//                }
//                printf("\n");
//            } else if (strcmp(alterAddNode->operation, "alterAddForeign") == 0) {
//                struct alterAddForeignNode* addForeignNode = (struct alterAddForeignNode*)alterAddNode->subNodes;
//                printf("Adding foreign key: FK name: %s, PK table name: %s, Update action: %s, Delete action: %s\n",
//                       addForeignNode->fkName, addForeignNode->pkTablename, addForeignNode->updateAction, addForeignNode->deleteAction);
//                printf("FK fields: ");
//                for (int i = 0; i < addForeignNode->fkNumFields; i++) {
//                    printf("%s  ", addForeignNode->fkFields[i]);
//                }
//                printf("\nPK fields: ");
//                for (int i = 0; i < addForeignNode->pkNumFields; i++) {
//                    printf("%s  ", addForeignNode->pkFields[i]);
//                }
//                printf("\n");
//            }
//        } else if (strcmp(alterNode->operation, "drop") == 0) {
//            struct alterDropNode* alterDropNode = (struct alterDropNode*)alterNode->subNodes;
//            printf("Drop operation: %s\n", alterDropNode->operation);
//            if (strcmp(alterDropNode->operation, "alterDropAttribute") == 0) {
//                struct alterDropAttributeNode* dropAttributeNode = (struct alterDropAttributeNode*)alterDropNode->subNodes;
//                printf("Dropping attribute: %s\n", dropAttributeNode->field);
//            } else if (strcmp(alterDropNode->operation, "alterDropFK") == 0) {
//                struct alterDropFKNode* dropFKNode = (struct alterDropFKNode*)alterDropNode->subNodes;
//                printf("Dropping foreign key: %s\n", dropFKNode->fkName);
//            }
//        } else if (strcmp(alterNode->operation, "change") == 0) {
//            struct alterChangeNode* changeNode = (struct alterChangeNode*)alterNode->subNodes;
//            printf("Changing field from %s to %s, Type: %s, Constraint: %s\n",
//                   changeNode->oldField, changeNode->newField, changeNode->type, changeNode->constraint);
//        } else if (strcmp(alterNode->operation, "rename") == 0) {
//            struct alterRenameNode* renameNode = (struct alterRenameNode*)alterNode->subNodes;
//            printf("Renaming table to: %s\n", renameNode->newTableName);
//        } else if (strcmp(alterNode->operation, "modify") == 0) {
//            struct alterModifyNode* modifyNode = (struct alterModifyNode*)alterNode->subNodes;
//            printf("Modifying Field: %s, Type: %s, Constraint: %s\n", modifyNode->field, modifyNode->type, modifyNode->constraint);
//        }
//    } else if (strcmp(node->operation, "show") == 0) {
//        struct showNode* showNode = (struct showNode*)node->subNodes;
//        printf("Show operation: %s\n", showNode->operation);
//        if (strcmp(showNode->operation, "showDatabases") == 0) {
//            printf("Showing databases\n");
//        } else if (strcmp(showNode->operation, "showTables") == 0) {
//            printf("Showing tables\n");
//        } else if (strcmp(showNode->operation, "showCreateTable") == 0) {
//            struct showCreateTableNode* showCreateTableNode = (struct showCreateTableNode*)showNode->subNodes;
//            printf("Table name: %s\n", showCreateTableNode->tableName);
//        } else if (strcmp(showNode->operation, "showGrants") == 0) {
//            struct showGrantsNode* showGrantsNode = (struct showGrantsNode*)showNode->subNodes;
//            printf("Show grants for: %s\n", showGrantsNode->id);
//        } else if (strcmp(showNode->operation, "showIndex") == 0) {
//            struct showIndexNode* showIndexNode = (struct showIndexNode*)showNode->subNodes;
//            printf("Show index for table: %s\n", showIndexNode->tableName);
//        }
//    } else if (strcmp(node->operation, "truncate") == 0) {
//        struct truncateNode* truncateNode = (struct truncateNode*)node->subNodes;
//        printf("Truncating table: %s\n", truncateNode->tableName);
//    } else if (strcmp(node->operation, "desc") == 0) {
//        struct descNode* descNode = (struct descNode*)node->subNodes;
//        printf("Describing table: %s\n", descNode->tableName);
//    } else if (strcmp(node->operation, "drop") == 0) {
//        struct dropNode* dropNode = (struct dropNode*)node->subNodes;
//        printf("Drop operation: %s\n", dropNode->operation);
//        if (strcmp(dropNode->operation, "dropDatabase") == 0) {
//            struct dropDatabaseNode* dropDatabaseNode = (struct dropDatabaseNode*)dropNode->subNodes;
//            printf("Dropping database: %s\n", dropDatabaseNode->databaseName);
//        } else if (strcmp(dropNode->operation, "dropIndex") == 0) {
//            struct dropIndexNode* dropIndexNode = (struct dropIndexNode*)dropNode->subNodes;
//            printf("Dropping index: %s on table: %s\n", dropIndexNode->indexName, dropIndexNode->tableName);
//        }
//    } else if (strcmp(node->operation, "use") == 0) {
//        struct useNode* useNode = (struct useNode*)node->subNodes;
//        printf("Using database: %s\n", useNode->databaseName);
//    } else if (strcmp(node->operation, "grant") == 0) {
//        struct grantNode* grantNode = (struct grantNode*)node->subNodes;
//        printf("Granting: ");
//        for (int i = 0; i < grantNode->numGrants; i++) {
//            printf("%s  ", grantNode->grants[i]);
//        }
//        printf("\nOn database: %s, table: %s\n", grantNode->databaseName, grantNode->tableName);
//        printf("To users: ");
//        for (int i = 0; i < grantNode->numIds; i++) {
//            printf("%s  ", grantNode->ids[i]);
//        }
//        printf("\n");
//    } else if (strcmp(node->operation, "revoke") == 0) {
//        struct revokeNode* revokeNode = (struct revokeNode*)node->subNodes;
//        printf("Revoking: ");
//        for (int i = 0; i < revokeNode->numGrants; i++) {
//            printf("%s  ", revokeNode->grants[i]);
//        }
//        printf("\nFrom database: %s, table: %s\n", revokeNode->databaseName, revokeNode->tableName);
//        printf("From users: ");
//        for (int i = 0; i < revokeNode->numIds; i++) {
//            printf("%s  ", revokeNode->ids[i]);
//        }
//        printf("\n");
//    }
//}