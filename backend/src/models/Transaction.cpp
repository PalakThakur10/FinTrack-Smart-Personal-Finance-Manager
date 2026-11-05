#include "../../include/models/Transaction.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

Transaction::Transaction()
    : id(""), userId(""), amount(0.0), category("Other"),
      description(""), date(""), paymentMethod("UPI"),
      type("general"), createdAt("") {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    createdAt = ss.str();
}

Transaction::Transaction(const std::string& id,
                         const std::string& userId,
                         double amount,
                         const std::string& category,
                         const std::string& description,
                         const std::string& date,
                         const std::string& paymentMethod,
                         const std::string& type,
                         const std::string& createdAt)
    : id(id), userId(userId), category(category),
      description(description), date(date),
      paymentMethod(paymentMethod), type(type) {
    setAmount(amount);
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

void Transaction::setAmount(double newAmount) {
    if (newAmount <= 0.0) {
        throw ValidationException("Transaction amount must be greater than zero.");
    }
    amount = newAmount;
}

void Transaction::setCategory(const std::string& newCategory) {
    if (newCategory.empty()) {
        throw ValidationException("Transaction category cannot be empty.");
    }
    category = newCategory;
}

void Transaction::setDate(const std::string& newDate) {
    if (newDate.empty()) {
        throw ValidationException("Transaction date cannot be empty.");
    }
    date = newDate;
}

crow::json::wvalue Transaction::toJson() const {
    crow::json::wvalue val;
    val["id"] = id;
    val["_id"] = id;
    val["userId"] = userId;
    val["amount"] = amount;
    val["category"] = category;
    val["description"] = description;
    val["date"] = date;
    val["paymentMethod"] = paymentMethod;
    val["type"] = type;
    val["createdAt"] = createdAt;
    return val;
}

