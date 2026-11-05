#include "../../include/services/AuthenticationManager.h"
#include <random>
#include <sstream>
#include <iomanip>

AuthenticationManager::AuthenticationManager(std::shared_ptr<DatabaseManager> dbManager)
    : db(dbManager) {}

std::string AuthenticationManager::generateSessionToken(const std::string& userId) {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;

    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::ostringstream ss;
    ss << "ftk_" << std::hex << now << "_" << dis(gen) << "_" << dis(gen);
    return ss.str();
}

crow::json::wvalue AuthenticationManager::registerUser(const std::string& name,
                                                       const std::string& email,
                                                       const std::string& password,
                                                       const std::string& confirmPassword) {
    if (name.empty()) {
        throw ValidationException("Full name is required.");
    }
    if (email.empty()) {
        throw ValidationException("Email address is required.");
    }
    if (password.empty() || password.length() < 6) {
        throw ValidationException("Password must be at least 6 characters long.");
    }
    if (password != confirmPassword) {
        throw ValidationException("Passwords do not match.");
    }

    if (db->findUserByEmail(email)) {
        throw ValidationException("An account with this email address already exists.");
    }

    User newUser;
    newUser.setName(name);
    newUser.setEmail(email);
    newUser.hashAndSetPassword(password);
    newUser.setCurrency("INR");

    if (!db->insertUser(newUser)) {
        throw AuthenticationException("Unable to complete user registration.");
    }

    auto createdUser = db->findUserByEmail(email);
    if (!createdUser) {
        throw AuthenticationException("User registration verification failed.");
    }

    // Auto-login session token
    std::string token = generateSessionToken(createdUser->getId());
    Session sess;
    sess.userId = createdUser->getId();
    sess.userEmail = createdUser->getEmail();
    sess.userName = createdUser->getName();
    sess.expiresAt = std::chrono::system_clock::now() + std::chrono::hours(24 * 7);

    {
        std::lock_guard<std::mutex> lock(sessionMutex);
        activeSessions[token] = sess;
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["message"] = "Registration successful";
    res["token"] = token;
    res["user"] = createdUser->toJson(false);
    return res;
}

std::pair<std::string, crow::json::wvalue> AuthenticationManager::loginUser(const std::string& email,
                                                                            const std::string& password) {
    if (email.empty() || password.empty()) {
        throw ValidationException("Invalid email or password.");
    }

    auto user = db->findUserByEmail(email);
    if (!user) {
        throw AuthenticationException("Invalid email or password.");
    }

    if (!user->verifyPassword(password)) {
        throw AuthenticationException("Invalid email or password.");
    }

    std::string token = generateSessionToken(user->getId());
    Session sess;
    sess.userId = user->getId();
    sess.userEmail = user->getEmail();
    sess.userName = user->getName();
    sess.expiresAt = std::chrono::system_clock::now() + std::chrono::hours(24 * 7);

    {
        std::lock_guard<std::mutex> lock(sessionMutex);
        activeSessions[token] = sess;
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["message"] = "Login successful";
    res["token"] = token;
    res["user"] = user->toJson(false);
    return {token, res};
}

bool AuthenticationManager::validateToken(const std::string& token, std::string& outUserId) const {
    if (token.empty()) return false;
    std::lock_guard<std::mutex> lock(sessionMutex);
    auto it = activeSessions.find(token);
    if (it != activeSessions.end()) {
        if (std::chrono::system_clock::now() < it->second.expiresAt) {
            outUserId = it->second.userId;
            return true;
        }
    }
    return false;
}

std::shared_ptr<User> AuthenticationManager::getAuthenticatedUser(const std::string& token) const {
    std::string userId;
    if (validateToken(token, userId)) {
        return db->findUserById(userId);
    }
    return nullptr;
}

void AuthenticationManager::logout(const std::string& token) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    activeSessions.erase(token);
}

