#include "Entity/basic_function/DatabaseManager.h"

namespace fs = std::filesystem;

void DatabaseManager::createDatabase(const std::string& dbName) {
    fs::path dbPath = fs::current_path() / "DB" / dbName;
    fs::path recoverPath = fs::current_path() / "Recover" / dbName;// ���ݺͻָ������ݿ�·��

    if (!fs::create_directory(dbPath)) {
        std::cerr << "Database creation failed or already exists." << std::endl;
    }
    else {
        std::cout << "Database created successfully." << std::endl;
    }

    // �������ڻָ������ݿ��ļ���
    if (!fs::create_directory(recoverPath)) {
        std::cerr << "Failed to create recovery directory for the database." << std::endl;
    }
    else {
        std::cout << "Recovery directory created successfully for the database." << std::endl;
    }
}

void DatabaseManager::deleteDatabase(const std::string& dbName) {
    fs::path dbPath = fs::current_path() / dbName;
    if (fs::remove_all(dbPath)) {
        std::cout << "Database deleted successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to delete database." << std::endl;
    }
}
