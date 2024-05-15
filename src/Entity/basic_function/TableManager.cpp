#include <set>
#include "Entity/basic_function/TableManager.h"

void TableManager::createTable(const std::string& dbName,
                                const std::string& tableName,
                                const std::vector<std::string>& columnNames,
                                const std::vector<std::string>& columnTypes,
                                const std::vector<int>& columnLengths,
                                const std::vector<bool>& isPrimaryKeys,
                                const std::vector<bool>& isNullables,
                                const std::vector<std::string>& defaultValues) {
    // �������ݿ�·��
    fs::path dbPath = fs::current_path() / dbName;
    fs::create_directories(dbPath); // ȷ�����ݿ�Ŀ¼����

    // �������ļ���·��
    fs::path tableDirPath = dbPath / tableName;
    fs::create_directories(tableDirPath);

    // ���������ļ���·��
    fs::path schemaFilePath = tableDirPath / (tableName + ".tdf");
    fs::path dataFilePath = tableDirPath / (tableName + ".trd");
    fs::path constraintFilePath = tableDirPath / (tableName + ".tid");

    // ������ͷ�����Լ��
    Table table;
    for (size_t i = 0; i < columnNames.size(); i++) {
        table.addColumn(columnNames[i], columnTypes[i], columnLengths[i], isPrimaryKeys[i], isNullables[i], defaultValues[i]);
    }

    // ����ṹ��Լ��д�뵽.tdf�ļ�
    std::ofstream schemaFile(schemaFilePath, std::ios::binary);
    table.writeToDisk(schemaFile);
    schemaFile.close();

    // ���������ļ���Լ���ļ�
    std::ofstream dataFile(dataFilePath, std::ios::binary);
    std::ofstream constraintFile(constraintFilePath, std::ios::binary);
    dataFile.close();
    constraintFile.close();
}

void TableManager::deleteTable(const std::string& dbName, const std::string& tableName) {
    // ������·��
    fs::path tableDirPath = fs::current_path() / dbName / tableName;

    // ����ɾ�����ļ��м��������ļ�
    std::error_code ec; // ʹ��error_code�����쳣
    if (!fs::remove_all(tableDirPath, ec)) {
        std::cerr << "Failed to delete table. Error: " << ec.message() << std::endl;
    }
    else {
        std::cout << "Table '" << tableName << "' deleted successfully." << std::endl;
    }
}

bool TableManager::loadTableSchema(const std::string& dbName, const std::string& tableName, Table& table) {
    // ������ṹ�ļ�·��
    fs::path schemaFilePath = fs::current_path() / dbName / tableName / (tableName + ".tdf");
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

    schemaFile.close();
    return true;
}

void TableManager::insertRecord(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& recordData) {
    // ������ṹ�ļ�·��
    fs::path schemaFilePath = fs::current_path() / dbName / tableName / (tableName + ".tdf");
    fs::path dataFilePath = fs::current_path() / dbName / tableName / (tableName + ".trd");

    Table table;
    if (!loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Error loading table schema. Insert operation aborted." << std::endl;
        return;
    }

    if (recordData.size() != table.columns.size()) {
        std::cerr << "Error: Record data does not match the number of table columns." << std::endl;
        return;
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

void TableManager::readTableData(const std::string& dbName, const std::string& tableName) {
    // ������ṹ�ļ�·���������ļ�·��
    fs::path schemaFilePath = fs::current_path() / dbName / tableName / (tableName + ".tdf");
    fs::path dataFilePath = fs::current_path() / dbName / tableName / (tableName + ".trd");

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

    // �����ͷ��������
    for (const auto& col : table.columns) {
        std::cout << col.name << "\t";
    }
    std::cout << std::endl;

    // ��ȡ����
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
    // ������ṹ�ļ�·���������ļ�·��
    fs::path schemaFilePath = fs::current_path() / dbName / tableName / (tableName + ".tdf");
    fs::path dataFilePath = fs::current_path() / dbName / tableName / (tableName + ".trd");

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

    // ���������Ϊ��ͷ
    std::cout << std::endl;
    for (const auto& fieldName : fieldNames) {
        std::cout << fieldName << "\t";
    }
    std::cout << std::endl;

    // ����������������ӳ��
    std::map<std::string, int> columnMap;
    for (int i = 0; i < table.columns.size(); ++i) {
        columnMap[table.columns[i].name] = i;
    }

    // ���ڿ���ʹ��columnMap����ȡ��ȷ������
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
                    }
                    if (col.type == "number") {
                        float value;
                        dataFile.read(reinterpret_cast<char*>(&value), sizeof(value));
                        std::cout << value << "\t";
                    }

                    std::cout << fieldValue << "\t";
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
    // ������ṹ�ļ�·���������ļ�·��
    fs::path dataFilePath = fs::current_path() / dbName / tableName / (tableName + ".trd");
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

        if (!matches) {
            outFile.write(rowBuffer.data(), rowWidth);
        }
    }

    inFile.close();
    outFile.close();

    fs::remove(dataFilePath);
    fs::rename(tempPath, dataFilePath);
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
    fs::path dataFilePath = fs::current_path() / dbName / tableName / (tableName + ".trd");
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
            for (size_t i = 0; i < updateColumn.size(); ++i) {

                if (columnMap.find(updateColumn[i]) != columnMap.end()) {
                    int updateIndex = columnMap[updateColumn[i]];
                    if (table.columns[updateIndex].isPrimaryKey) {
                        if (updateValue[i].empty()) {
                            std::cerr << "Attempt to set primary key '" << updateColumn[i] << "' to an empty value denied." << std::endl;
                            continue; // Skip this update, proceed with other updates
                        }
                    }
                    std::string newValue = updateValue[i];
                    newValue.resize(table.columns[updateIndex].length, '\0');
                    std::memcpy(rowBuffer.data() + fieldOffsets[updateIndex], newValue.data(), newValue.size());
                }
            }
        }

        outFile.write(rowBuffer.data(), rowWidth);
    }

    inFile.close();
    outFile.close();

    fs::remove(dataFilePath);
    fs::rename(tempPath, dataFilePath);
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
    fs::path dataFilePath = fs::current_path() / dbName / tableName / (tableName + ".trd");
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


//alter语句功能函数
void TableManager::alter_addColumnToTable(const std::string& dbName,
                            const std::string& tableName,
                            const std::vector<std::string>& columnNames,
                            const std::vector<std::string>& columnTypes,
                            const std::vector<int>& columnLengths,
                            const std::vector<bool>& isPrimaryKeys,
                            const std::vector<bool>& isNullables,
                            const std::vector<std::string>& defaultValues) {
    fs::path tableDirPath = fs::current_path() / dbName / tableName;
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
    fs::path tableDirPath = fs::current_path() / dbName / tableName;
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

// 读取列数据
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
