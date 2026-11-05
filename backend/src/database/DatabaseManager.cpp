#include "../../include/database/DatabaseManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

DatabaseManager::DatabaseManager(const std::string& dir, const std::string& uri)
    : dataDirectory(dir), mongoUri(uri), isConnected(false) {
    initialize();
}

DatabaseManager::~DatabaseManager() {
    // Flush to disk
    saveUsers();
    saveTransactions();
    saveBudgets();
    saveSavingsGoals();
}

std::string DatabaseManager::generateObjectId() const {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;

    auto timestamp = static_cast<uint32_t>(std::time(nullptr));
    uint64_t randomBits = dis(gen);

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << timestamp;
    ss << std::setw(16) << randomBits;
    return ss.str().substr(0, 24);
}

bool DatabaseManager::initialize() {
    std::lock_guard<std::mutex> lock(dbMutex);

    MKDIR("database");
    MKDIR(dataDirectory.c_str());

    loadAllCollections();

    // Check if initial seed is needed
    if (usersCollection.empty()) {
        std::cout << "[DatabaseManager] Initializing fresh MongoDB collection data..." << std::endl;
        seedInitialDemoData();
    }

    isConnected = true;
    return true;
}

bool DatabaseManager::connect() {
    return isConnected;
}

void DatabaseManager::loadAllCollections() {
    // Load Users
    {
        std::ifstream file(dataDirectory + "/users.json");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto parsed = crow::json::load(buffer.str());
            if (parsed.type == crow::json::Type::Array) {
                for (size_t i = 0; i < parsed.size(); ++i) {
                    const auto& item = parsed[i];
                    std::string id = item.has("id") ? item["id"].s() : (item.has("_id") ? item["_id"].s() : generateObjectId());
                    std::string name = item.has("name") ? item["name"].s() : "User";
                    std::string email = item.has("email") ? item["email"].s() : "";
                    std::string hash = item.has("passwordHash") ? item["passwordHash"].s() : "";
                    std::string salt = item.has("salt") ? item["salt"].s() : "fintrack_salt";
                    std::string cur = item.has("currency") ? item["currency"].s() : "INR";
                    std::string created = item.has("createdAt") ? item["createdAt"].s() : "";
                    if (!email.empty()) {
                        usersCollection.emplace_back(id, name, email, hash, salt, cur, created);
                    }
                }
            }
        }
    }

    // Load Transactions
    {
        std::ifstream file(dataDirectory + "/transactions.json");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto parsed = crow::json::load(buffer.str());
            if (parsed.type == crow::json::Type::Array) {
                for (size_t i = 0; i < parsed.size(); ++i) {
                    const auto& item = parsed[i];
                    std::string id = item.has("id") ? item["id"].s() : (item.has("_id") ? item["_id"].s() : generateObjectId());
                    std::string userId = item.has("userId") ? item["userId"].s() : "";
                    double amount = item.has("amount") ? item["amount"].d() : 0.0;
                    std::string category = item.has("category") ? item["category"].s() : "Other";
                    std::string desc = item.has("description") ? item["description"].s() : "";
                    std::string date = item.has("date") ? item["date"].s() : "";
                    std::string method = item.has("paymentMethod") ? item["paymentMethod"].s() : "UPI";
                    std::string type = item.has("type") ? item["type"].s() : "expense";
                    std::string created = item.has("createdAt") ? item["createdAt"].s() : "";

                    if (type == "income") {
                        std::string src = item.has("source") ? item["source"].s() : category;
                        bool recur = item.has("isRecurring") ? item["isRecurring"].b() : false;
                        transactionsCollection.push_back(std::make_shared<Income>(
                            id, userId, amount, category, desc, date, method, src, recur, created
                        ));
                    } else {
                        std::string merchant = item.has("merchant") ? item["merchant"].s() : "";
                        bool tax = item.has("isTaxDeductible") ? item["isTaxDeductible"].b() : false;
                        transactionsCollection.push_back(std::make_shared<Expense>(
                            id, userId, amount, category, desc, date, method, merchant, tax, created
                        ));
                    }
                }
            }
        }
    }

    // Load Budgets
    {
        std::ifstream file(dataDirectory + "/budgets.json");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto parsed = crow::json::load(buffer.str());
            if (parsed.type == crow::json::Type::Array) {
                for (size_t i = 0; i < parsed.size(); ++i) {
                    const auto& item = parsed[i];
                    std::string id = item.has("id") ? item["id"].s() : (item.has("_id") ? item["_id"].s() : generateObjectId());
                    std::string userId = item.has("userId") ? item["userId"].s() : "";
                    std::string category = item.has("category") ? item["category"].s() : "Other";
                    double limit = item.has("limit") ? item["limit"].d() : (item.has("limitAmount") ? item["limitAmount"].d() : 1000.0);
                    double spent = item.has("spent") ? item["spent"].d() : (item.has("spentAmount") ? item["spentAmount"].d() : 0.0);
                    int month = item.has("month") ? item["month"].i() : 11;
                    int year = item.has("year") ? item["year"].i() : 2026;
                    std::string created = item.has("createdAt") ? item["createdAt"].s() : "";
                    budgetsCollection.emplace_back(id, userId, category, limit, spent, month, year, created);
                }
            }
        }
    }

    // Load Savings Goals
    {
        std::ifstream file(dataDirectory + "/savings_goals.json");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto parsed = crow::json::load(buffer.str());
            if (parsed.type == crow::json::Type::Array) {
                for (size_t i = 0; i < parsed.size(); ++i) {
                    const auto& item = parsed[i];
                    std::string id = item.has("id") ? item["id"].s() : (item.has("_id") ? item["_id"].s() : generateObjectId());
                    std::string userId = item.has("userId") ? item["userId"].s() : "";
                    std::string name = item.has("name") ? item["name"].s() : "";
                    double target = item.has("targetAmount") ? item["targetAmount"].d() : 1000.0;
                    double curr = item.has("currentAmount") ? item["currentAmount"].d() : 0.0;
                    std::string date = item.has("targetDate") ? item["targetDate"].s() : "";
                    std::string cat = item.has("category") ? item["category"].s() : "General";
                    std::string created = item.has("createdAt") ? item["createdAt"].s() : "";
                    savingsGoalsCollection.emplace_back(id, userId, name, target, curr, date, cat, created);
                }
            }
        }
    }
}

