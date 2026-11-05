#pragma once

#ifndef INCOME_H
#define INCOME_H

#include "Transaction.h"

/**
 * @brief Represents an Income transaction.
 * Demonstrates: Inheritance, Method Overriding, Polymorphism.
 */
class Income : public Transaction {
private:
    std::string source;      // Salary, Freelance, Business, Scholarship, Investment, Other
    bool isRecurring;

public:
    // Constructors (Constructor Chaining / Base class initialization)
    Income();
    Income(const std::string& id,
           const std::string& userId,
           double amount,
           const std::string& category,
           const std::string& description,
           const std::string& date,
           const std::string& paymentMethod,
           const std::string& source = "Salary",
           bool isRecurring = false,
           const std::string& createdAt = "");

    // Overridden methods (Dynamic Polymorphism)
    void display() const override;
    double calculateImpact() const override; // Returns +amount
    std::unique_ptr<Transaction> clone() const override;
    crow::json::wvalue toJson() const override;

    // Getters and Setters
    std::string getSource() const { return source; }
    void setSource(const std::string& newSource) { source = newSource; }
    bool getIsRecurring() const { return isRecurring; }
    void setIsRecurring(bool recurring) { isRecurring = recurring; }
};

#endif // INCOME_H

