#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <iostream>
#include <filesystem>

class DatabaseManager {
public:
    /**
     * �������ݿ�
     *
     * @param dbName ���ݿ�����
     * @throws None
     *
     * @author ������
     */
    void createDatabase(const std::string& dbName);
    /**
     * ɾ�����ݿ�
     *
     * @param dbName ���ݿ�����
     * @throws None
     *
     * @author ������
     */
    void deleteDatabase(const std::string& dbName);
};

#endif // DATABASE_MANAGER_H
