//
// Created by ERQ on 2024/5/14.
//

#ifndef DBMS_USERMANAGER_H
#define DBMS_USERMANAGER_H
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>

class UserManager {
private:
    struct Permissions {
        std::map<std::string, std::set<std::string>> tablePermissions;
    };

    struct User {
        std::string username;
        std::string password; // 添加密码字段
        std::map<std::string, Permissions> dbPermissions;
        bool isAdmin;
    };

    std::map<std::string, User> users;
    std::string directoryPath;  // 储存目录
    std::string filePath;

public:
    UserManager(const std::string& tableName) : directoryPath("cmake-build-debug/" + tableName), filePath(directoryPath + "/user.tu") {
        // 初始化时创建管理员账户，并设置密码
        addUser("admin", "admin123", true);
        // 为管理员账户授予所有权限
        std::vector<std::string> allPermissions = {"insert", "update", "create", "delete", "alter"};
        for (auto& perm : allPermissions) {
            grantPermission("admin", "*", "*", perm);  // '*' 表示所有数据库和表
        }
        std::filesystem::path dir(directoryPath);
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);  // 创建这个目录如果不存在
        }
        loadFromFile(filePath); // 从表里读取数据
    }

    ~UserManager() {
        saveToFile("user.tu"); // 保存用户数据
    }

    bool addUser(const std::string& username, const std::string& password, bool isAdmin = false) {
        if (users.find(username) != users.end()) {
            std::cerr << "User already exists." << std::endl;
            return false;
        }
        users[username] = User{username, password, {}, isAdmin};
        return true;
    }

    bool setPassword(const std::string& username, const std::string& newPassword) {
        auto it = users.find(username);
        if (it == users.end()) {
            std::cerr << "User does not exist." << std::endl;
            return false;
        }
        it->second.password = newPassword;
        return true;
    }

    bool validatePassword(const std::string& username, const std::string& password) {
        auto it = users.find(username);
        if (it == users.end()) {
            std::cerr << "User does not exist." << std::endl;
            return false;
        }
        return it->second.password == password;
    }

    bool grantPermission(const std::string& username, const std::string& dbName, const std::string& tableName, const std::string& permission) {
        if (!validatePermission(permission)) {
            std::cerr << "Invalid permission: " << permission << std::endl;
            return false;
        }
        if (users.find(username) == users.end()) {
            std::cerr << "User does not exist." << std::endl;
            return false;
        }
        users[username].dbPermissions[dbName].tablePermissions[tableName].insert(permission);
        return true;
    }

    bool revokePermission(const std::string& username, const std::string& dbName,const std::string& tableName, const std::string& permission) {
        auto userIt = users.find(username);
        if (userIt == users.end()) {
            std::cerr << "User does not exist." << std::endl;
            return false;
        }
        auto& dbPerms = userIt->second.dbPermissions;
        auto dbIt = dbPerms.find(dbName);
        if (dbIt == dbPerms.end()) {
            return false;
        }
        auto& tablePerms = dbIt->second.tablePermissions;
        auto tableIt = tablePerms.find(tableName);
        if (tableIt == tablePerms.end()) {
            return false;
        }
        tableIt->second.erase(permission);
        return true;
    }

    bool checkPermission(const std::string& username, const std::string& dbName, const std::string& tableName, const std::string& permission) {
        auto it = users.find(username);
        if (it == users.end()) {
            std::cerr << "User does not exist." << std::endl;
            return false;
        }
        const auto& dbPerms = it->second.dbPermissions;
        auto dbIt = dbPerms.find(dbName);
        if (dbIt == dbPerms.end()) {
            return false;
        }
        const auto& tablePerms = dbIt->second.tablePermissions;
        auto tableIt = tablePerms.find(tableName);
        if (tableIt == tablePerms.end()) {
            return false;
        }
        return tableIt->second.find(permission) != tableIt->second.end();
    }

    void saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        for (const auto& [username, user] : users) {
            file << username << "," << user.password << "," << user.isAdmin << "\n";
        }
        file.close();
    }

    void loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;
        while (getline(file, line)) {
            std::istringstream iss(line);
            std::string username, password;
            bool isAdmin;
            char delim;
            if (std::getline(iss, username, ',') && std::getline(iss, password, ',') && iss >> isAdmin) {
                addUser(username, password, isAdmin);
            }
        }
        file.close();
    }

    void listUserPermissions(const std::string& username, const std::string& dbName, const std::string& tableName) {
        auto it = users.find(username);
        if (it == users.end()) {
            std::cerr << "User does not exist." << std::endl;
            return;
        }
        auto dbIt = it->second.dbPermissions.find(dbName);
        if (dbIt == it->second.dbPermissions.end()) {
            std::cerr << "No permissions set for database: " << dbName << std::endl;
            return;
        }
        auto tableIt = dbIt->second.tablePermissions.find(tableName);
        if (tableIt == dbIt->second.tablePermissions.end()) {
            std::cerr << "No permissions set for table: " << tableName << std::endl;
            return;
        }
        std::cout << "Permissions for " << username << " on " << dbName << "." << tableName << ": ";
        for (const auto& perm : tableIt->second) {
            std::cout << perm << " ";
        }
        std::cout << std::endl;
    }

private:
    bool validatePermission(const std::string& permission) {
        static const std::set<std::string> validPermissions = {"insert", "update", "create", "delete", "alter"};
        return validPermissions.find(permission) != validPermissions.end();
    }

};

#endif //DBMS_USERMANAGER_H
