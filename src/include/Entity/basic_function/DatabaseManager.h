#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <iostream>
#include <filesystem>

class DatabaseManager {
public:
    /**
     * 创建数据库
     *
     * @param dbName 数据库名称
     * @throws None
     *
     * @author 韩玉龙
     */
    void createDatabase(const std::string& dbName);
    /**
     * 删除数据库
     *
     * @param dbName 数据库名称
     * @throws None
     *
     * @author 韩玉龙
     */
    void deleteDatabase(const std::string& dbName);
};

#endif // DATABASE_MANAGER_H