void DatabaseManager::saveUsers() {
    std::ofstream file(dataDirectory + "/users.json");
    if (!file.is_open()) return;
    crow::json::wvalue arr;
    for (size_t i = 0; i < usersCollection.size(); ++i) {
        arr[i] = usersCollection[i].toJson(true);
    }
    file << arr.dump();
}

void DatabaseManager::saveTransactions() {
    std::ofstream file(dataDirectory + "/transactions.json");
    if (!file.is_open()) return;
    crow::json::wvalue arr;
    for (size_t i = 0; i < transactionsCollection.size(); ++i) {
        arr[i] = transactionsCollection[i]->toJson();
    }
    file << arr.dump();
}

void DatabaseManager::saveBudgets() {
    std::ofstream file(dataDirectory + "/budgets.json");
    if (!file.is_open()) return;
    crow::json::wvalue arr;
    for (size_t i = 0; i < budgetsCollection.size(); ++i) {
        arr[i] = budgetsCollection[i].toJson();
    }
    file << arr.dump();
}

void DatabaseManager::saveSavingsGoals() {
    std::ofstream file(dataDirectory + "/savings_goals.json");
    if (!file.is_open()) return;
    crow::json::wvalue arr;
    for (size_t i = 0; i < savingsGoalsCollection.size(); ++i) {
        arr[i] = savingsGoalsCollection[i].toJson();
    }
    file << arr.dump();
}

