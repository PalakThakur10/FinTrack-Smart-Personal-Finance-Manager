#pragma once

#ifndef EXPENSE_H
#define EXPENSE_H

#include "Transaction.h"

/**
 * @brief Represents an Expense transaction.
 * Demonstrates: Inheritance, Method Overriding, Polymorphism.
 */
class Expense : public Transaction {
private:
    std::string merchant;
    bool isTaxDeductible;

public:
    // Constructors (Constructor Chaining / Base class initialization)
    Expense();
    Expense(const std::string& id,
            const std::string& userId,
            double amount,
            const std::string& category,
            const std::string& description,
            const std::string& date,
            const std::string& paymentMethod,
            const std::string& merchant = "",
            bool isTaxDeductible = false,
            const std::string& createdAt = "");

    // Overridden methods (Dynamic Polymorphism)
    void display() const override;
    double calculateImpact() const override; // Returns -amount
    std::unique_ptr<Transaction> clone() const override;
    crow::json::wvalue toJson() const override;

    // Getters and Setters
    std::string getMerchant() const { return merchant; }
    void setMerchant(const std::string& newMerchant) { merchant = newMerchant; }
    bool getIsTaxDeductible() const { return isTaxDeductible; }
    void setIsTaxDeductible(bool taxDeductible) { isTaxDeductible = taxDeductible; }
};

#endif // EXPENSE_H

