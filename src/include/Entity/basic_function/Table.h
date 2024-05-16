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
        std::string columnName; // 外键字段
        std::string referenceTable; // 对应表
        std::string referenceColumn; // 对应字段
        ForeignKeyAction onDelete;
        ForeignKeyAction onUpdate;
    };

    std::vector<ForeignKey> foreignKeys;
    /**
 * 加一个字段
 *
 * @param name 字段名称
 * @param type 字段类型
 * @param length 类型长度
 * @param isPrimaryKey 字段类型
 * @param isNullable 字段类型
 * @param defaultValue 默认值
 * @throws None
 *
 * @author 韩玉龙 鄂日启
 */
    void addColumn(const std::string& name, const std::string& type, int length, bool isPrimaryKey, bool isNullable, const std::string& defaultValue);
/**
 * 加一个字段
 *
 * @param name 字段名称
 * @throws None
 *
 * @author 鄂日启
 */
    void dropColumn(const std::string& name);
/**
 * 写入表头
 *
 * @param outFile 输出文件
 * @throws None
 *
 * @author 韩玉龙
 */
    void writeToDisk(std::ofstream& outFile) const;
/**
 * 加一个外键
 *
 * @param columnName 字段名称
 * @param referenceTable 对应表
 * @param referenceColumn 对应字段
 * @param onDelete 删除时外键约束
 * @param onUpdate 更新时外键约束
 * @throws None
 *
 * @author 韩玉龙
 */
    void addForeignKey(const std::string& columnName, const std::string& referenceTable, const std::string& referenceColumn, ForeignKeyAction onDelete, ForeignKeyAction onUpdate);
/**
 * 删除一个外键
 *
 * @param columnName 字段名称
 * @throws None
 *
 * @author 韩玉龙
 */
    void removeForeignKey(const std::string& columnName);
};

#endif // RECORD_H
