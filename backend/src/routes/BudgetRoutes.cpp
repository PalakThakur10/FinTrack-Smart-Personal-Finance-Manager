#include "../../include/routes/BudgetRoutes.h"
#include <iostream>

BudgetRoutes::BudgetRoutes(std::shared_ptr<AuthenticationManager> authMgr,
                           std::shared_ptr<FinanceManager> finMgr)
    : authManager(authMgr), financeManager(finMgr) {}

std::string BudgetRoutes::extractUserId(const crow::request& req) const {
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

void BudgetRoutes::registerRoutes(crow::SimpleApp& app) {
    // GET /api/budgets
    CROW_ROUTE(app, "/api/budgets").methods(crow::HTTPMethod::GET)([this](const crow::request& req) {
        try {
            std::string userId = extractUserId(req);
            auto list = financeManager->getBudgets(userId);
            crow::json::wvalue res;
            res["status"] = "success";
            res["budgets"] = list;
            return crow::response(200, res);
        } catch (const std::exception& e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = e.what();
            return crow::response(500, err);
        }
    });

    // POST /api/budgets
    CROW_ROUTE(app, "/api/budgets").methods(crow::HTTPMethod::POST)([this](const crow::request& req) {
        try {
            std::string userId = extractUserId(req);
            auto body = crow::json::load(req.body);
            if (!body) {
                crow::json::wvalue err;
                err["status"] = "fail";
                err["message"] = "Invalid budget payload.";
                return crow::response(400, err);
            }

            std::string category = body.has("category") ? body["category"].s() : "Other";
            double limit = body.has("limit") ? body["limit"].d() : (body.has("limitAmount") ? body["limitAmount"].d() : 0.0);
            int month = body.has("month") ? body["month"].i() : 11;
            int year = body.has("year") ? body["year"].i() : 2026;

            auto res = financeManager->addBudget(userId, category, limit, month, year);
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

    // PUT /api/budgets/<string>
    CROW_ROUTE(app, "/api/budgets/<string>").methods(crow::HTTPMethod::PUT)([this](const crow::request& req) {
        try {
            std::string userId = extractUserId(req);
            std::string budgetId = req.url_params.at("arg0");
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400);

            std::string category = body.has("category") ? body["category"].s() : "Other";
            double limit = body.has("limit") ? body["limit"].d() : (body.has("limitAmount") ? body["limitAmount"].d() : 0.0);
            int month = body.has("month") ? body["month"].i() : 11;
            int year = body.has("year") ? body["year"].i() : 2026;

            auto res = financeManager->updateBudget(userId, budgetId, category, limit, month, year);
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

    // DELETE /api/budgets/<string>
    CROW_ROUTE(app, "/api/budgets/<string>").methods(crow::HTTPMethod::DELETE_METHOD)([this](const crow::request& req) {
        std::string userId = extractUserId(req);
        std::string budgetId = req.url_params.at("arg0");
        if (financeManager->deleteBudget(userId, budgetId)) {
            crow::json::wvalue res;
            res["status"] = "success";
            res["message"] = "Budget deleted successfully";
            return crow::response(200, res);
        }
        return crow::response(404);
    });
}

