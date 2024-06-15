#include "Entity/basic_function/DatabaseManager.h"

namespace fs = std::filesystem;

void DatabaseManager::createDatabase(const std::string& dbName) {
    fs::path dbPath = fs::current_path() / "DB" / dbName;
    if (!fs::create_directory(dbPath)) {
        std::cerr << "Database creation failed or already exists." << std::endl;
    }
    else {
        std::cout << "Database created successfully." << std::endl;
    }
}

void DatabaseManager::deleteDatabase(const std::string& dbName) {
    fs::path dbPath = fs::current_path() / "DB" / dbName;
    if (fs::remove_all(dbPath) != 0u) {
        std::cout << "Database deleted successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to delete database." << std::endl;
    }
}
