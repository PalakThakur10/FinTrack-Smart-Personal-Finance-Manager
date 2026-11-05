#pragma once

#ifndef AUTHENTICATION_MANAGER_H
#define AUTHENTICATION_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include "../database/DatabaseManager.h"
#include "../models/User.h"

class AuthenticationException : public std::exception {
private:
    std::string message;
public:
    explicit AuthenticationException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

struct Session {
    std::string userId;
    std::string userEmail;
    std::string userName;
    std::chrono::system_clock::time_point expiresAt;
};

/**
 * @brief Manages authentication, password hashing, session tokens, and security.
 * Demonstrates: Association with DatabaseManager, Encapsulation, Exception Handling.
 */
class AuthenticationManager {
private:
    std::shared_ptr<DatabaseManager> db;
    mutable std::mutex sessionMutex;
    std::unordered_map<std::string, Session> activeSessions;

    std::string generateSessionToken(const std::string& userId);

public:
    explicit AuthenticationManager(std::shared_ptr<DatabaseManager> dbManager);
    ~AuthenticationManager() = default;

    // Core Auth Operations
    crow::json::wvalue registerUser(const std::string& name,
                                    const std::string& email,
                                    const std::string& password,
                                    const std::string& confirmPassword);

    std::pair<std::string, crow::json::wvalue> loginUser(const std::string& email,
                                                         const std::string& password);

    bool validateToken(const std::string& token, std::string& outUserId) const;
    std::shared_ptr<User> getAuthenticatedUser(const std::string& token) const;
    void logout(const std::string& token);
};

#endif // AUTHENTICATION_MANAGER_H

