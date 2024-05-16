#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "Table.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cassert>
#include <map>
#include <istream>
#include <set>
#include <iomanip>

namespace fs = std::filesystem;

class TableManager {
private:
    std::string currentDatabase; // 存储当前数据库名称

public:
    /**
     * 创建数据表
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param columnNames 字段名列表。
     * @param columnTypes 字段类型列表
     * @param columnLengths 类型长度列表
     * @param isPrimaryKeys 是否是主键
     * @param isNullables 是否为空
     * @param defaultValues 默认值
     * @throws None
     *
     * @author 韩玉龙
     */
    void createTable(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& columnNames, const std::vector<std::string>& columnTypes, const std::vector<int>& columnLengths, const std::vector<bool>& isPrimaryKeys, const std::vector<bool>& isNullables, const std::vector<std::string>& defaultValues, const std::vector<Table::ForeignKey>& foreignKeys, const std::string& createStatement);
    /**
     * 删除数据表
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @throws None
     *
     * @author 韩玉龙
     */
    void deleteTable(const std::string& dbName, const std::string& tableName);
    /**
     * 读取文件格式
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param table 要填充的虚拟表对象
     * @return 如果成功读取文件格式并填充到虚拟表对象，则返回 true；否则返回 false
     *
     * @author 韩玉龙
     */
    bool loadTableSchema(const std::string& dbName, const std::string& tableName, Table& table);
    /**
     * 插入数据表
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param recordData 插入数据（暂时只允许一行查询）
     * @throws None
     *
     * @author 韩玉龙
     */
    void insertRecord(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& recordData);
    /**
     * 读取整表
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @throws None
     *
     * @author 韩玉龙
     */
    void readTableData(const std::string& dbName, const std::string& tableName);
    /**
     * 自定义读取表
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param fieldNames 要读取的字段名列表
     * @param conditionColumn 条件字段
     * @param operation 比较符
     * @param conditionValue 比较值
     * @throws None
     *
     * @author 韩玉龙
     */
    void readRecords(const std::string& dbName, const std::string& tableName,
                     const std::vector<std::string>& fieldNames,
                     const std::vector<std::string>& conditionColumn,
                     const std::vector<std::string>& operation,
                     const std::vector<std::string>& conditionValue);
    /**
     * 自定义删除表
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param conditionColumn 条件字段
     * @param operation 比较符
     * @param conditionValue 比较值
     * @throws None
     *
     * @author 韩玉龙
     */
    void deleteRecords(const std::string& dbName, const std::string& tableName,
                       const std::vector<std::string>& conditionColumn,
                       const std::vector<std::string>& operation,
                       const std::vector<std::string>& conditionValue);
    /**
     * 条件判断
     *
     * @param fieldValue 被比较的字段
     * @param op 比较符
     * @param value 被比较的值
     * @throws None
     *
     * @author 韩玉龙
     */
    bool checkCondition(const std::string& fieldValue, const std::string& op, const std::string& value);
    /**
     * 判断是否为浮点数
     *
     * @param fieldValue 被比较的字符串
     * @throws None
     *
     * @author 韩玉龙
     */
    bool isNumber(const std::string& str);
    /**
     * 自定义更新表
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param conditionColumn 条件字段名
     * @param operation 比较符
     * @param conditionValue 比较值
     * @param updateColumn 更新字段名
     * @param updateValue 更新值
     * @throws None
     *
     * @author 鄂日启
     */
    void updateTable(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& conditionColumn, const std::vector<std::string>& operation, const std::vector<std::string>& conditionValue, const std::vector<std::string>& updateColumn, const std::vector<std::string>& updateValue);
    /**
     * 排序读取表
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param sortColumn 排序字段
     * @param orders 顺序
     * @param fieldNames 搜索字段
     * @throws None
     *
     * @author 韩玉龙
     */
    void orderByRecord(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& sortColumn, const std::vector<std::string>& orders, const std::vector<std::string>& fieldNames);
    /**
     * 顺序返回读取表数据
     *
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param table 虚拟表
     * @throws None
     *
     * @author 韩玉龙
     */
    std::vector<std::vector<std::string>> readSortTableData(const std::string& dbName, const std::string& tableName, const Table& table);
    /**
     * alter语句分支增加列
     *
     * @param dbName 数据库名称
     * @param tableName 表名
     * @param columnNames 增加字段名
     * @param columnTypes 增加字段类型
     * @param columnLengths 增加字段长度
     * @param isPrimaryKeys 增加字段是否为主键
     * @param isNullables 增加字段是否为空
     * @param defaultValues 增加字段默认值
     *
     * @throws None
     *
     * @author 鄂日启
     */
    void alter_addColumnToTable(const std::string& dbName,
                                const std::string& tableName,
                                const std::vector<std::string>& columnNames,
                                const std::vector<std::string>& columnTypes,
                                const std::vector<int>& columnLengths,
                                const std::vector<bool>& isPrimaryKeys,
                                const std::vector<bool>& isNullables,
                                const std::vector<std::string>& defaultValues);
    /**
     * alter语句分支删除列
     *
     * @param dbName 数据库名称
     * @param tableName 表名
     * @param columnsToDelete 删除字段名
     *
     * @throws None
     *
     * @author 鄂日启
     */
    void alter_deleteColumns(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& columnsToDelete);
    /**
     * 重命名表
     *
     * @param dbName 数据库名称
     * @param oldTableName 旧表名称
     * @param newTableName 新表名称
     * @throws None
     *
     * @author 韩玉龙
     */
    bool renameTable(const std::string& dbName, const std::string& oldTableName, const std::string& newTableName);
    /**
     * 查询所有表
     *
     * @throws None
     *
     * @author 韩玉龙
     */
    std::vector<std::string> listAllDatabases() const;
    /**
     * USE数据库
     *
     * @param dbName 数据库名
     * @throws None
     *
     * @author 韩玉龙
     */
    void useDatabase(const std::string& dbName);
    /**
     * 获取当前数据库名称
     *
     * @throws None
     *
     * @author 韩玉龙
     */
    std::string selectDatabase() const;
    /**
     * 显示当前数据库下的所有表名
     *
     * @throws None
     *
     * @author 韩玉龙
     */
    void showTables();
    /**
     * desc Table
     *
     * @param dbName 表名
     * @throws None
     *
     * @author 韩玉龙
     */
    void describeTable(const std::string& tableName);
    /**
     * 辅助函数：把外键约束转化为字符串
     *
     * @param action 外键约束
     * @throws None
     *
     * @author 韩玉龙
     */
    std::string foreignKeyActionToString(Table::ForeignKeyAction action);
    /**
     * 显示建表语句
     *
     * @param dbName 数据库名
     * @param tableName 表名
     * @throws None
     *
     * @author 韩玉龙
     */
    void showCreateTable(const std::string& dbName, const std::string& tableName);
    /**
     * 清除表中数据
     *
     * @param dbName 数据库名
     * @param tableName 表名
     * @throws None
     *
     * @author 韩玉龙
     */
    void truncateTable(const std::string& dbName, const std::string& tableName);
    /**
     * 多表查询
     *
     * @param dbName 数据库名
     * @param table1 表1名
     * @param table2 表2名
     * @param column1 连接字段在表1
     * @param column2 连接字段在表2
     * @param selectColumns 查询字段
     * @throws None
     *
     * @author 韩玉龙
     */
    void innerJoin(const std::string& dbName, const std::string& table1, const std::string& table2, const std::string& column1, const std::string& column2, const std::vector<std::string>& selectColumns);

