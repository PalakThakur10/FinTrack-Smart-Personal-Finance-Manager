#pragma once

#ifndef SAVINGS_GOAL_H
#define SAVINGS_GOAL_H

#include <string>
#include "../crow.h"

/**
 * @brief Represents a user savings target or financial goal.
 * Demonstrates: Encapsulation, State Mutation, Business Metrics.
 */
class SavingsGoal {
private:
    std::string id;
    std::string userId;
    std::string name;
    double targetAmount;
    double currentAmount;
    std::string targetDate;
    std::string category;
    std::string createdAt;

public:
    SavingsGoal();
    SavingsGoal(const std::string& id,
                const std::string& userId,
                const std::string& name,
                double targetAmount,
                double currentAmount = 0.0,
                const std::string& targetDate = "",
                const std::string& category = "General",
                const std::string& createdAt = "");

    // Encapsulation: Getters
    std::string getId() const { return id; }
    std::string getUserId() const { return userId; }
    std::string getName() const { return name; }
    double getTargetAmount() const { return targetAmount; }
    double getCurrentAmount() const { return currentAmount; }
    std::string getTargetDate() const { return targetDate; }
    std::string getCategory() const { return category; }
    std::string getCreatedAt() const { return createdAt; }

    // Encapsulation: Setters with Validation
    void setId(const std::string& newId) { id = newId; }
    void setUserId(const std::string& newUserId) { userId = newUserId; }
    void setName(const std::string& newName);
    void setTargetAmount(double target);
    void setCurrentAmount(double current);
    void setTargetDate(const std::string& date) { targetDate = date; }
    void setCategory(const std::string& cat) { category = cat; }
    void setCreatedAt(const std::string& created) { createdAt = created; }

    // Business Logic Methods
    double getPercentage() const;
    double getRemaining() const;
    bool isCompleted() const;
    void addMoney(double amount);
    void withdrawMoney(double amount);

    crow::json::wvalue toJson() const;
};

#endif // SAVINGS_GOAL_H

