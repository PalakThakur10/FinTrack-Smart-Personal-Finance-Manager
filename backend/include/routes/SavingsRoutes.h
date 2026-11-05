#pragma once

#ifndef SAVINGS_ROUTES_H
#define SAVINGS_ROUTES_H

#include "../crow.h"
#include "../services/AuthenticationManager.h"
#include "../services/FinanceManager.h"

class SavingsRoutes {
private:
    std::shared_ptr<AuthenticationManager> authManager;
    std::shared_ptr<FinanceManager> financeManager;

    std::string extractUserId(const crow::request& req) const;

public:
    SavingsRoutes(std::shared_ptr<AuthenticationManager> authMgr,
                  std::shared_ptr<FinanceManager> finMgr);

    void registerRoutes(crow::SimpleApp& app);
};

#endif // SAVINGS_ROUTES_H

