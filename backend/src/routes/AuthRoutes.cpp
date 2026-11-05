#include "../../include/routes/AuthRoutes.h"
#include <iostream>

AuthRoutes::AuthRoutes(std::shared_ptr<AuthenticationManager> authMgr)
    : authManager(authMgr) {}

void AuthRoutes::registerRoutes(crow::SimpleApp& app) {
    // POST /api/auth/register
    CROW_ROUTE(app, "/api/auth/register").methods(crow::HTTPMethod::POST)([this](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) {
                crow::json::wvalue err;
                err["status"] = "fail";
                err["message"] = "Invalid JSON payload provided.";
                return crow::response(400, err);
            }

            std::string name = body.has("name") ? body["name"].s() : "";
            std::string email = body.has("email") ? body["email"].s() : "";
            std::string password = body.has("password") ? body["password"].s() : "";
            std::string confirmPassword = body.has("confirmPassword") ? body["confirmPassword"].s() : password;

            auto res = authManager->registerUser(name, email, password, confirmPassword);
            return crow::response(201, res);
        } catch (const ValidationException& e) {
            crow::json::wvalue err;
            err["status"] = "fail";
            err["message"] = e.what();
            return crow::response(400, err);
        } catch (const std::exception& e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = e.what();
            return crow::response(500, err);
        }
    });

    // POST /api/auth/login
    CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::POST)([this](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body) {
                crow::json::wvalue err;
                err["status"] = "fail";
                err["message"] = "Invalid JSON payload provided.";
                return crow::response(400, err);
            }

            std::string email = body.has("email") ? body["email"].s() : "";
            std::string password = body.has("password") ? body["password"].s() : "";

            auto [token, res] = authManager->loginUser(email, password);
            return crow::response(200, res);
        } catch (const ValidationException& e) {
            crow::json::wvalue err;
            err["status"] = "fail";
            err["message"] = e.what();
            return crow::response(400, err);
        } catch (const AuthenticationException& e) {
            crow::json::wvalue err;
            err["status"] = "fail";
            err["message"] = e.what();
            return crow::response(401, err);
        } catch (const std::exception& e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = "Something went wrong. Please try again.";
            return crow::response(500, err);
        }
    });

    // POST /api/auth/logout
    CROW_ROUTE(app, "/api/auth/logout").methods(crow::HTTPMethod::POST)([this](const crow::request& req) {
        std::string authHeader = req.get_header_value("Authorization");
        std::string token;
        if (authHeader.rfind("Bearer ", 0) == 0) {
            token = authHeader.substr(7);
        } else {
            token = authHeader;
        }

        if (!token.empty()) {
            authManager->logout(token);
        }

        crow::json::wvalue res;
        res["status"] = "success";
        res["message"] = "Logged out successfully";
        return crow::response(200, res);
    });

    // GET /api/auth/me
    CROW_ROUTE(app, "/api/auth/me").methods(crow::HTTPMethod::GET)([this](const crow::request& req) {
        std::string authHeader = req.get_header_value("Authorization");
        std::string token;
        if (authHeader.rfind("Bearer ", 0) == 0) {
            token = authHeader.substr(7);
        } else {
            token = authHeader;
        }

        auto user = authManager->getAuthenticatedUser(token);
        if (!user) {
            crow::json::wvalue err;
            err["status"] = "fail";
            err["message"] = "Unauthorized. Please log in.";
            return crow::response(401, err);
        }

        crow::json::wvalue res;
        res["status"] = "success";
        res["user"] = user->toJson(false);
        return crow::response(200, res);
    });
}

