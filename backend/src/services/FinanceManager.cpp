#include "../../include/services/FinanceManager.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>

FinanceManager::FinanceManager(std::shared_ptr<DatabaseManager> dbManager)
    : db(dbManager) {}

FinancialSummary FinanceManager::calculateSummary(const std::string& userId) const {
    FinancialSummary summary;
    auto transactions = db->getTransactionsByUserId(userId);

    double balance = 0.0;
    double income = 0.0;
    double expenses = 0.0;

    // Dynamic Polymorphism: calculateImpact() called on base Transaction pointer
    for (const auto& tx : transactions) {
        double impact = tx->calculateImpact();
        balance += impact;
        if (impact > 0.0) {
            income += impact;
        } else {
            expenses += std::abs(impact);
        }
    }

    summary.totalBalance = balance;
    summary.totalIncome = income;
    summary.totalExpenses = expenses;

    // Savings goals accumulated
    auto goals = db->getSavingsGoalsByUserId(userId);
    double goalsSaved = 0.0;
    for (const auto& g : goals) {
        goalsSaved += g.getCurrentAmount();
    }
    // Total savings shows net balance or goal savings
    summary.totalSavings = (balance > 0.0) ? balance : goalsSaved;

    if (income > 0.0) {
        summary.savingsRate = ((income - expenses) / income) * 100.0;
        if (summary.savingsRate < 0.0) summary.savingsRate = 0.0;
    } else {
        summary.savingsRate = 0.0;
    }

    auto user = db->findUserById(userId);
    if (user) {
        summary.currency = user->getCurrency();
    }

    return summary;
}

crow::json::wvalue FinanceManager::getDashboardData(const std::string& userId) const {
    FinancialSummary summary = calculateSummary(userId);
    crow::json::wvalue res;

    // 1. Metric Cards
    res["summary"]["totalBalance"] = summary.totalBalance;
    res["summary"]["totalIncome"] = summary.totalIncome;
    res["summary"]["totalExpenses"] = summary.totalExpenses;
    res["summary"]["totalSavings"] = summary.totalSavings;
    res["summary"]["savingsRate"] = summary.savingsRate;
    res["summary"]["currency"] = summary.currency;

    // 2. Budgets with dynamic calculation
    auto budgets = db->getBudgetsByUserId(userId);
    auto txs = db->getTransactionsByUserId(userId);

    // Compute actual spending per category
    std::map<std::string, double> categorySpending;
    for (const auto& tx : txs) {
        if (tx->calculateImpact() < 0.0) {
            categorySpending[tx->getCategory()] += tx->getAmount();
        }
    }

    crow::json::wvalue budgetArr;
    for (size_t i = 0; i < budgets.size(); ++i) {
        auto b = budgets[i];
        if (categorySpending.find(b.getCategory()) != categorySpending.end()) {
            b.setSpentAmount(categorySpending[b.getCategory()]);
        }
        budgetArr[i] = b.toJson();
    }
    res["budgets"] = budgetArr;

    // 3. Expense Breakdown (Doughnut Chart)
    crow::json::wvalue catLabels;
    crow::json::wvalue catAmounts;
    size_t catIdx = 0;
    for (const auto& pair : categorySpending) {
        catLabels[catIdx] = pair.first;
        catAmounts[catIdx] = pair.second;
        catIdx++;
    }
    res["expenseBreakdown"]["labels"] = catLabels;
    res["expenseBreakdown"]["data"] = catAmounts;

    // 4. Monthly Trend (Line/Bar Chart)
    std::map<std::string, std::pair<double, double>> monthlyMap; // month -> {income, expense}
    for (const auto& tx : txs) {
        std::string date = tx->getDate();
        std::string monthKey = (date.length() >= 7) ? date.substr(0, 7) : "2026-11";
        if (tx->calculateImpact() > 0.0) {
            monthlyMap[monthKey].first += tx->getAmount();
        } else {
            monthlyMap[monthKey].second += tx->getAmount();
        }
    }

    crow::json::wvalue monthLabels;
    crow::json::wvalue monthIncome;
    crow::json::wvalue monthExpense;
    size_t mIdx = 0;
    for (const auto& pair : monthlyMap) {
        monthLabels[mIdx] = pair.first;
        monthIncome[mIdx] = pair.second.first;
        monthExpense[mIdx] = pair.second.second;
        mIdx++;
    }
    // If empty, provide current month
    if (mIdx == 0) {
        monthLabels[0] = "2026-11";
        monthIncome[0] = summary.totalIncome;
        monthExpense[0] = summary.totalExpenses;
    }
    res["monthlyTrend"]["labels"] = monthLabels;
    res["monthlyTrend"]["income"] = monthIncome;
    res["monthlyTrend"]["expenses"] = monthExpense;

    // 5. Recent Transactions (Sorted descending by date)
    auto sortedTxs = txs;
    std::sort(sortedTxs.begin(), sortedTxs.end(), [](const std::shared_ptr<Transaction>& a, const std::shared_ptr<Transaction>& b) {
        return a->getDate() > b->getDate();
    });

    crow::json::wvalue recentArr;
    size_t limit = std::min(sortedTxs.size(), static_cast<size_t>(5));
    for (size_t i = 0; i < limit; ++i) {
        recentArr[i] = sortedTxs[i]->toJson();
    }
    res["recentTransactions"] = recentArr;

    return res;
}