void DatabaseManager::seedInitialDemoData() {
    std::string demoUserId = "65e8a1f2b3c4d5e6f7a8b901";

    // 1. Create Demo User: demo@fintrack.com / Demo@123
    User demoUser;
    demoUser.setId(demoUserId);
    demoUser.setName("Alex Mercer");
    demoUser.setEmail("demo@fintrack.com");
    demoUser.setSalt("fintrack_salt_key");
    demoUser.hashAndSetPassword("Demo@123");
    demoUser.setCurrency("INR");
    demoUser.setCreatedAt("2026-11-01T08:00:00Z");
    usersCollection.push_back(demoUser);

    // 2. Realistic Sample Income (Total ₹75,000)
    // - Salary: ₹50,000
    // - Freelance: ₹15,000
    // - Other: ₹10,000
    transactionsCollection.push_back(std::make_shared<Income>(
        "65e8a1f2b3c4d5e6f7a8b101", demoUserId, 50000.0, "Salary", "Monthly Software Engineer Salary", "2026-11-01", "Bank Transfer", "Salary", true, "2026-11-01T09:00:00Z"
    ));
    transactionsCollection.push_back(std::make_shared<Income>(
        "65e8a1f2b3c4d5e6f7a8b102", demoUserId, 15000.0, "Freelance", "UI/UX Client Project Milestone", "2026-11-03", "UPI", "Freelance", false, "2026-11-03T14:30:00Z"
    ));
    transactionsCollection.push_back(std::make_shared<Income>(
        "65e8a1f2b3c4d5e6f7a8b103", demoUserId, 10000.0, "Other", "Quarterly Mutual Fund Dividend", "2026-11-04", "Bank Transfer", "Investment", false, "2026-11-04T11:15:00Z"
    ));

    // 3. Realistic Sample Expenses (Total ₹29,320)
    // - Food: ₹4,200
    // - Transport: ₹2,100
    // - Shopping: ₹5,500
    // - Entertainment: ₹2,500
    // - Education: ₹3,000
    // - Bills: ₹4,020
    // - Other: ₹8,000
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b201", demoUserId, 4200.0, "Food", "Supermarket Monthly Grocery", "2026-11-02", "UPI", "Nature Basket", false, "2026-11-02T16:20:00Z"
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b202", demoUserId, 2100.0, "Transport", "Metro SmartCard & Fuel Recharge", "2026-11-02", "Debit Card", "Metro Station", false, "2026-11-02T18:00:00Z"
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b203", demoUserId, 5500.0, "Shopping", "Amazon Ergonomic Chair & Tech Accessories", "2026-11-03", "Credit Card", "Amazon India", true, "2026-11-03T20:10:00Z"
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b204", demoUserId, 2500.0, "Entertainment", "Weekend Movies & Netflix Subscription", "2026-11-04", "UPI", "PVR Cinemas", false, "2026-11-04T21:45:00Z"
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b205", demoUserId, 3000.0, "Education", "C++ Advanced Systems & Algorithms Course", "2026-11-04", "Credit Card", "Coursera / OOPS Academy", true, "2026-11-04T12:00:00Z"
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b206", demoUserId, 4020.0, "Bills", "Fiber Broadband & Electricity Utility Bills", "2026-11-05", "Bank Transfer", "Power & Net Utility", false, "2026-11-05T10:00:00Z"
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b207", demoUserId, 8000.0, "Other", "Quarterly Health Insurance Premium", "2026-11-05", "Bank Transfer", "Health Corp", true, "2026-11-05T15:30:00Z"
    ));

    // 4. Sample Budgets (Monthly Budgets matching prompt)
    // - Food: ₹4,200 / ₹6,000 (70%)
    // - Transport: ₹2,100 / ₹4,000 (52%)
    // - Shopping: ₹5,500 / ₹8,000 (69%)
    // - Entertainment: ₹2,500 / ₹3,000 (83% - Warning)
    // - Education: ₹3,000 / ₹5,000 (60%)
    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b301", demoUserId, "Food", 6000.0, 4200.0, 11, 2026, "2026-11-01T00:00:00Z");
    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b302", demoUserId, "Transport", 4000.0, 2100.0, 11, 2026, "2026-11-01T00:00:00Z");
    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b303", demoUserId, "Shopping", 8000.0, 5500.0, 11, 2026, "2026-11-01T00:00:00Z");
    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b304", demoUserId, "Entertainment", 3000.0, 2500.0, 11, 2026, "2026-11-01T00:00:00Z");
    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b305", demoUserId, "Education", 5000.0, 3000.0, 11, 2026, "2026-11-01T00:00:00Z");

    // 5. Sample Savings Goals (Matching prompt)
    // - New Laptop: Target ₹80,000, Saved ₹45,000 (56%)
    // - Emergency Fund: Target ₹50,000, Saved ₹30,000 (60%)
    // - New Phone: Target ₹40,000, Saved ₹18,000 (45%)
    savingsGoalsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b401", demoUserId, "New Laptop", 80000.0, 45000.0, "2027-03-31", "Electronics", "2026-11-01T00:00:00Z");
    savingsGoalsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b402", demoUserId, "Emergency Fund", 50000.0, 30000.0, "2027-06-30", "Safety", "2026-11-01T00:00:00Z");
    savingsGoalsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b403", demoUserId, "New Phone", 40000.0, 18000.0, "2026-12-31", "Gadgets", "2026-11-01T00:00:00Z");

    saveUsers();
    saveTransactions();
    saveBudgets();
    saveSavingsGoals();
    std::cout << "[DatabaseManager] Seeded demo account (demo@fintrack.com) successfully." << std::endl;
}

