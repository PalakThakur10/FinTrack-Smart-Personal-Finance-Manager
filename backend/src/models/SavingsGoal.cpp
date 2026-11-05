#include "../../include/models/SavingsGoal.h"
#include "../../include/models/Transaction.h" // For ValidationException
#include <chrono>
#include <iomanip>
#include <sstream>

SavingsGoal::SavingsGoal()
    : id(""), userId(""), name(""), targetAmount(1000.0),
      currentAmount(0.0), targetDate(""), category("General"), createdAt("") {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    createdAt = ss.str();
}

SavingsGoal::SavingsGoal(const std::string& id,
                         const std::string& userId,
                         const std::string& name,
                         double targetAmount,
                         double currentAmount,
                         const std::string& targetDate,
                         const std::string& category,
                         const std::string& createdAt)
    : id(id), userId(userId), targetDate(targetDate), category(category) {
    setName(name);
    setTargetAmount(targetAmount);
    setCurrentAmount(currentAmount);

    if (createdAt.empty()) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
        this->createdAt = ss.str();
    } else {
        this->createdAt = createdAt;
    }
}

void SavingsGoal::setName(const std::string& newName) {
    if (newName.empty()) {
        throw ValidationException("Savings goal name cannot be empty.");
    }
    name = newName;
}

void SavingsGoal::setTargetAmount(double target) {
    if (target <= 0.0) {
        throw ValidationException("Savings target amount must be greater than zero.");
    }
    targetAmount = target;
}

void SavingsGoal::setCurrentAmount(double current) {
    currentAmount = (current < 0.0) ? 0.0 : current;
}

double SavingsGoal::getPercentage() const {
    if (targetAmount <= 0.0) return 0.0;
    return (currentAmount / targetAmount) * 100.0;
}

double SavingsGoal::getRemaining() const {
    double rem = targetAmount - currentAmount;
    return (rem > 0.0) ? rem : 0.0;
}

bool SavingsGoal::isCompleted() const {
    return currentAmount >= targetAmount;
}

void SavingsGoal::addMoney(double amount) {
    if (amount <= 0.0) {
        throw ValidationException("Amount to add must be positive.");
    }
    currentAmount += amount;
}

void SavingsGoal::withdrawMoney(double amount) {
    if (amount <= 0.0) {
        throw ValidationException("Amount to withdraw must be positive.");
    }
    if (amount > currentAmount) {
        throw ValidationException("Cannot withdraw more than current savings.");
    }
    currentAmount -= amount;
}

crow::json::wvalue SavingsGoal::toJson() const {
    crow::json::wvalue val;
    val["id"] = id;
    val["_id"] = id;
    val["userId"] = userId;
    val["name"] = name;
    val["targetAmount"] = targetAmount;
    val["currentAmount"] = currentAmount;
    val["remaining"] = getRemaining();
    val["percentage"] = getPercentage();
    val["targetDate"] = targetDate;
    val["category"] = category;
    val["isCompleted"] = isCompleted();
    val["createdAt"] = createdAt;
    return val;
}