    void alter_addForeignKey(const std::string& dbName, const std::string& tableName, const std::string& columnName, const std::string& referenceTable, const std::string& referenceColumn, Table::ForeignKeyAction onDelete, Table::ForeignKeyAction onUpdate);

    void alter_deleteForeignKey(const std::string& dbName, const std::string& tableName, const std::string& columnName);

    bool handleForeignKeyAction(const std::string& dbName, const Table& table, const std::string& columnName, const std::string& value, Table::ForeignKeyAction action);
    /**
     * 检查引用的表中是否存在对应的外键值
     *
     * @param dbName 数据库名
     * @param referenceTable 对应表
     * @param referenceColumn 对应字段
     * @param value 对应值
     * @throws None
     *
     * @author 韩玉龙
     */
    bool checkForeignKeyConstraint(const std::string& dbName, const std::string& referenceTable, const std::string& referenceColumn, const std::string& value, const std::string& columnType);
    /**
     * 单独读取某一列的数据
     *
     * @param dbName 数据库名称
     * @param tableName 表名
     * @param columnName 字段名
     *
     * @throws None
     *
     * @author 鄂日启
     */
    std::vector<std::string> readColumnData(const std::string& dbName, const std::string& tableName, const std::string& columnName);
    /**
     * modify函数
     *
     * @param dbName 数据库名称
     * @param tableName 表名
     * @param columnName 字段名
     * @param newType 新加的字段类型
     * @param newLength 新字段长度
     * @param newIsPrimaryKey 是否为主键
     * @param newIsNullable 是否为空
     * @param newDefaultValue 新的默认值
     * @throws filesystem_error
     *
     * @author 鄂日启
     */
    void modifyColumn(const std::string& dbName, const std::string& tableName, const std::string& columnName, const std::string& newType, int newLength, bool newIsPrimaryKey, bool newIsNullable, const std::string& newDefaultValue);
    /**
     * change函数
     *
     * @param dbName 数据库名称
     * @param tableName 表名
     * @param columnName 字段名
     * @param newType 新加的字段类型
     * @param newLength 新字段长度
     * @param newIsPrimaryKey 是否为主键
     * @param newIsNullable 是否为空
     * @param newDefaultValue 新的默认值
     * @throws filesystem_error
     *
     * @author 鄂日启
     */
    void changeColumn(const std::string& dbName, const std::string& tableName, const std::string& oldName, const std::string& newName, const std::string& newType, int newLength, bool newIsPrimaryKey, bool newIsNullable, const std::string& newDefaultValue);
    void printTree(struct sqlNode* node);
};
#endif // TABLE_MANAGER_H
