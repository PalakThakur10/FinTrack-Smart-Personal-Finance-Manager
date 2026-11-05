#pragma once

#ifndef REPORT_MANAGER_H
#define REPORT_MANAGER_H

#include <string>
#include <memory>
#include "../database/DatabaseManager.h"
#include "../services/FinanceManager.h"
#include "../crow.h"

/**
 * @brief Generates comprehensive financial statements, audits, and performance reports.
 * Demonstrates: Association with FinanceManager & DatabaseManager, Analytical Processing.
 */
class ReportManager {
private:
    std::shared_ptr<DatabaseManager> db;
    std::shared_ptr<FinanceManager> finance;

public:
    ReportManager(std::shared_ptr<DatabaseManager> dbManager,
                  std::shared_ptr<FinanceManager> finManager);
    ~ReportManager() = default;

    crow::json::wvalue generateReport(const std::string& userId,
                                      const std::string& period = "monthly",
                                      const std::string& year = "2026",
                                      const std::string& monthOrQuarter = "11") const;
};

#endif // REPORT_MANAGER_H

