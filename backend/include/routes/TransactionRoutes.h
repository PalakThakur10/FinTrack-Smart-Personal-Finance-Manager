#pragma once

#ifndef TRANSACTION_ROUTES_H
#define TRANSACTION_ROUTES_H

#include "../crow.h"
#include "../services/AuthenticationManager.h"
#include "../services/FinanceManager.h"
#include "../services/ReportManager.h"

class TransactionRoutes {
private:
    std::shared_ptr<AuthenticationManager> authManager;
    std::shared_ptr<FinanceManager> financeManager;
    std::shared_ptr<ReportManager> reportManager;

    std::string extractUserId(const crow::request& req) const;

public:
    TransactionRoutes(std::shared_ptr<AuthenticationManager> authMgr,
                      std::shared_ptr<FinanceManager> finMgr,
                      std::shared_ptr<ReportManager> repMgr);

    void registerRoutes(crow::SimpleApp& app);
};

#endif // TRANSACTION_ROUTES_H

