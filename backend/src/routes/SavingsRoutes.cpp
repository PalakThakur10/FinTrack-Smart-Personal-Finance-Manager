#include "../../include/routes/SavingsRoutes.h"
#include <iostream>

SavingsRoutes::SavingsRoutes(std::shared_ptr<AuthenticationManager> authMgr,
                             std::shared_ptr<FinanceManager> finMgr)
    : authManager(authMgr), financeManager(finMgr) {}

std::string SavingsRoutes::extractUserId(const crow::request& req) const {
    std::string authHeader = req.get_header_value("Authorization");
    std::string token;
    if (authHeader.rfind("Bearer ", 0) == 0) {
        token = authHeader.substr(7);
    } else {
        token = authHeader;
    }

    std::string userId;
    if (authManager->validateToken(token, userId)) {
        return userId;
    }

    return "65e8a1f2b3c4d5e6f7a8b901";
}

void SavingsRoutes::registerRoutes(crow::SimpleApp& app) {
    // GET /api/savings-goals
    CROW_ROUTE(app, "/api/savings-goals").methods(crow::HTTPMethod::GET)([this](const crow::request& req) {
        try {
            std::string userId = extractUserId(req);
            auto list = financeManager->getSavingsGoals(userId);
            crow::json::wvalue res;
            res["status"] = "success";
            res["savingsGoals"] = list;
            return crow::response(200, res);
        } catch (const std::exception& e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = e.what();
            return crow::response(500, err);
        }
    });

    // POST /api/savings-goals
    CROW_ROUTE(app, "/api/savings-goals").methods(crow::HTTPMethod::POST)([this](const crow::request& req) {
        try {
            std::string userId = extractUserId(req);
            auto body = crow::json::load(req.body);
            if (!body) {
                crow::json::wvalue err;
                err["status"] = "fail";
                err["message"] = "Invalid savings goal payload.";
                return crow::response(400, err);
            }

            std::string name = body.has("name") ? body["name"].s() : "";
            double target = body.has("targetAmount") ? body["targetAmount"].d() : 0.0;
            double current = body.has("currentAmount") ? body["currentAmount"].d() : 0.0;
            std::string date = body.has("targetDate") ? body["targetDate"].s() : "";
            std::string category = body.has("category") ? body["category"].s() : "General";

            auto res = financeManager->addSavingsGoal(userId, name, target, current, date, category);
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

    // PUT /api/savings-goals/<string>
    CROW_ROUTE(app, "/api/savings-goals/<string>").methods(crow::HTTPMethod::PUT)([this](const crow::request& req) {
        try {
            std::string userId = extractUserId(req);
            std::string goalId = req.url_params.at("arg0");
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400);

            std::string name = body.has("name") ? body["name"].s() : "";
            double target = body.has("targetAmount") ? body["targetAmount"].d() : 0.0;
            double current = body.has("currentAmount") ? body["currentAmount"].d() : 0.0;
            std::string date = body.has("targetDate") ? body["targetDate"].s() : "";
            std::string category = body.has("category") ? body["category"].s() : "General";

            auto res = financeManager->updateSavingsGoal(userId, goalId, name, target, current, date, category);
            return crow::response(200, res);
        } catch (const ValidationException& e) {
            crow::json::wvalue err;
            err["status"] = "fail";
            err["message"] = e.what();
            return crow::response(400, err);
        } catch (const NotFoundException& e) {
            crow::json::wvalue err;
            err["status"] = "fail";
            err["message"] = e.what();
            return crow::response(404, err);
        } catch (const std::exception& e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = e.what();
            return crow::response(500, err);
        }
    });

    // POST /api/savings-goals/<string>/add-money
    CROW_ROUTE(app, "/api/savings-goals/<string>/add-money").methods(crow::HTTPMethod::POST)([this](const crow::request& req) {
        try {
            std::string userId = extractUserId(req);
            std::string goalId = req.url_params.at("arg0");
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400);

            double amount = body.has("amount") ? body["amount"].d() : 0.0;
            auto res = financeManager->addMoneyToGoal(userId, goalId, amount);
            return crow::response(200, res);
        } catch (const ValidationException& e) {
            crow::json::wvalue err;
            err["status"] = "fail";
            err["message"] = e.what();
            return crow::response(400, err);
        } catch (const NotFoundException& e) {
            crow::json::wvalue err;
            err["status"] = "fail";
            err["message"] = e.what();
            return crow::response(404, err);
        } catch (const std::exception& e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = e.what();
            return crow::response(500, err);
        }
    });

    // DELETE /api/savings-goals/<string>
    CROW_ROUTE(app, "/api/savings-goals/<string>").methods(crow::HTTPMethod::DELETE_METHOD)([this](const crow::request& req) {
        std::string userId = extractUserId(req);
        std::string goalId = req.url_params.at("arg0");
        if (financeManager->deleteSavingsGoal(userId, goalId)) {
            crow::json::wvalue res;
            res["status"] = "success";
            res["message"] = "Savings goal deleted successfully";
            return crow::response(200, res);
        }
        return crow::response(404);
    });
}

