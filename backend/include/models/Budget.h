#pragma once

#ifndef BUDGET_H
#define BUDGET_H

#include <string>
#include "../crow.h"

/**
 * @brief Represents a category-specific budget limit.
 * Demonstrates: Encapsulation, Business Logic Computation.
 */
class Budget {
private:
    std::string id;
    std::string userId;
    std::string category;
    double limitAmount;
    double spentAmount;
    int month;
    int year;
    std::string createdAt;

public:
    Budget();
    Budget(const std::string& id,
           const std::string& userId,
           const std::string& category,
           double limitAmount,
           double spentAmount = 0.0,
           int month = 0,
           int year = 0,
           const std::string& createdAt = "");

    // Encapsulation: Getters
    std::string getId() const { return id; }
    std::string getUserId() const { return userId; }
    std::string getCategory() const { return category; }
    double getLimitAmount() const { return limitAmount; }
    double getSpentAmount() const { return spentAmount; }
    int getMonth() const { return month; }
    int getYear() const { return year; }
    std::string getCreatedAt() const { return createdAt; }

    // Encapsulation: Setters with Validation
    void setId(const std::string& newId) { id = newId; }
    void setUserId(const std::string& newUserId) { userId = newUserId; }
    void setCategory(const std::string& newCategory);
    void setLimitAmount(double newLimit);
    void setSpentAmount(double newSpent);
    void setMonth(int m) { month = m; }
    void setYear(int y) { year = y; }
    void setCreatedAt(const std::string& c) { createdAt = c; }

    // Business Logic Methods
    double getPercentage() const;
    double getRemaining() const;
    bool isApproachingLimit(double threshold = 0.80) const;
    bool isExceeded() const;
    std::string getStatus() const; // "Safe", "Warning", "Exceeded"
    void addSpending(double amount);
    void resetSpending();

    crow::json::wvalue toJson() const;
};

#endif // BUDGET_H

