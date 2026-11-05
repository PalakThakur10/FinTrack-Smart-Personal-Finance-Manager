#pragma once

#ifndef USER_H
#define USER_H

#include <string>
#include "../crow.h"

/**
 * @brief Represents a registered system user.
 * Demonstrates: Encapsulation, Secure Data Handling.
 */
class User {
private:
    std::string id;
    std::string name;
    std::string email;
    std::string passwordHash;
    std::string salt;
    std::string currency;
    std::string createdAt;

public:
    User();
    User(const std::string& id,
         const std::string& name,
         const std::string& email,
         const std::string& passwordHash,
         const std::string& salt = "",
         const std::string& currency = "INR",
         const std::string& createdAt = "");

    // Encapsulation: Getters
    std::string getId() const { return id; }
    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    std::string getPasswordHash() const { return passwordHash; }
    std::string getSalt() const { return salt; }
    std::string getCurrency() const { return currency; }
    std::string getCreatedAt() const { return createdAt; }

    // Encapsulation: Setters with Validation
    void setId(const std::string& newId) { id = newId; }
    void setName(const std::string& newName);
    void setEmail(const std::string& newEmail);
    void setPasswordHash(const std::string& hash) { passwordHash = hash; }
    void setSalt(const std::string& newSalt) { salt = newSalt; }
    void setCurrency(const std::string& cur) { currency = cur; }
    void setCreatedAt(const std::string& created) { createdAt = created; }

    // Password validation
    bool verifyPassword(const std::string& plainPassword) const;
    void hashAndSetPassword(const std::string& plainPassword);

    crow::json::wvalue toJson(bool includeHash = false) const;
};

#endif // USER_H