crow::json::wvalue FinanceManager::addTransaction(const std::string& userId,
                                                 const std::string& type,
                                                 double amount,
                                                 const std::string& category,
                                                 const std::string& description,
                                                 const std::string& date,
                                                 const std::string& paymentMethod,
                                                 const std::string& extraField) {
    if (amount <= 0.0) {
        throw ValidationException("Transaction amount must be greater than zero.");
    }
    if (category.empty()) {
        throw ValidationException("Category cannot be empty.");
    }
    if (date.empty()) {
        throw ValidationException("Date cannot be empty.");
    }

    std::shared_ptr<Transaction> tx;
    if (type == "income") {
        tx = std::make_shared<Income>("", userId, amount, category, description, date, paymentMethod, extraField);
    } else {
        tx = std::make_shared<Expense>("", userId, amount, category, description, date, paymentMethod, extraField);
    }

    if (!db->insertTransaction(tx)) {
        throw DatabaseException("Failed to persist transaction to database.");
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["message"] = "Transaction created successfully";
    res["transaction"] = tx->toJson();
    return res;
}

crow::json::wvalue FinanceManager::updateTransaction(const std::string& userId,
                                                    const std::string& transactionId,
                                                    const std::string& type,
                                                    double amount,
                                                    const std::string& category,
                                                    const std::string& description,
                                                    const std::string& date,
                                                    const std::string& paymentMethod,
                                                    const std::string& extraField) {
    auto existing = db->findTransactionById(transactionId);
    if (!existing || existing->getUserId() != userId) {
        throw NotFoundException("Transaction not found or access denied.");
    }

    std::shared_ptr<Transaction> updated;
    if (type == "income") {
        updated = std::make_shared<Income>(transactionId, userId, amount, category, description, date, paymentMethod, extraField, false, existing->getCreatedAt());
    } else {
        updated = std::make_shared<Expense>(transactionId, userId, amount, category, description, date, paymentMethod, extraField, false, existing->getCreatedAt());
    }

    if (!db->updateTransaction(updated)) {
        throw DatabaseException("Failed to update transaction in database.");
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["message"] = "Transaction updated successfully";
    res["transaction"] = updated->toJson();
    return res;
}

bool FinanceManager::deleteTransaction(const std::string& userId, const std::string& transactionId) {
    return db->deleteTransaction(transactionId, userId);
}

// Function Overloading: Implementation 1 (all transactions for user)
crow::json::wvalue FinanceManager::getTransactions(const std::string& userId) const {
    auto txs = db->getTransactionsByUserId(userId);
    std::sort(txs.begin(), txs.end(), [](const auto& a, const auto& b) {
        return a->getDate() > b->getDate();
    });

    crow::json::wvalue res;
    for (size_t i = 0; i < txs.size(); ++i) {
        res[i] = txs[i]->toJson();
    }
    return res;
}

// Function Overloading: Implementation 2 (filter by type: income or expense)
crow::json::wvalue FinanceManager::getTransactions(const std::string& userId, const std::string& type) const {
    auto txs = db->getTransactionsByUserId(userId);
    std::sort(txs.begin(), txs.end(), [](const auto& a, const auto& b) {
        return a->getDate() > b->getDate();
    });

    crow::json::wvalue res;
    size_t count = 0;
    for (const auto& tx : txs) {
        if (tx->getType() == type) {
            res[count++] = tx->toJson();
        }
    }
    return res;
}

// Function Overloading: Implementation 3 (multi-criteria search and filter)
crow::json::wvalue FinanceManager::getTransactions(const std::string& userId,
                                                   const std::string& type,
                                                   const std::string& category,
                                                   const std::string& startDate,
                                                   const std::string& endDate,
                                                   const std::string& searchKeyword,
                                                   double minAmount,
                                                   double maxAmount) const {
    auto txs = db->getTransactionsByUserId(userId);

    std::string lowerKeyword = searchKeyword;
    std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);

    std::vector<std::shared_ptr<Transaction>> filtered;
    for (const auto& tx : txs) {
        // Type filter
        if (!type.empty() && type != "all" && tx->getType() != type) continue;

        // Category filter
        if (!category.empty() && category != "all" && tx->getCategory() != category) continue;

        // Amount filter
        if (tx->getAmount() < minAmount || tx->getAmount() > maxAmount) continue;

        // Date range
        if (!startDate.empty() && tx->getDate() < startDate) continue;
        if (!endDate.empty() && tx->getDate() > endDate) continue;

        // Search Keyword matching description or category or paymentMethod
        if (!lowerKeyword.empty()) {
            std::string desc = tx->getDescription();
            std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);
            std::string cat = tx->getCategory();
            std::transform(cat.begin(), cat.end(), cat.begin(), ::tolower);
            if (desc.find(lowerKeyword) == std::string::npos && cat.find(lowerKeyword) == std::string::npos) {
                continue;
            }
        }

        filtered.push_back(tx);
    }

    std::sort(filtered.begin(), filtered.end(), [](const auto& a, const auto& b) {
        return a->getDate() > b->getDate();
    });

    crow::json::wvalue res;
    for (size_t i = 0; i < filtered.size(); ++i) {
        res[i] = filtered[i]->toJson();
    }
    return res;
}

