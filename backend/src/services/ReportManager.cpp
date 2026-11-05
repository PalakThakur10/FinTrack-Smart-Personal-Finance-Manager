#include "../../include/services/ReportManager.h"
#include <algorithm>
#include <cmath>
#include <map>

ReportManager::ReportManager(std::shared_ptr<DatabaseManager> dbManager,
                             std::shared_ptr<FinanceManager> finManager)
    : db(dbManager), finance(finManager) {}

crow::json::wvalue ReportManager::generateReport(const std::string& userId,
                                                const std::string& period,
                                                const std::string& year,
                                                const std::string& monthOrQuarter) const {
    auto txs = db->getTransactionsByUserId(userId);
    auto user = db->findUserById(userId);

    double totalIncome = 0.0;
    double totalExpenses = 0.0;
    double largestExpenseAmount = 0.0;
    std::string largestExpenseDesc = "None";
    std::string largestExpenseCat = "None";

    std::map<std::string, double> catSpending;
    std::map<std::string, double> incomeSources;

    // Filter transactions by period
    std::vector<std::shared_ptr<Transaction>> periodTxs;
    for (const auto& tx : txs) {
        std::string date = tx->getDate(); // YYYY-MM-DD
        if (date.length() >= 4 && !year.empty()) {
            if (date.substr(0, 4) != year) continue;
        }

        if (period == "monthly" && date.length() >= 7 && !monthOrQuarter.empty()) {
            std::string mStr = date.substr(5, 2);
            std::string targetM = monthOrQuarter;
            if (targetM.length() == 1) targetM = "0" + targetM;
            if (mStr != targetM) continue;
        }

        periodTxs.push_back(tx);

        double impact = tx->calculateImpact();
        if (impact > 0.0) {
            totalIncome += impact;
            incomeSources[tx->getCategory()] += impact;
        } else {
            double expenseAmount = std::abs(impact);
            totalExpenses += expenseAmount;
            catSpending[tx->getCategory()] += expenseAmount;

            if (expenseAmount > largestExpenseAmount) {
                largestExpenseAmount = expenseAmount;
                largestExpenseDesc = tx->getDescription();
                largestExpenseCat = tx->getCategory();
            }
        }
    }

    double netSavings = totalIncome - totalExpenses;
    double savingsRate = (totalIncome > 0.0) ? (netSavings / totalIncome) * 100.0 : 0.0;
    if (savingsRate < 0.0) savingsRate = 0.0;

    // Top spending category
    std::string topCat = "None";
    double topCatAmount = 0.0;
    for (const auto& pair : catSpending) {
        if (pair.second > topCatAmount) {
            topCatAmount = pair.second;
            topCat = pair.first;
        }
    }

    crow::json::wvalue res;
    res["status"] = "success";
    res["report"]["period"] = period;
    res["report"]["year"] = year;
    res["report"]["monthOrQuarter"] = monthOrQuarter;
    res["report"]["generatedFor"] = user ? user->getName() : "Valued Member";
    res["report"]["currency"] = user ? user->getCurrency() : "INR";

    res["report"]["summary"]["totalIncome"] = totalIncome;
    res["report"]["summary"]["totalExpenses"] = totalExpenses;
    res["report"]["summary"]["netSavings"] = netSavings;
    res["report"]["summary"]["savingsRate"] = savingsRate;
    res["report"]["summary"]["largestExpenseAmount"] = largestExpenseAmount;
    res["report"]["summary"]["largestExpenseDescription"] = largestExpenseDesc;
    res["report"]["summary"]["largestExpenseCategory"] = largestExpenseCat;
    res["report"]["summary"]["topSpendingCategory"] = topCat;
    res["report"]["summary"]["topSpendingCategoryAmount"] = topCatAmount;

    // Health Assessment
    std::string healthScore = "Good";
    std::string recommendation;
    if (savingsRate >= 40.0) {
        healthScore = "Excellent (Grade A+)";
        recommendation = "Outstanding financial discipline! Over 40% of your earnings are retained. Consider allocating extra surplus into long-term index investments or emergency buffers.";
    } else if (savingsRate >= 20.0) {
        healthScore = "Healthy (Grade A)";
        recommendation = "You maintain a solid savings rate that adheres well to standard 50/30/20 budget guidelines. Keep your discretionary spending in check to boost savings further.";
    } else if (savingsRate >= 10.0) {
        healthScore = "Moderate (Grade B)";
        recommendation = "Your income exceeds expenses, but margins are tight. Review top spending categories (such as " + topCat + ") to identify optimization opportunities.";
    } else {
        healthScore = "Needs Attention (Grade C)";
        recommendation = "Your expenses are closely tracking or exceeding income. Consider setting strict monthly budget caps and pausing non-essential discretionary purchases.";
    }

    res["report"]["healthScore"] = healthScore;
    res["report"]["recommendation"] = recommendation;

    // Category Breakdowns
    crow::json::wvalue catArr;
    size_t cIdx = 0;
    for (const auto& pair : catSpending) {
        crow::json::wvalue item;
        item["category"] = pair.first;
        item["amount"] = pair.second;
        item["percentage"] = (totalExpenses > 0.0) ? (pair.second / totalExpenses) * 100.0 : 0.0;
        catArr[cIdx++] = item;
    }
    res["report"]["categoryBreakdown"] = catArr;

    // Transaction list included in report
    crow::json::wvalue txArr;
    for (size_t i = 0; i < periodTxs.size(); ++i) {
        txArr[i] = periodTxs[i]->toJson();
    }
    res["report"]["transactions"] = txArr;

    return res;
}