// User Operations
bool DatabaseManager::insertUser(const User& user) {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (const auto& u : usersCollection) {
        if (u.getEmail() == user.getEmail()) {
            throw ValidationException("User with this email already exists.");
        }
    }
    User copy = user;
    if (copy.getId().empty()) {
        copy.setId(generateObjectId());
    }
    usersCollection.push_back(copy);
    saveUsers();
    return true;
}

std::shared_ptr<User> DatabaseManager::findUserByEmail(const std::string& email) const {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (const auto& u : usersCollection) {
        if (u.getEmail() == email) {
            return std::make_shared<User>(u);
        }
    }
    return nullptr;
}

std::shared_ptr<User> DatabaseManager::findUserById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (const auto& u : usersCollection) {
        if (u.getId() == id) {
            return std::make_shared<User>(u);
        }
    }
    return nullptr;
}

bool DatabaseManager::updateUser(const User& user) {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (auto& u : usersCollection) {
        if (u.getId() == user.getId()) {
            u = user;
            saveUsers();
            return true;
        }
    }
    return false;
}

// Transaction Operations
bool DatabaseManager::insertTransaction(std::shared_ptr<Transaction> tx) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (tx->getId().empty()) {
        tx->setId(generateObjectId());
    }
    transactionsCollection.push_back(tx);
    saveTransactions();
    return true;
}

