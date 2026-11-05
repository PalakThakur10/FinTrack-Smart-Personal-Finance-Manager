#pragma once

#ifndef AUTH_ROUTES_H
#define AUTH_ROUTES_H

#include "../crow.h"
#include "../services/AuthenticationManager.h"

class AuthRoutes {
private:
    std::shared_ptr<AuthenticationManager> authManager;

public:
    explicit AuthRoutes(std::shared_ptr<AuthenticationManager> authMgr);
    void registerRoutes(crow::SimpleApp& app);
};

#endif // AUTH_ROUTES_H