// Budget Management
crow::json::wvalue FinanceManager::getBudgets(const std::string& userId) const {
    auto budgets = db->getBudgetsByUserId(userId);
    auto txs = db->getTransactionsByUserId(userId);

    std::map<std::string, double> categorySpending;
    for (const auto& tx : txs) {
        if (tx->calculateImpact() < 0.0) {
            categorySpending[tx->getCategory()] += tx->getAmount();
        }
    }

    crow::json::wvalue res;
    for (size_t i = 0; i < budgets.size(); ++i) {
        auto b = budgets[i];
        if (categorySpending.find(b.getCategory()) != categorySpending.end()) {
            b.setSpentAmount(categorySpending[b.getCategory()]);
        }
        res[i] = b.toJson();
    }
    return res;
}

crow::json::wvalue FinanceManager::addBudget(const std::string& userId,
                                             const std::string& category,
                                             double limitAmount,
                                             int month,
                                             int year) {
    if (limitAmount <= 0.0) {
        throw ValidationException("Budget limit must be greater than zero.");
    }
    if (category.empty()) {
        throw ValidationException("Category cannot be empty.");
    }
    if (month <= 0) month = 11;
    if (year <= 0) year = 2026;

    Budget budget("", userId, category, limitAmount, 0.0, month, year);
    if (!db->insertBudget(budget)) {
        throw DatabaseException("Failed to add budget.");
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["message"] = "Budget created successfully";
    res["budget"] = budget.toJson();
    return res;
}

crow::json::wvalue FinanceManager::updateBudget(const std::string& userId,
                                                const std::string& budgetId,
                                                const std::string& category,
                                                double limitAmount,
                                                int month,
                                                int year) {
    auto existing = db->findBudgetById(budgetId);
    if (!existing || existing->getUserId() != userId) {
        throw NotFoundException("Budget not found or access denied.");
    }

    existing->setCategory(category);
    existing->setLimitAmount(limitAmount);
    existing->setMonth(month);
    existing->setYear(year);

    if (!db->updateBudget(*existing)) {
        throw DatabaseException("Failed to update budget.");
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["message"] = "Budget updated successfully";
    res["budget"] = existing->toJson();
    return res;
}

bool FinanceManager::deleteBudget(const std::string& userId, const std::string& budgetId) {
    return db->deleteBudget(budgetId, userId);
}

// Savings Goals Management
crow::json::wvalue FinanceManager::getSavingsGoals(const std::string& userId) const {
    auto goals = db->getSavingsGoalsByUserId(userId);
    crow::json::wvalue res;
    for (size_t i = 0; i < goals.size(); ++i) {
        res[i] = goals[i].toJson();
    }
    return res;
}

crow::json::wvalue FinanceManager::addSavingsGoal(const std::string& userId,
                                                 const std::string& name,
                                                 double targetAmount,
                                                 double currentAmount,
                                                 const std::string& targetDate,
                                                 const std::string& category) {
    if (name.empty()) {
        throw ValidationException("Goal name cannot be empty.");
    }
    if (targetAmount <= 0.0) {
        throw ValidationException("Target amount must be greater than zero.");
    }

    SavingsGoal goal("", userId, name, targetAmount, currentAmount, targetDate, category);
    if (!db->insertSavingsGoal(goal)) {
        throw DatabaseException("Failed to add savings goal.");
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["message"] = "Savings goal created successfully";
    res["goal"] = goal.toJson();
    return res;
}

crow::json::wvalue FinanceManager::updateSavingsGoal(const std::string& userId,
                                                    const std::string& goalId,
                                                    const std::string& name,
                                                    double targetAmount,
                                                    double currentAmount,
                                                    const std::string& targetDate,
                                                    const std::string& category) {
    auto existing = db->findSavingsGoalById(goalId);
    if (!existing || existing->getUserId() != userId) {
        throw NotFoundException("Savings goal not found or access denied.");
    }

    existing->setName(name);
    existing->setTargetAmount(targetAmount);
    existing->setCurrentAmount(currentAmount);
    existing->setTargetDate(targetDate);
    existing->setCategory(category);

    if (!db->updateSavingsGoal(*existing)) {
        throw DatabaseException("Failed to update savings goal.");
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["message"] = "Savings goal updated successfully";
    res["goal"] = existing->toJson();
    return res;
}

crow::json::wvalue FinanceManager::addMoneyToGoal(const std::string& userId,
                                                 const std::string& goalId,
                                                 double amount) {
    if (amount <= 0.0) {
        throw ValidationException("Deposit amount must be greater than zero.");
    }
    auto existing = db->findSavingsGoalById(goalId);
    if (!existing || existing->getUserId() != userId) {
        throw NotFoundException("Savings goal not found or access denied.");
    }

    existing->addMoney(amount);
    if (!db->updateSavingsGoal(*existing)) {
        throw DatabaseException("Failed to deposit money to goal.");
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["message"] = "Money added to savings goal successfully";
    res["goal"] = existing->toJson();
    return res;
}

bool FinanceManager::deleteSavingsGoal(const std::string& userId, const std::string& goalId) {
    return db->deleteSavingsGoal(goalId, userId);
}

// Analytics and Category breakdown
crow::json::wvalue FinanceManager::getCategoryBreakdown(const std::string& userId) const {
    auto txs = db->getTransactionsByUserId(userId);
    std::map<std::string, double> catSpending;
    double totalExp = 0.0;

    for (const auto& tx : txs) {
        if (tx->calculateImpact() < 0.0) {
            catSpending[tx->getCategory()] += tx->getAmount();
            totalExp += tx->getAmount();
        }
    }

    crow::json::wvalue res;
    size_t idx = 0;
    for (const auto& pair : catSpending) {
        crow::json::wvalue item;
        item["category"] = pair.first;
        item["amount"] = pair.second;
        item["percentage"] = (totalExp > 0.0) ? (pair.second / totalExp) * 100.0 : 0.0;
        res[idx++] = item;
    }
    return res;
}

crow::json::wvalue FinanceManager::getMonthlySpendingTrend(const std::string& userId) const {
    auto txs = db->getTransactionsByUserId(userId);
    std::map<std::string, std::pair<double, double>> trend; // month -> {income, expense}

    for (const auto& tx : txs) {
        std::string date = tx->getDate();
        std::string mKey = (date.length() >= 7) ? date.substr(0, 7) : "2026-11";
        if (tx->calculateImpact() > 0.0) {
            trend[mKey].first += tx->getAmount();
        } else {
            trend[mKey].second += tx->getAmount();
        }
    }

    crow::json::wvalue res;
    size_t idx = 0;
    for (const auto& pair : trend) {
        crow::json::wvalue item;
        item["month"] = pair.first;
        item["income"] = pair.second.first;
        item["expense"] = pair.second.second;
        item["savings"] = pair.second.first - pair.second.second;
        res[idx++] = item;
    }
    return res;
}

crow::json::wvalue FinanceManager::getAnalytics(const std::string& userId, const std::string& period) const {
    crow::json::wvalue res;
    auto summary = calculateSummary(userId);
    res["summary"]["totalIncome"] = summary.totalIncome;
    res["summary"]["totalExpenses"] = summary.totalExpenses;
    res["summary"]["totalSavings"] = summary.totalSavings;
    res["summary"]["savingsRate"] = summary.savingsRate;

    res["categoryBreakdown"] = getCategoryBreakdown(userId);
    res["monthlyTrend"] = getMonthlySpendingTrend(userId);

    // Budget performance
    res["budgetPerformance"] = getBudgets(userId);

    // Savings Growth
    auto goals = db->getSavingsGoalsByUserId(userId);
    crow::json::wvalue goalArr;
    for (size_t i = 0; i < goals.size(); ++i) {
        goalArr[i] = goals[i].toJson();
    }
    res["savingsGrowth"] = goalArr;

    return res;
}

