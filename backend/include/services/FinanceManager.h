#pragma once

#ifndef FINANCE_MANAGER_H
#define FINANCE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "../database/DatabaseManager.h"
#include "../models/Transaction.h"
#include "../models/Income.h"
#include "../models/Expense.h"
#include "../models/Budget.h"
#include "../models/SavingsGoal.h"
#include "../crow.h"

struct FinancialSummary {
    double totalBalance = 0.0;
    double totalIncome = 0.0;
    double totalExpenses = 0.0;
    double totalSavings = 0.0;
    double savingsRate = 0.0;
    std::string currency = "INR";
};

/**
 * @brief Coordinates financial computations, budget recalculation, and dynamic polymorphism.
 * Demonstrates: Polymorphism, Method Overloading, Association, Encapsulation.
 */
class FinanceManager {
private:
    std::shared_ptr<DatabaseManager> db;

public:
    explicit FinanceManager(std::shared_ptr<DatabaseManager> dbManager);
    ~FinanceManager() = default;

    // Financial Computation (Polymorphism)
    FinancialSummary calculateSummary(const std::string& userId) const;
    crow::json::wvalue getDashboardData(const std::string& userId) const;

    // Transaction Management
    crow::json::wvalue addTransaction(const std::string& userId,
                                      const std::string& type,
                                      double amount,
                                      const std::string& category,
                                      const std::string& description,
                                      const std::string& date,
                                      const std::string& paymentMethod,
                                      const std::string& extraField = "");

    crow::json::wvalue updateTransaction(const std::string& userId,
                                         const std::string& transactionId,
                                         const std::string& type,
                                         double amount,
                                         const std::string& category,
                                         const std::string& description,
                                         const std::string& date,
                                         const std::string& paymentMethod,
                                         const std::string& extraField = "");

    bool deleteTransaction(const std::string& userId, const std::string& transactionId);

    // Function Overloading: Polymorphic Filtering
    crow::json::wvalue getTransactions(const std::string& userId) const;
    crow::json::wvalue getTransactions(const std::string& userId, const std::string& type) const;
    crow::json::wvalue getTransactions(const std::string& userId,
                                       const std::string& type,
                                       const std::string& category,
                                       const std::string& startDate,
                                       const std::string& endDate,
                                       const std::string& searchKeyword,
                                       double minAmount = 0.0,
                                       double maxAmount = 1e9) const;

    // Budget Management with Dynamic Recalculation
    crow::json::wvalue getBudgets(const std::string& userId) const;
    crow::json::wvalue addBudget(const std::string& userId,
                                 const std::string& category,
                                 double limitAmount,
                                 int month = 0,
                                 int year = 0);
    crow::json::wvalue updateBudget(const std::string& userId,
                                    const std::string& budgetId,
                                    const std::string& category,
                                    double limitAmount,
                                    int month,
                                    int year);
    bool deleteBudget(const std::string& userId, const std::string& budgetId);

    // Savings Goals Management
    crow::json::wvalue getSavingsGoals(const std::string& userId) const;
    crow::json::wvalue addSavingsGoal(const std::string& userId,
                                      const std::string& name,
                                      double targetAmount,
                                      double currentAmount,
                                      const std::string& targetDate,
                                      const std::string& category = "General");
    crow::json::wvalue updateSavingsGoal(const std::string& userId,
                                         const std::string& goalId,
                                         const std::string& name,
                                         double targetAmount,
                                         double currentAmount,
                                         const std::string& targetDate,
                                         const std::string& category);
    crow::json::wvalue addMoneyToGoal(const std::string& userId,
                                      const std::string& goalId,
                                      double amount);
    bool deleteSavingsGoal(const std::string& userId, const std::string& goalId);

    // Category Breakdown & Analytics
    crow::json::wvalue getCategoryBreakdown(const std::string& userId) const;
    crow::json::wvalue getMonthlySpendingTrend(const std::string& userId) const;
    crow::json::wvalue getAnalytics(const std::string& userId, const std::string& period = "monthly") const;
};

#endif // FINANCE_MANAGER_H

