#include "../../include/models/Income.h"
#include <iostream>

Income::Income()
    : Transaction("", "", 0.0, "Income", "Income deposit", "", "Bank Transfer", "income"),
      source("Salary"), isRecurring(false) {}

Income::Income(const std::string& id,
               const std::string& userId,
               double amount,
               const std::string& category,
               const std::string& description,
               const std::string& date,
               const std::string& paymentMethod,
               const std::string& source,
               bool isRecurring,
               const std::string& createdAt)
    : Transaction(id, userId, amount, category, description, date, paymentMethod, "income", createdAt),
      source(source.empty() ? category : source),
      isRecurring(isRecurring) {}

void Income::display() const {
    std::cout << "[INCOME] ID: " << id
              << " | Source: " << source
              << " | Category: " << category
              << " | Amount: +" << amount
              << " | Date: " << date << std::endl;
}

double Income::calculateImpact() const {
    // Inflow increases total balance
    return amount;
}

std::unique_ptr<Transaction> Income::clone() const {
    return std::make_unique<Income>(*this);
}

crow::json::wvalue Income::toJson() const {
    crow::json::wvalue val = Transaction::toJson();
    val["source"] = source;
    val["isRecurring"] = isRecurring;
    return val;
}

