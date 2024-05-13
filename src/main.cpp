#include "Entity/basic_function/DatabaseManager.h"
#include "Entity/basic_function/TableManager.h"
#include "Entity/basic_function/Table.h"

int main() {
    DatabaseManager dbManager; // 数据库管理器
    TableManager tableManager; // 表文件管理器

    std::string dbName = "TestDB";
    std::string tableName = "Employees";

    // 建库
    dbManager.createDatabase(dbName);

    // 定义表头
    std::vector<std::string> columnNames = { "ID", "Name", "Age", "Work" }; // 定义字段名
    std::vector<std::string> columnTypes = { "integer", "str", "integer", "str" }; // 定义字段类型
    std::vector<int> columnLengths = { 4, 32, 4, 32 };  // 定义字段长度
    std::vector<bool> isPrimaryKeys = { true, false, false, false };
    std::vector<bool> isNullables = { false, false, false, true };
    std::vector<std::string> defaultValues = { "", "", "20", ""};

    // 建表
    tableManager.createTable(dbName, tableName, columnNames, columnTypes, columnLengths, isPrimaryKeys, isNullables, defaultValues);

    // 插入数据
    std::vector<std::string> record1 = { "1", "Alice", "30", "Boss" };
    std::vector<std::string> record2 = { "2", "Bob", "25", "Worker" };
    std::vector<std::string> record3 = { "3", "Green", "", "" };

    tableManager.insertRecord(dbName, tableName, record1);
    tableManager.insertRecord(dbName, tableName, record2);
    tableManager.insertRecord(dbName, tableName, record3);

    // 读取全表
    std::cout << "读取全表：" << std::endl;
    tableManager.readTableData(dbName, tableName);
    //更改表名

    // 自定义读取表
    std::cout << "条件读取：" << std::endl;
    std::vector<std::string> record4 = { "Age" };
    std::vector<std::string> record5 = { "=" };
    std::vector<std::string> record6 = { "\x19" };
    std::vector<std::string> fieldNames = { "ID", "Name" };
    tableManager.readRecords(dbName, tableName, fieldNames, record4, record5, record6);

//    // 自定义删表
//    std::cout << "删除后读取全表：" << std::endl;
//    tableManager.deleteRecords(dbName, tableName, record4, record5, record6);
//    tableManager.readTableData(dbName, tableName);
    //自定义更新表
    std::cout << "更新后读取全表：" << std::endl;
    std::vector<std::string> updateColumn = { "ID" };
    std::vector<std::string> updateValue = { "" };

    tableManager.updateTable(dbName, tableName, record4, record5, record6,updateColumn,updateValue);
    tableManager.readTableData(dbName, tableName);
    // 排序读取
    std::cout << "排序读取：" << std::endl;
    tableManager.orderByRecord(dbName, tableName, record4, {"ASC"}, fieldNames);
    //增加列
    printf("*****************************\n");
    std::vector<std::string> addNames = { "Company" };
    std::vector<std::string> addNames_types = { "str" };
    std::vector<int> addNames_lengths = { 32 };
    std::vector<bool> addNames_isPrimaryKeys = {false};
    std::vector<bool> addNames_isNullables = {true};
    std::vector<std::string> addNames_defaultValues = {"BJTU"};
    tableManager.alter_addColumnToTable(dbName, tableName,addNames,addNames_types,addNames_lengths,addNames_isPrimaryKeys,addNames_isNullables,addNames_defaultValues);
    tableManager.readTableData(dbName, tableName);
    //删除列
    printf("*****************************\n");
    std::vector<std::string> dropNames = { "Company" };
    tableManager.alter_deleteColumns(dbName, tableName,dropNames);
    tableManager.readTableData(dbName, tableName);
    return 0;
}
