#include <iostream>
#include <memory>
#include <string>

#include "../include/crow.h"
#include "../include/database/DatabaseManager.h"
#include "../include/services/AuthenticationManager.h"
#include "../include/services/FinanceManager.h"
#include "../include/services/ReportManager.h"
#include "../include/routes/AuthRoutes.h"
#include "../include/routes/TransactionRoutes.h"
#include "../include/routes/BudgetRoutes.h"
#include "../include/routes/SavingsRoutes.h"

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    std::cout << "================================================================" << std::endl;
    std::cout << "    FINTRACK - SMART PERSONAL FINANCE MANAGEMENT SYSTEM         " << std::endl;
    std::cout << "    C++ OOPS Backend with Crow REST APIs and local JSON storage    " << std::endl;
    std::cout << "================================================================" << std::endl;

    // Phase 4: Database Manager initialization
    auto dbManager = std::make_shared<DatabaseManager>("database/data");
    std::cout << "[Database] Initialized local JSON repositories (MongoDB integration planned)." << std::endl;

    // Phase 3 & Phase 7: Authentication Manager
    auto authManager = std::make_shared<AuthenticationManager>(dbManager);

    // Phase 3 & Phase 8: Finance Manager (Dynamic Polymorphism)
    auto financeManager = std::make_shared<FinanceManager>(dbManager);

    // Phase 8: Financial Report Manager
    auto reportManager = std::make_shared<ReportManager>(dbManager, financeManager);

    // Crow Application setup
    crow::SimpleApp app;
    app.port(port).multithreaded();
    app.static_dir_path("frontend");

    // Phase 5: REST API Route Registrations
    AuthRoutes authRoutes(authManager);
    authRoutes.registerRoutes(app);

    TransactionRoutes txRoutes(authManager, financeManager, reportManager);
    txRoutes.registerRoutes(app);

    BudgetRoutes budgetRoutes(authManager, financeManager);
    budgetRoutes.registerRoutes(app);

    SavingsRoutes savingsRoutes(authManager, financeManager);
    savingsRoutes.registerRoutes(app);

    // Add Demo Data Reset Route
    CROW_ROUTE(app, "/api/demo/reset").methods(crow::HTTPMethod::POST)([dbManager](const crow::request& req) {
        dbManager->resetDemoData("65e8a1f2b3c4d5e6f7a8b901");
        crow::json::wvalue res;
        res["status"] = "success";
        res["message"] = "Demo data restored successfully to baseline realistic values.";
        return crow::response(200, res);
    });

    std::cout << "\n[Demo Credentials]" << std::endl;
    std::cout << "  Email:    demo@fintrack.com" << std::endl;
    std::cout << "  Password: Demo@123" << std::endl;
    std::cout << "\n[Server Ready]" << std::endl;
    std::cout << "  Frontend: http://localhost:" << port << "/" << std::endl;
    std::cout << "  REST API: http://localhost:" << port << "/api/dashboard" << std::endl;
    std::cout << "================================================================\n" << std::endl;

    app.run();
    return 0;
}

