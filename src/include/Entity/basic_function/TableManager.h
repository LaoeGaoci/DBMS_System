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

namespace fs = std::filesystem;

class TableManager {
public:
    /**
     * ?????????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @param columnNames ??????б???
     * @param columnTypes ????????б?
     * @param columnLengths ????????б?
     * @param isPrimaryKeys ?????????
     * @param isNullables ??????
     * @param defaultValues ????
     * @throws None
     *
     * @author ??????
     */
    void
    createTable(const std::string &dbName, const std::string &tableName, const std::vector<std::string> &columnNames,
                const std::vector<std::string> &columnTypes, const std::vector<int> &columnLengths,
                const std::vector<bool> &isPrimaryKeys, const std::vector<bool> &isNullables,
                const std::vector<std::string> &defaultValues);

    /**
     * ????????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @throws None
     *
     * @author ??????
     */
    void deleteTable(const std::string &dbName, const std::string &tableName);

    /**
     * ?????????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @param table ??????????????
     * @return ???????????????????????????????? true??????? false
     *
     * @author ??????
     */
    bool loadTableSchema(const std::string &dbName, const std::string &tableName, Table &table);

    /**
     * ?????????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @param recordData ???????????????????в????
     * @throws None
     *
     * @author ??????
     */
    void
    insertRecord(const std::string &dbName, const std::string &tableName, const std::vector<std::string> &recordData);

    /**
     * ???????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @throws None
     *
     * @author ??????
     */
    void readTableData(const std::string &dbName, const std::string &tableName);

    /**
     * ?????????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @param fieldNames ????????????б?
     * @param conditionColumn ???????
     * @param operation ????
     * @param conditionValue ????
     * @throws None
     *
     * @author ??????
     */
    void readRecords(const std::string &dbName, const std::string &tableName,
                     const std::vector<std::string> &fieldNames,
                     const std::vector<std::string> &conditionColumn,
                     const std::vector<std::string> &operation,
                     const std::vector<std::string> &conditionValue);

    /**
     * ??????????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @param conditionColumn ???????
     * @param operation ????
     * @param conditionValue ????
     * @throws None
     *
     * @author ??????
     */
    void deleteRecords(const std::string &dbName, const std::string &tableName,
                       const std::vector<std::string> &conditionColumn,
                       const std::vector<std::string> &operation,
                       const std::vector<std::string> &conditionValue);

    /**
     * ?????ж?
     *
     * @param fieldValue ?????????
     * @param op ????
     * @param value ???????
     * @throws None
     *
     * @author ??????
     */
    bool checkCondition(const std::string &fieldValue, const std::string &op, const std::string &value);

    /**
     * ?ж???????????
     *
     * @param fieldValue ???????????
     * @throws None
     *
     * @author ??????
     */
    bool isNumber(const std::string &str);

    /**
     * д?????????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @param columnNames ?????
     * @param isPrimaryKey ????????
     * @param isNullable ??????
     * @param defaultValues ???????
     * @throws None
     *
     * @author ??????
     */
    void updateTable(const std::string &dbName, const std::string &tableName,
                     const std::vector<std::string> &conditionColumn, const std::vector<std::string> &operation,
                     const std::vector<std::string> &conditionValue, const std::vector<std::string> &updateColumn,
                     const std::vector<std::string> &updateValue);

    /**
     * ????????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @param sortColumn ???????
     * @param orders ???
     * @param fieldNames ???????
     * @throws None
     *
     * @author ??????
     */
    void orderByRecord(const std::string &dbName, const std::string &tableName, const std::vector<std::string> &sortColumn,
                  const std::vector<std::string> &orders, const std::vector<std::string> &fieldNames);

    /**
     * ????????????
     *
     * @param dbName ?????????
     * @param tableName ??????
     * @param table ?????
     * @throws None
     *
     * @author ??????
     */
    std::vector<std::vector<std::string>>
    readSortTableData(const std::string &dbName, const std::string &tableName, const Table &table);

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
    void alter_addColumnToTable(const std::string &dbName,
                                const std::string &tableName,
                                const std::vector<std::string> &columnNames,
                                const std::vector<std::string> &columnTypes,
                                const std::vector<int> &columnLengths,
                                const std::vector<bool> &isPrimaryKeys,
                                const std::vector<bool> &isNullables,
                                const std::vector<std::string> &defaultValues);

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
    void alter_deleteColumns(const std::string& dbName,
                                        const std::string& tableName,
                                        const std::vector<std::string>& columnsToDelete);

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
    std::vector<std::string> readColumnData(const std::string& dbName,
                                                            const std::string& tableName,
                                                            const std::string& columnName);
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
    void modifyColumn(const std::string& dbName,
                                    const std::string& tableName,
                                    const std::string& columnName,
                                    const std::string& newType,
                                    int newLength,
                                    bool newIsPrimaryKey,
                                    bool newIsNullable,
                                    const std::string& newDefaultValue);
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

    void changeColumn(const std::string& dbName,
                                    const std::string& tableName,
                                    const std::string& oldName,
                                    const std::string& newName,
                                    const std::string& newType,
                                    int newLength,
                                    bool newIsPrimaryKey,
                                    bool newIsNullable,
                                    const std::string& newDefaultValue);
};
#endif // TABLE_MANAGER_H
