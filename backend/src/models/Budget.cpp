#include "../../include/models/Budget.h"
#include "../../include/models/Transaction.h" // For ValidationException
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

Budget::Budget()
    : id(""), userId(""), category("Other"), limitAmount(1000.0),
      spentAmount(0.0), month(1), year(2026), createdAt("") {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    createdAt = ss.str();
}

Budget::Budget(const std::string& id,
               const std::string& userId,
               const std::string& category,
               double limitAmount,
               double spentAmount,
               int month,
               int year,
               const std::string& createdAt)
    : id(id), userId(userId), month(month), year(year) {
    setCategory(category);
    setLimitAmount(limitAmount);
    setSpentAmount(spentAmount);

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

void Budget::setCategory(const std::string& newCategory) {
    if (newCategory.empty()) {
        throw ValidationException("Budget category cannot be empty.");
    }
    category = newCategory;
}

void Budget::setLimitAmount(double newLimit) {
    if (newLimit <= 0.0) {
        throw ValidationException("Budget limit must be greater than zero.");
    }
    limitAmount = newLimit;
}

void Budget::setSpentAmount(double newSpent) {
    spentAmount = (newSpent < 0.0) ? 0.0 : newSpent;
}

double Budget::getPercentage() const {
    if (limitAmount <= 0.0) return 0.0;
    return (spentAmount / limitAmount) * 100.0;
}

double Budget::getRemaining() const {
    double rem = limitAmount - spentAmount;
    return (rem > 0.0) ? rem : 0.0;
}

bool Budget::isApproachingLimit(double threshold) const {
    double pct = getPercentage() / 100.0;
    return (pct >= threshold && pct <= 1.0);
}

bool Budget::isExceeded() const {
    return spentAmount > limitAmount;
}

std::string Budget::getStatus() const {
    if (isExceeded()) return "Exceeded";
    if (isApproachingLimit(0.80)) return "Warning";
    return "Safe";
}

void Budget::addSpending(double amount) {
    if (amount > 0.0) {
        spentAmount += amount;
    }
}

void Budget::resetSpending() {
    spentAmount = 0.0;
}

crow::json::wvalue Budget::toJson() const {
    crow::json::wvalue val;
    val["id"] = id;
    val["_id"] = id;
    val["userId"] = userId;
    val["category"] = category;
    val["limit"] = limitAmount;
    val["limitAmount"] = limitAmount;
    val["spent"] = spentAmount;
    val["spentAmount"] = spentAmount;
    val["remaining"] = getRemaining();
    val["percentage"] = getPercentage();
    val["status"] = getStatus();
    val["isApproachingLimit"] = isApproachingLimit();
    val["isExceeded"] = isExceeded();
    val["month"] = month;
    val["year"] = year;
    val["createdAt"] = createdAt;
    return val;
}

