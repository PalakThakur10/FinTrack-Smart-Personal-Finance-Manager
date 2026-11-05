#pragma once

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "../models/User.h"
#include "../models/Transaction.h"
#include "../models/Income.h"
#include "../models/Expense.h"
#include "../models/Budget.h"
#include "../models/SavingsGoal.h"

class DatabaseException : public std::exception {
private:
    std::string message;
public:
    explicit DatabaseException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

/**
 * @brief Manages persistence for MongoDB collections: users, transactions, budgets, savings_goals.
 * Demonstrates: Encapsulation, Abstraction, Thread Safety, Data Persistence.
 */
class DatabaseManager {
private:
    std::string dataDirectory;
    std::string mongoUri;
    bool isConnected;
    mutable std::mutex dbMutex;

    // In-memory collection indexes & storage
    std::vector<User> usersCollection;
    std::vector<std::shared_ptr<Transaction>> transactionsCollection;
    std::vector<Budget> budgetsCollection;
    std::vector<SavingsGoal> savingsGoalsCollection;

    // Helpers
    std::string generateObjectId() const;
    void loadAllCollections();
    void saveUsers();
    void saveTransactions();
    void saveBudgets();
    void saveSavingsGoals();

public:
    DatabaseManager(const std::string& dir = "database/data", const std::string& uri = "mongodb://127.0.0.1:27017/fintrack");
    ~DatabaseManager();

    // Database Lifecycle
    bool initialize();
    bool connect();
    void seedInitialDemoData();

    // User Operations
    bool insertUser(const User& user);
    std::shared_ptr<User> findUserByEmail(const std::string& email) const;
    std::shared_ptr<User> findUserById(const std::string& id) const;
    bool updateUser(const User& user);

    // Transaction Operations
    bool insertTransaction(std::shared_ptr<Transaction> tx);
    std::shared_ptr<Transaction> findTransactionById(const std::string& id) const;
    std::vector<std::shared_ptr<Transaction>> getTransactionsByUserId(const std::string& userId) const;
    bool updateTransaction(std::shared_ptr<Transaction> tx);
    bool deleteTransaction(const std::string& id, const std::string& userId);

    // Budget Operations
    bool insertBudget(const Budget& budget);
    std::shared_ptr<Budget> findBudgetById(const std::string& id) const;
    std::vector<Budget> getBudgetsByUserId(const std::string& userId) const;
    bool updateBudget(const Budget& budget);
    bool deleteBudget(const std::string& id, const std::string& userId);

    // Savings Goals Operations
    bool insertSavingsGoal(const SavingsGoal& goal);
    std::shared_ptr<SavingsGoal> findSavingsGoalById(const std::string& id) const;
    std::vector<SavingsGoal> getSavingsGoalsByUserId(const std::string& userId) const;
    bool updateSavingsGoal(const SavingsGoal& goal);
    bool deleteSavingsGoal(const std::string& id, const std::string& userId);

    // Reset Data for demo user
    void resetDemoData(const std::string& demoUserId);
};

#endif // DATABASE_MANAGER_H

