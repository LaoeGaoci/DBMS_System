#include "Entity/basic_function/DatabaseManager.h"
#include "Entity/basic_function/TableManager.h"
#include "Entity/basic_function/Table.h"
#include "Entity/basic_function/AggregationFunctions.h"
#include "Entity/basic_function/UserManager.h"
void showPrimaryKeys(const std::string& dbName, const std::string& tableName) {
    TableManager dbManager;  // Assume DatabaseManager has necessary methods
    Table table;

    // Load the table schema
    if (!dbManager.loadTableSchema(dbName, tableName, table)) {
        std::cerr << "Failed to load table schema for " << tableName << " in database " << dbName << "." << std::endl;
        return;
    }

    // Display all primary keys
    std::cout << "Primary keys in table '" << tableName << "':" << std::endl;
    bool foundPrimaryKey = false;
    for (const auto& column : table.columns) {
        if (column.isPrimaryKey) {
            foundPrimaryKey = true;
            std::cout << "Column Name: " << column.name << "\n"
                        << "  Type: " << column.type << "\n"
                        << "  Length: " << column.length << "\n"
                        << "  Is Nullable: " << (column.isNullable ? "Yes" : "No") << "\n"
                        << "  Default Value: " << column.defaultValue << std::endl;
        }
    }

    if (!foundPrimaryKey) {
        std::cout << "No primary keys found in table '" << tableName << "'." << std::endl;
    }
}
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
    std::vector<std::string> record3 = { "3", "Green", "35", "" };

    tableManager.insertRecord(dbName, tableName, record1);
    tableManager.insertRecord(dbName, tableName, record2);
    tableManager.insertRecord(dbName, tableName, record3);

    // 读取全表
    std::cout << "读取全表：" << std::endl;
    tableManager.readTableData(dbName, tableName);
    std::cout << "*****************************" << std::endl;
    tableManager.changeColumn(dbName, tableName, "Work","Work","str",32,true,false,"null");
    showPrimaryKeys(dbName, tableName);
    //    tableManager.readTableData(dbName, tableName);
    //    std::cout << "*****************************" << std::endl;
//    //权限设置
//    UserManager userManager(dbName);
//    std::string user_name = "ERQ";
//    std::string user_pass = "116116116";
//    std::string permission_1 = "insert";
//    std::string permission_2 = "delete";
//    userManager.addUser(user_name, user_pass);
//    userManager.grantPermission(user_name , dbName, tableName, permission_1);
//    userManager.grantPermission(user_name , dbName, tableName, permission_2);
//    userManager.listUserPermissions(user_name , dbName, tableName);
    //std::cout << "User1 has "<<permission_1<<" permission: " << userManager.checkPermission(user_name , dbName, tableName, permission_1) << std::endl;

//    userManager.revokePermission(user_name, dbName, tableName, permission_1);
//    std::cout << "User1 has "<<permission_1<< "permission after revoke: " << userManager.checkPermission(user_name , dbName, tableName, permission_1) << std::endl;
//    //聚组函数
//    std::vector<std::string> columnData = tableManager.readColumnData(dbName, tableName, "Name");
//    //判断是否存在*
//    bool have_all = false;
//    std::cout << "Average of age: " << AggregationFunctions::average(columnData) << std::endl;
//    std::cout << "Sum of age: " << AggregationFunctions::sum(columnData) << std::endl;
//    std::cout << "Max of age: " << AggregationFunctions::max(columnData) << std::endl;
//    std::cout << "Min of age: " << AggregationFunctions::min(columnData) << std::endl;
//    std::cout << "Min of age: " << AggregationFunctions::count(columnData,have_all) << std::endl;
//    // 自定义读取表
//    std::cout << "条件读取：" << std::endl;
//    std::vector<std::string> record4 = { "Age" };
//    std::vector<std::string> record5 = { "=" };
//    std::vector<std::string> record6 = { "\x19" };
//    std::vector<std::string> fieldNames = { "ID", "Name" };
//    tableManager.readRecords(dbName, tableName, fieldNames, record4, record5, record6);

//    // 自定义删表
//    std::cout << "删除后读取全表：" << std::endl;
//    tableManager.deleteRecords(dbName, tableName, record4, record5, record6);
//    tableManager.readTableData(dbName, tableName);
    //自定义更新表
//    std::cout << "更新后读取全表：" << std::endl;
//    std::vector<std::string> updateColumn = { "ID" };
//    std::vector<std::string> updateValue = { "" };
//
//    tableManager.updateTable(dbName, tableName, record4, record5, record6,updateColumn,updateValue);
//    tableManager.readTableData(dbName, tableName);
//    // 排序读取
//    std::cout << "排序读取：" << std::endl;
//    tableManager.orderByRecord(dbName, tableName, record4, {"ASC"}, fieldNames);
//    //增加列
//    printf("*****************************\n");
//    std::vector<std::string> addNames = { "Company" };
//    std::vector<std::string> addNames_types = { "str" };
//    std::vector<int> addNames_lengths = { 32 };
//    std::vector<bool> addNames_isPrimaryKeys = {false};
//    std::vector<bool> addNames_isNullables = {true};
//    std::vector<std::string> addNames_defaultValues = {"BJTU"};
//    tableManager.alter_addColumnToTable(dbName, tableName,addNames,addNames_types,addNames_lengths,addNames_isPrimaryKeys,addNames_isNullables,addNames_defaultValues);
//    tableManager.readTableData(dbName, tableName);
    //删除列
//    printf("*****************************\n");
//    std::vector<std::string> dropNames = { "Work" };
//    tableManager.alter_deleteColumns(dbName, tableName,dropNames);
//    tableManager.readTableData(dbName, tableName);
    return 0;
}
