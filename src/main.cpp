#include "Entity/basic_function/DatabaseManager.h"
#include "Entity/basic_function/TableManager.h"
#include "Entity/basic_function/Table.h"
#include "Entity/basic_function/AggregationFunctions.h"
#include "Entity/basic_function/UserManager.h"

int main() {
    DatabaseManager dbManager; // 数据库管理器
    TableManager tableManager; // 表文件管理器

    std::string dbName = "TestDB";
    std::string table1 = "Employees";
    std::string table2 = "Departments";

    // 建库
    dbManager.createDatabase(dbName);

    // list all databases
    std::vector<std::string> listDB = tableManager.listAllDatabases();
    for (std::string DB : listDB) {
        std::cout << DB << std::endl;
    }

    // use database()
    tableManager.useDatabase(dbName);

    // 定义表头
    std::vector<std::string> empColumns = { "ID", "Name", "Age", "Sex", "DeptID" };
    std::vector<std::string> empTypes = { "integer", "str", "integer", "str", "integer" };
    std::vector<int> empLengths = { 4, 32, 4, 32, 4 };
    std::vector<bool> empPK = { true, false, false, false, false };
    std::vector<bool> empNull = { false, false, false, true, false };
    std::vector<std::string> empDefault = { "", "", "20", "", "" };
    std::vector<Table::ForeignKey> empFK = { { "DeptID", "Departments", "ID", Table::ForeignKeyAction::RESTRICT, Table::ForeignKeyAction::RESTRICT } };

    // 建表
    tableManager.createTable(dbName, table1, empColumns, empTypes, empLengths, empPK, empNull, empDefault, empFK, "");

    // 定义表头
    std::vector<std::string> deptColumns = { "ID", "DeptName" };
    std::vector<std::string> deptTypes = { "integer", "str" };
    std::vector<int> deptLengths = { 4, 32 };
    std::vector<bool> deptPK = { true, false };
    std::vector<bool> deptNull = { false, false };
    std::vector<std::string> deptDefault = { "", "" };

    // 建表
    tableManager.createTable(dbName, table2, deptColumns, deptTypes, deptLengths, deptPK, deptNull, deptDefault, {}, "");

    // show tables
    tableManager.showTables();

    // desc
    std::cout << std::endl;
    tableManager.describeTable(table1);
    std::cout << std::endl;
    tableManager.describeTable(table2);

    // 插入数据
    std::vector<std::string> deptRecord1 = { "1", "HR" };
    std::vector<std::string> deptRecord2 = { "2", "Engineering" };
    std::vector<std::string> deptRecord3 = { "3", "Boss" };
    tableManager.insertRecord(dbName, table2, deptRecord1);
    tableManager.insertRecord(dbName, table2, deptRecord2);
    tableManager.insertRecord(dbName, table2, deptRecord3);

    std::vector<std::string> record1 = { "1", "Alice", "30", "女", "1" };
    std::vector<std::string> record2 = { "2", "Bob", "25", "男", "2" };
    std::vector<std::string> record3 = { "3", "Green", "", "", "3" };
    tableManager.insertRecord(dbName, table1, record1);
    tableManager.insertRecord(dbName, table1, record2);
    tableManager.insertRecord(dbName, table1, record3);

    // 读取全表
    std::cout << std::endl;
    std::cout << "读取全表：";
    tableManager.readTableData(dbName, table1);
    tableManager.readTableData(dbName, table2);

    // 自定义读取表
    std::cout << std::endl;
    std::cout << "条件读取：";
    std::vector<std::string> record4 = { "Age" };
    std::vector<std::string> record5 = { "=" };
    std::vector<std::string> record6 = { "\x19" };
    std::vector<std::string> fieldNames = { "ID", "Name" };
    tableManager.readRecords(dbName, table1, fieldNames, record4, record5, record6);

    // 删除外键
    std::cout << std::endl;
    std::cout << "删除外键：";
    tableManager.alter_deleteForeignKey(dbName, table1, "DeptID");
    tableManager.describeTable(table1);

    // 添加外键
    std::cout << std::endl;
    std::cout << "添加外键：";
    tableManager.alter_addForeignKey(dbName, table1, "DeptID", "Departments", "ID", Table::ForeignKeyAction::NOACTION, Table::ForeignKeyAction::NOACTION);
    tableManager.describeTable(table1);

    // 多表
    std::cout << std::endl;
    std::cout << "多表查询：";
    std::vector<std::string> selectColumns = { "ID", "Name", "DeptName" };
    tableManager.innerJoin(dbName, table1, table2, "DeptID", "ID", selectColumns);

    // 排序读取
    std::cout << std::endl;
    std::cout << "排序读取：";
    tableManager.orderByRecord(dbName, table1, record4, { "ASC" }, fieldNames);

    // 更新后读取全表
    std::cout << std::endl;
    std::cout << "更新后读取全表：";
    std::vector<std::string> updateColumn = { "ID" };
    std::vector<std::string> updateValue = { "\x19" };
    tableManager.updateTable(dbName, table1, record4, record5, record6, updateColumn, updateValue);
    tableManager.readTableData(dbName, table1);

    // 自定义删表后读取全表
    std::cout << std::endl;
    std::cout << "自定义删表后读取全表：";
    tableManager.deleteRecords(dbName, table1, record4, record5, record6);
    tableManager.readTableData(dbName, table1);

    // 修改表名
    std::cout << std::endl;
    std::cout << "修改表名：";
    std::string newTableName = "Renametable";
    tableManager.renameTable(dbName, table1, newTableName);

    //增加列
    std::cout << std::endl;
    std::cout << "增加列：";
    std::vector<std::string> addNames = { "Company" };
    std::vector<std::string> addNames_types = { "str" };
    std::vector<int> addNames_lengths = { 32 };
    std::vector<bool> addNames_isPrimaryKeys = { false };
    std::vector<bool> addNames_isNullables = { true };
    std::vector<std::string> addNames_defaultValues = { "BJTU" };
    tableManager.alter_addColumnToTable(dbName, newTableName, addNames, addNames_types, addNames_lengths, addNames_isPrimaryKeys, addNames_isNullables, addNames_defaultValues);
    tableManager.readTableData(dbName, newTableName);

    //删除列
    std::cout << std::endl;
    std::cout << "删除列：";
    std::vector<std::string> dropNames = { "Company" };
    tableManager.alter_deleteColumns(dbName, newTableName, dropNames);
    tableManager.readTableData(dbName, newTableName);

//    //修改字段
//    tableManager.changeColumn(dbName, tableName, "Work", "Work", "str", 32, true, false, "null");

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
//    std::cout << "User1 has "<<permission_1<<" permission: " << userManager.checkPermission(user_name , dbName, tableName, permission_1) << std::endl;
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

    return 0;
}
