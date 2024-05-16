#ifndef RECORD_H
#define RECORD_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <filesystem>
#include <algorithm>

class Table {
public:
    struct Column {
        std::string name;
        std::string type;
        int length;
        bool isPrimaryKey;
        bool isNullable;
        std::string defaultValue;
    };

    std::vector<Column> columns;

    enum ForeignKeyAction {
        RESTRICT,
        NOACTION,
        CASCADE,
        SET_NULL,
        SET_DEFAULT
    };

    struct ForeignKey {
        std::string columnName; // ����ֶ�
        std::string referenceTable; // ��Ӧ��
        std::string referenceColumn; // ��Ӧ�ֶ�
        ForeignKeyAction onDelete;
        ForeignKeyAction onUpdate;
    };

    std::vector<ForeignKey> foreignKeys;
    /**
 * ��һ���ֶ�
 *
 * @param name �ֶ�����
 * @param type �ֶ�����
 * @param length ���ͳ���
 * @param isPrimaryKey �ֶ�����
 * @param isNullable �ֶ�����
 * @param defaultValue Ĭ��ֵ
 * @throws None
 *
 * @author ������ ������
 */
    void addColumn(const std::string& name, const std::string& type, int length, bool isPrimaryKey, bool isNullable, const std::string& defaultValue);
/**
 * ��һ���ֶ�
 *
 * @param name �ֶ�����
 * @throws None
 *
 * @author ������
 */
    void dropColumn(const std::string& name);
/**
 * д���ͷ
 *
 * @param outFile ����ļ�
 * @throws None
 *
 * @author ������
 */
    void writeToDisk(std::ofstream& outFile) const;
/**
 * ��һ�����
 *
 * @param columnName �ֶ�����
 * @param referenceTable ��Ӧ��
 * @param referenceColumn ��Ӧ�ֶ�
 * @param onDelete ɾ��ʱ���Լ��
 * @param onUpdate ����ʱ���Լ��
 * @throws None
 *
 * @author ������
 */
    void addForeignKey(const std::string& columnName, const std::string& referenceTable, const std::string& referenceColumn, ForeignKeyAction onDelete, ForeignKeyAction onUpdate);
/**
 * ɾ��һ�����
 *
 * @param columnName �ֶ�����
 * @throws None
 *
 * @author ������
 */
    void removeForeignKey(const std::string& columnName);
};

#endif // RECORD_H
