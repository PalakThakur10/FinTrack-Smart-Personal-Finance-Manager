#include "../../include/models/Expense.h"
#include <iostream>

Expense::Expense()
    : Transaction("", "", 0.0, "Expense", "Expense withdrawal", "", "UPI", "expense"),
      merchant(""), isTaxDeductible(false) {}

Expense::Expense(const std::string& id,
                 const std::string& userId,
                 double amount,
                 const std::string& category,
                 const std::string& description,
                 const std::string& date,
                 const std::string& paymentMethod,
                 const std::string& merchant,
                 bool isTaxDeductible,
                 const std::string& createdAt)
    : Transaction(id, userId, amount, category, description, date, paymentMethod, "expense", createdAt),
      merchant(merchant),
      isTaxDeductible(isTaxDeductible) {}

void Expense::display() const {
    std::cout << "[EXPENSE] ID: " << id
              << " | Merchant: " << merchant
              << " | Category: " << category
              << " | Amount: -" << amount
              << " | Date: " << date << std::endl;
}

double Expense::calculateImpact() const {
    // Outflow decreases total balance
    return -amount;
}

std::unique_ptr<Transaction> Expense::clone() const {
    return std::make_unique<Expense>(*this);
}

crow::json::wvalue Expense::toJson() const {
    crow::json::wvalue val = Transaction::toJson();
    val["merchant"] = merchant;
    val["isTaxDeductible"] = isTaxDeductible;
    return val;
}

