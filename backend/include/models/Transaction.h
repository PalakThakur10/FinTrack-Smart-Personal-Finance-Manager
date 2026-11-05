#pragma once

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <memory>
#include "../crow.h"

// Custom Exceptions
class ValidationException : public std::exception {
private:
    std::string message;
public:
    explicit ValidationException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class NotFoundException : public std::exception {
private:
    std::string message;
public:
    explicit NotFoundException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

/**
 * @brief Abstract Base Class representing a financial transaction.
 * Demonstrates: Abstraction, Encapsulation, Polymorphism, Virtual Destructor.
 */
class Transaction {
protected:
    std::string id;
    std::string userId;
    double amount;
    std::string category;
    std::string description;
    std::string date;
    std::string paymentMethod; // Cash, UPI, Debit Card, Credit Card, Bank Transfer
    std::string type;          // "income" or "expense"
    std::string createdAt;

public:
    // Constructors (Demonstrating Constructor Overloading)
    Transaction();
    Transaction(const std::string& id,
                const std::string& userId,
                double amount,
                const std::string& category,
                const std::string& description,
                const std::string& date,
                const std::string& paymentMethod,
                const std::string& type,
                const std::string& createdAt = "");

    // Virtual Destructor (Crucial for polymorphic base classes)
    virtual ~Transaction() = default;

    // Pure Virtual Functions (Abstraction & Dynamic Polymorphism)
    virtual void display() const = 0;
    virtual double calculateImpact() const = 0;
    virtual std::unique_ptr<Transaction> clone() const = 0;

    // Virtual Method for JSON serialization
    virtual crow::json::wvalue toJson() const;

    // Encapsulation: Getters
    std::string getId() const { return id; }
    std::string getUserId() const { return userId; }
    double getAmount() const { return amount; }
    std::string getCategory() const { return category; }
    std::string getDescription() const { return description; }
    std::string getDate() const { return date; }
    std::string getPaymentMethod() const { return paymentMethod; }
    std::string getType() const { return type; }
    std::string getCreatedAt() const { return createdAt; }

    // Encapsulation: Setters with Validation
    void setId(const std::string& newId) { id = newId; }
    void setUserId(const std::string& newUserId) { userId = newUserId; }
    void setAmount(double newAmount);
    void setCategory(const std::string& newCategory);
    void setDescription(const std::string& newDescription) { description = newDescription; }
    void setDate(const std::string& newDate);
    void setPaymentMethod(const std::string& newPaymentMethod) { paymentMethod = newPaymentMethod; }
    void setCreatedAt(const std::string& newCreatedAt) { createdAt = newCreatedAt; }
};

#endif // TRANSACTION_H

