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
     * �������ݱ�
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @param columnNames �ֶ����б���
     * @param columnTypes �ֶ������б�
     * @param columnLengths ���ͳ����б�
     * @param isPrimaryKeys �Ƿ�������
     * @param isNullables �Ƿ�Ϊ��
     * @param defaultValues Ĭ��ֵ
     * @throws None
     *
     * @author ������
     */
    void createTable(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& columnNames, const std::vector<std::string>& columnTypes, const std::vector<int>& columnLengths, const std::vector<bool>& isPrimaryKeys, const std::vector<bool>& isNullables, const std::vector<std::string>& defaultValues);
    /**
     * ɾ�����ݱ�
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @throws None
     *
     * @author ������
     */
    void deleteTable(const std::string& dbName, const std::string& tableName);
    /**
     * ��ȡ�ļ���ʽ
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @param table Ҫ�������������
     * @return ����ɹ���ȡ�ļ���ʽ����䵽����������򷵻� true�����򷵻� false
     *
     * @author ������
     */
    bool loadTableSchema(const std::string& dbName, const std::string& tableName, Table& table);
    /**
     * �������ݱ�
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @param recordData �������ݣ���ʱֻ����һ�в�ѯ��
     * @throws None
     *
     * @author ������
     */
    void insertRecord(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& recordData);
    /**
     * ��ȡ����
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @throws None
     *
     * @author ������
     */
    void readTableData(const std::string& dbName, const std::string& tableName);
    /**
     * �Զ����ȡ��
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @param fieldNames Ҫ��ȡ���ֶ����б�
     * @param conditionColumn �����ֶ�
     * @param operation �ȽϷ�
     * @param conditionValue �Ƚ�ֵ
     * @throws None
     *
     * @author ������
     */
    void readRecords(const std::string& dbName, const std::string& tableName,
                     const std::vector<std::string>& fieldNames,
                     const std::vector<std::string>& conditionColumn,
                     const std::vector<std::string>& operation,
                     const std::vector<std::string>& conditionValue);
    /**
     * �Զ���ɾ����
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @param conditionColumn �����ֶ�
     * @param operation �ȽϷ�
     * @param conditionValue �Ƚ�ֵ
     * @throws None
     *
     * @author ������
     */
    void deleteRecords(const std::string& dbName, const std::string& tableName,
                       const std::vector<std::string>& conditionColumn,
                       const std::vector<std::string>& operation,
                       const std::vector<std::string>& conditionValue);
    /**
     * �����ж�
     *
     * @param fieldValue ���Ƚϵ��ֶ�
     * @param op �ȽϷ�
     * @param value ���Ƚϵ�ֵ
     * @throws None
     *
     * @author ������
     */
    bool checkCondition(const std::string& fieldValue, const std::string& op, const std::string& value);
    /**
     * �ж��Ƿ�Ϊ������
     *
     * @param fieldValue ���Ƚϵ��ַ���
     * @throws None
     *
     * @author ������
     */
    bool isNumber(const std::string& str);
    /**
     * д��Լ������
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @param columnNames �ֶ���
     * @param isPrimaryKey �Ƿ�Ϊ����
     * @param isNullable �Ƿ�Ϊ��
     * @param defaultValues �ֶ�Ĭ��ֵ
     * @throws None
     *
     * @author ������
     */
    void updateTable(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& conditionColumn, const std::vector<std::string>& operation, const std::vector<std::string>& conditionValue, const std::vector<std::string>& updateColumn, const std::vector<std::string>& updateValue);
    /**
     * �����ȡ��
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @param sortColumn �����ֶ�
     * @param orders ˳��
     * @param fieldNames �����ֶ�
     * @throws None
     *
     * @author ������
     */
    void orderByRecord(const std::string& dbName, const std::string& tableName, const std::vector<std::string>& sortColumn, const std::vector<std::string>& orders, const std::vector<std::string>& fieldNames);
    /**
     * ˳�򷵻ض�ȡ������
     *
     * @param dbName ���ݿ�����
     * @param tableName ������
     * @param table �����
     * @throws None
     *
     * @author ������
     */
    std::vector<std::vector<std::string>> readSortTableData(const std::string& dbName, const std::string& tableName, const Table& table);
};

#endif // TABLE_MANAGER_H
