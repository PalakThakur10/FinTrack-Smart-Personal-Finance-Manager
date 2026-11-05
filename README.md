# 💰 FINTRACK — Smart Personal Finance Management System

> **University Mini Project | Object-Oriented Programming (C++)**  
> Full-Stack Web Application demonstrating core OOP concepts with a C++17 backend and modern frontend. MongoDB integration is planned as a next development phase.

---

## 🚀 How to Run (Instant — No Setup Required)

**For the current demo, open this file in your browser:**

```
frontend/login.html
```

Or double-click **`OPEN_DEMO.bat`** — it opens the app automatically.

**Demo Login:**
| Field | Value |
|-------|-------|
| Email | `demo@fintrack.com` |
| Password | `Demo@123` |

> The app runs fully with built-in demo data — no server or database needed.

---

## 🏗️ Technology Stack

| Layer | Technology |
|-------|-----------|
| **Frontend** | HTML5, CSS3, Vanilla JavaScript, Chart.js |
| **Backend** | C++17, Crow HTTP Framework (REST API) |
| **Storage** | Local JSON/file persistence (MongoDB integration in progress) |
| **API** | RESTful JSON APIs over HTTP |

---

## 📐 OOP Concepts Demonstrated

| Concept | Implementation |
|---------|---------------|
| **Abstract Class** | `Transaction` — pure virtual `display()` and `calculateImpact()` |
| **Inheritance** | `Income` and `Expense` extend `Transaction` |
| **Polymorphism** | `std::vector<std::shared_ptr<Transaction>>` with virtual dispatch |
| **Encapsulation** | All models use private fields with getters/setters |
| **Abstraction** | Manager classes hide implementation details behind clean interfaces |
| **Composition** | `FinanceManager` owns `DatabaseManager`, `AuthenticationManager`, `ReportManager` |
| **Exception Handling** | Custom `ValidationException`, `AuthException`, `DatabaseException` |
| **Constructors/Destructors** | All model classes have parameterized constructors and virtual destructors |
| **Function Overloading** | Multiple `addTransaction()` overloads in `FinanceManager` |
| **Method Overriding** | `display()` and `calculateImpact()` overridden in `Income` and `Expense` |

---

## 📁 Project Structure

```
FinTrack/
├── frontend/                   # HTML/CSS/JS Web App
│   ├── login.html              # ← OPEN THIS to start
│   ├── register.html
│   ├── pages/
│   │   ├── dashboard.html      # Main dashboard with charts & cards
│   │   ├── transactions.html   # Full transaction table + CRUD
│   │   ├── income.html         # Income management
│   │   ├── expenses.html       # Expense tracking
│   │   ├── budgets.html        # Budget management + warnings
│   │   ├── savings.html        # Savings goals + progress
│   │   ├── analytics.html      # Chart.js analytics dashboard
│   │   ├── reports.html        # Financial reports
│   │   └── settings.html       # App settings + demo reset
│   ├── css/
│   │   ├── style.css           # Global styles (dark navy theme)
│   │   ├── dashboard.css       # Dashboard-specific styles
│   │   └── responsive.css      # Mobile responsiveness
│   └── js/
│       ├── api.js              # API layer + offline demo data
│       ├── auth.js             # Auth & session management
│       ├── dashboard.js        # Dashboard logic & Chart.js
│       ├── transactions.js     # Transaction CRUD
│       ├── income.js           # Income page logic
│       ├── expenses.js         # Expense page logic
│       ├── budgets.js          # Budget management
│       ├── savings.js          # Savings goals
│       ├── analytics.js        # Analytics charts
│       └── reports.js          # Report generation
│
├── backend/                    # C++ OOP Backend
│   ├── include/
│   │   ├── crow.h              # Crow HTTP framework (single header)
│   │   ├── models/             # OOP Model Headers
│   │   │   ├── Transaction.h   # Abstract base class
│   │   │   ├── Income.h        # Derived class
│   │   │   ├── Expense.h       # Derived class
│   │   │   ├── User.h
│   │   │   ├── Budget.h
│   │   │   └── SavingsGoal.h
│   │   ├── services/           # Manager/Service Headers
│   │   │   ├── AuthenticationManager.h
│   │   │   ├── FinanceManager.h
│   │   │   └── ReportManager.h
│   │   ├── database/
│   │   │   └── DatabaseManager.h  # In-memory data store
│   │   └── routes/             # REST API Route Headers
│   │       ├── AuthRoutes.h
│   │       ├── TransactionRoutes.h
│   │       ├── BudgetRoutes.h
│   │       └── SavingsRoutes.h
│   └── src/                    # C++ Implementation Files
│       ├── main.cpp
│       ├── models/             # Transaction, Income, Expense, etc.
│       ├── services/           # AuthenticationManager, FinanceManager, etc.
│       ├── database/           # DatabaseManager (in-memory + file I/O)
│       └── routes/             # REST API route handlers
│
├── OPEN_DEMO.bat               # ← Double-click to open app instantly
├── RUN_FINTRACK.bat            # Starts C++ server + opens browser
├── README.md
└── .gitignore
```

---

## 🎨 Features

- **Dashboard** — balance cards, income vs expense chart, budget progress, recent transactions
- **Transactions** — add, edit, delete, search, filter by type/category/date
- **Income** — manage income sources (Salary, Freelance, Business, etc.)
- **Expenses** — track expenses with category breakdown chart
- **Budgets** — monthly budgets with ⚠️ warning at 80% and 🔴 exceeded alerts
- **Savings Goals** — goal tracking with progress bars and add-money feature
- **Analytics** — 5 Chart.js charts (line, bar, doughnut, radar)
- **Reports** — monthly/quarterly/yearly summaries with print support
- **Settings** — reset demo data, export ledger as JSON
- **Auth** — login, register, logout with password hashing in C++ backend

---

## 🔑 Class Hierarchy (OOP Architecture)

```
Transaction  (Abstract Base Class)
│
├── Income   (Concrete — override display(), calculateImpact())
│
└── Expense  (Concrete — override display(), calculateImpact())

User              — registered user with hashed password
Budget            — monthly category budget
SavingsGoal       — savings target with progress

DatabaseManager   — local JSON/file-backed CRUD store (MongoDB integration planned)
AuthenticationManager — login, register, JWT-like token
FinanceManager    — business logic, dashboard data, analytics
ReportManager     — financial report generation
```

---

## 🌐 REST API Endpoints (C++ Backend)

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/api/auth/login` | Login & get token |
| POST | `/api/auth/register` | Create account |
| GET | `/api/dashboard` | Dashboard summary |
| GET/POST | `/api/transactions` | List / add transaction |
| PUT/DELETE | `/api/transactions/:id` | Edit / delete |
| GET/POST | `/api/budgets` | Budgets CRUD |
| GET/POST | `/api/savings-goals` | Savings goals CRUD |
| GET | `/api/analytics` | Analytics data |
| GET | `/api/reports` | Financial reports |

---

## ⚙️ Running the C++ Backend (Optional)

The app works without the backend. To run the full C++ server:

1. Compile with MinGW-w64 (g++ C++17):
   ```bash
   g++ -std=c++17 -O2 -Ibackend/include backend/src/**/*.cpp backend/src/main.cpp -o backend/fintrack_server.exe -lws2_32
   ```
2. Run:
   ```bash
   backend\fintrack_server.exe 8080
   ```
3. Open: `http://localhost:8080/login.html`

---

*Developed for OOP Subject — University Mini Project*