std::shared_ptr<Transaction> DatabaseManager::findTransactionById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (const auto& tx : transactionsCollection) {
        if (tx->getId() == id) {
            return tx;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Transaction>> DatabaseManager::getTransactionsByUserId(const std::string& userId) const {
    std::lock_guard<std::mutex> lock(dbMutex);
    std::vector<std::shared_ptr<Transaction>> userList;
    for (const auto& tx : transactionsCollection) {
        if (tx->getUserId() == userId) {
            userList.push_back(tx);
        }
    }
    return userList;
}

bool DatabaseManager::updateTransaction(std::shared_ptr<Transaction> tx) {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (auto& current : transactionsCollection) {
        if (current->getId() == tx->getId() && current->getUserId() == tx->getUserId()) {
            current = tx;
            saveTransactions();
            return true;
        }
    }
    return false;
}

bool DatabaseManager::deleteTransaction(const std::string& id, const std::string& userId) {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (auto it = transactionsCollection.begin(); it != transactionsCollection.end(); ++it) {
        if ((*it)->getId() == id && (*it)->getUserId() == userId) {
            transactionsCollection.erase(it);
            saveTransactions();
            return true;
        }
    }
    return false;
}

// Budget Operations
bool DatabaseManager::insertBudget(const Budget& budget) {
    std::lock_guard<std::mutex> lock(dbMutex);
    Budget copy = budget;
    if (copy.getId().empty()) {
        copy.setId(generateObjectId());
    }
    budgetsCollection.push_back(copy);
    saveBudgets();
    return true;
}

std::shared_ptr<Budget> DatabaseManager::findBudgetById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (const auto& b : budgetsCollection) {
        if (b.getId() == id) {
            return std::make_shared<Budget>(b);
        }
    }
    return nullptr;
}

std::vector<Budget> DatabaseManager::getBudgetsByUserId(const std::string& userId) const {
    std::lock_guard<std::mutex> lock(dbMutex);
    std::vector<Budget> userBudgets;
    for (const auto& b : budgetsCollection) {
        if (b.getUserId() == userId) {
            userBudgets.push_back(b);
        }
    }
    return userBudgets;
}

bool DatabaseManager::updateBudget(const Budget& budget) {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (auto& b : budgetsCollection) {
        if (b.getId() == budget.getId() && b.getUserId() == budget.getUserId()) {
            b = budget;
            saveBudgets();
            return true;
        }
    }
    return false;
}

bool DatabaseManager::deleteBudget(const std::string& id, const std::string& userId) {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (auto it = budgetsCollection.begin(); it != budgetsCollection.end(); ++it) {
        if (it->getId() == id && it->getUserId() == userId) {
            budgetsCollection.erase(it);
            saveBudgets();
            return true;
        }
    }
    return false;
}

// Savings Goals Operations
bool DatabaseManager::insertSavingsGoal(const SavingsGoal& goal) {
    std::lock_guard<std::mutex> lock(dbMutex);
    SavingsGoal copy = goal;
    if (copy.getId().empty()) {
        copy.setId(generateObjectId());
    }
    savingsGoalsCollection.push_back(copy);
    saveSavingsGoals();
    return true;
}

std::shared_ptr<SavingsGoal> DatabaseManager::findSavingsGoalById(const std::string& id) const {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (const auto& g : savingsGoalsCollection) {
        if (g.getId() == id) {
            return std::make_shared<SavingsGoal>(g);
        }
    }
    return nullptr;
}

std::vector<SavingsGoal> DatabaseManager::getSavingsGoalsByUserId(const std::string& userId) const {
    std::lock_guard<std::mutex> lock(dbMutex);
    std::vector<SavingsGoal> userGoals;
    for (const auto& g : savingsGoalsCollection) {
        if (g.getUserId() == userId) {
            userGoals.push_back(g);
        }
    }
    return userGoals;
}

bool DatabaseManager::updateSavingsGoal(const SavingsGoal& goal) {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (auto& g : savingsGoalsCollection) {
        if (g.getId() == goal.getId() && g.getUserId() == goal.getUserId()) {
            g = goal;
            saveSavingsGoals();
            return true;
        }
    }
    return false;
}

bool DatabaseManager::deleteSavingsGoal(const std::string& id, const std::string& userId) {
    std::lock_guard<std::mutex> lock(dbMutex);
    for (auto it = savingsGoalsCollection.begin(); it != savingsGoalsCollection.end(); ++it) {
        if (it->getId() == id && it->getUserId() == userId) {
            savingsGoalsCollection.erase(it);
            saveSavingsGoals();
            return true;
        }
    }
    return false;
}

void DatabaseManager::resetDemoData(const std::string& demoUserId) {
    std::lock_guard<std::mutex> lock(dbMutex);
    // Remove all demo transactions, budgets, savings
    transactionsCollection.erase(
        std::remove_if(transactionsCollection.begin(), transactionsCollection.end(),
            [&](const std::shared_ptr<Transaction>& tx) { return tx->getUserId() == demoUserId; }),
        transactionsCollection.end()
    );
    budgetsCollection.erase(
        std::remove_if(budgetsCollection.begin(), budgetsCollection.end(),
            [&](const Budget& b) { return b.getUserId() == demoUserId; }),
        budgetsCollection.end()
    );
    savingsGoalsCollection.erase(
        std::remove_if(savingsGoalsCollection.begin(), savingsGoalsCollection.end(),
            [&](const SavingsGoal& g) { return g.getUserId() == demoUserId; }),
        savingsGoalsCollection.end()
    );

    // Call seed logic
    // Add realistic seed data back
    transactionsCollection.push_back(std::make_shared<Income>(
        "65e8a1f2b3c4d5e6f7a8b101", demoUserId, 50000.0, "Salary", "Monthly Software Engineer Salary", "2026-11-01", "Bank Transfer", "Salary", true
    ));
    transactionsCollection.push_back(std::make_shared<Income>(
        "65e8a1f2b3c4d5e6f7a8b102", demoUserId, 15000.0, "Freelance", "UI/UX Client Project Milestone", "2026-11-03", "UPI", "Freelance", false
    ));
    transactionsCollection.push_back(std::make_shared<Income>(
        "65e8a1f2b3c4d5e6f7a8b103", demoUserId, 10000.0, "Other", "Quarterly Mutual Fund Dividend", "2026-11-04", "Bank Transfer", "Investment", false
    ));

    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b201", demoUserId, 4200.0, "Food", "Supermarket Monthly Grocery", "2026-11-02", "UPI", "Nature Basket", false
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b202", demoUserId, 2100.0, "Transport", "Metro SmartCard & Fuel Recharge", "2026-11-02", "Debit Card", "Metro Station", false
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b203", demoUserId, 5500.0, "Shopping", "Amazon Ergonomic Chair & Tech Accessories", "2026-11-03", "Credit Card", "Amazon India", true
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b204", demoUserId, 2500.0, "Entertainment", "Weekend Movies & Netflix Subscription", "2026-11-04", "UPI", "PVR Cinemas", false
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b205", demoUserId, 3000.0, "Education", "C++ Advanced Systems & Algorithms Course", "2026-11-04", "Credit Card", "Coursera / OOPS Academy", true
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b206", demoUserId, 4020.0, "Bills", "Fiber Broadband & Electricity Utility Bills", "2026-11-05", "Bank Transfer", "Power & Net Utility", false
    ));
    transactionsCollection.push_back(std::make_shared<Expense>(
        "65e8a1f2b3c4d5e6f7a8b207", demoUserId, 8000.0, "Other", "Quarterly Health Insurance Premium", "2026-11-05", "Bank Transfer", "Health Corp", true
    ));

    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b301", demoUserId, "Food", 6000.0, 4200.0, 11, 2026);
    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b302", demoUserId, "Transport", 4000.0, 2100.0, 11, 2026);
    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b303", demoUserId, "Shopping", 8000.0, 5500.0, 11, 2026);
    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b304", demoUserId, "Entertainment", 3000.0, 2500.0, 11, 2026);
    budgetsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b305", demoUserId, "Education", 5000.0, 3000.0, 11, 2026);

    savingsGoalsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b401", demoUserId, "New Laptop", 80000.0, 45000.0, "2027-03-31", "Electronics");
    savingsGoalsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b402", demoUserId, "Emergency Fund", 50000.0, 30000.0, "2027-06-30", "Safety");
    savingsGoalsCollection.emplace_back("65e8a1f2b3c4d5e6f7a8b403", demoUserId, "New Phone", 40000.0, 18000.0, "2026-12-31", "Gadgets");

    saveTransactions();
    saveBudgets();
    saveSavingsGoals();
}

