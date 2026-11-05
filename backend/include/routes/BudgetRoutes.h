#pragma once

#ifndef BUDGET_ROUTES_H
#define BUDGET_ROUTES_H

#include "../crow.h"
#include "../services/AuthenticationManager.h"
#include "../services/FinanceManager.h"

class BudgetRoutes {
private:
    std::shared_ptr<AuthenticationManager> authManager;
    std::shared_ptr<FinanceManager> financeManager;

    std::string extractUserId(const crow::request& req) const;

public:
    BudgetRoutes(std::shared_ptr<AuthenticationManager> authMgr,
                 std::shared_ptr<FinanceManager> finMgr);

    void registerRoutes(crow::SimpleApp& app);
};

#endif // BUDGET_ROUTES_H

