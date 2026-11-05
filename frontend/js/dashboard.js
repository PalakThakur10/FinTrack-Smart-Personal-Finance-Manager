/**
 * FINTRACK - Dashboard Controller
 * Loads real-time financial stats, Chart.js graphs, budgets, and recent transactions.
 */

let incomeExpenseChart = null;
let expenseBreakdownChart = null;

document.addEventListener('DOMContentLoaded', () => {
  FinTrackAuth.checkAuth(true);
  loadDashboardData();
  setupAddTransactionModal();
});

async function loadDashboardData() {
  try {
    const data = await FinTrackAPI.get('/api/dashboard');
    if (!data || !data.summary) return;

    // 1. Update 4 Metric Cards
    const cur = '₹';
    document.getElementById('totalBalance').textContent = FinTrackAPI.formatCurrency(data.summary.totalBalance, cur);
    document.getElementById('totalIncome').textContent = FinTrackAPI.formatCurrency(data.summary.totalIncome, cur);
    document.getElementById('totalExpenses').textContent = FinTrackAPI.formatCurrency(data.summary.totalExpenses, cur);
    document.getElementById('totalSavings').textContent = FinTrackAPI.formatCurrency(data.summary.totalSavings, cur);

    // Savings rate trend
    const rateEl = document.getElementById('savingsRateBadge');
    if (rateEl) {
      rateEl.textContent = `${Number(data.summary.savingsRate).toFixed(1)}% savings rate`;
    }

    // 2. Render Charts
    renderIncomeExpenseChart(data.monthlyTrend);
    renderExpenseBreakdownChart(data.expenseBreakdown);

    // 3. Render Monthly Budgets
    renderMonthlyBudgets(data.budgets || []);

    // 4. Render Recent Transactions
    renderRecentTransactions(data.recentTransactions || []);

  } catch (err) {
    console.error('Failed to load dashboard:', err);
    FinTrackAPI.showToast('Unable to fetch live dashboard metrics.', 'error');
  }
}

// Chart 1: Income vs Expenses
function renderIncomeExpenseChart(trend) {
  const ctx = document.getElementById('incomeExpenseChart');
  if (!ctx) return;

  if (incomeExpenseChart) {
    incomeExpenseChart.destroy();
  }

  const labels = (trend && trend.labels && trend.labels.length) ? trend.labels : ['Nov 2026'];
  const incomeData = (trend && trend.income && trend.income.length) ? trend.income : [75000];
  const expenseData = (trend && trend.expenses && trend.expenses.length) ? trend.expenses : [29320];

  incomeExpenseChart = new Chart(ctx, {
    type: 'bar',
    data: {
      labels: labels,
      datasets: [
        {
          label: 'Income',
          data: incomeData,
          backgroundColor: 'rgba(16, 185, 129, 0.75)',
          borderColor: '#10b981',
          borderWidth: 1.5,
          borderRadius: 6,
          barPercentage: 0.6
        },
        {
          label: 'Expenses',
          data: expenseData,
          backgroundColor: 'rgba(239, 68, 68, 0.75)',
          borderColor: '#ef4444',
          borderWidth: 1.5,
          borderRadius: 6,
          barPercentage: 0.6
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          position: 'top',
          labels: { color: '#cbd5e1', font: { family: 'Plus Jakarta Sans', weight: '600' } }
        },
        tooltip: {
          callbacks: {
            label: function(context) {
              return `${context.dataset.label}: ₹${Number(context.raw).toLocaleString('en-IN')}`;
            }
          }
        }
      },
      scales: {
        x: {
          grid: { color: 'rgba(255, 255, 255, 0.05)' },
          ticks: { color: '#94a3b8' }
        },
        y: {
          grid: { color: 'rgba(255, 255, 255, 0.05)' },
          ticks: {
            color: '#94a3b8',
            callback: value => '₹' + (value >= 1000 ? (value / 1000) + 'k' : value)
          }
        }
      }
    }
  });
}

// Chart 2: Expense Breakdown (Doughnut)
function renderExpenseBreakdownChart(breakdown) {
  const ctx = document.getElementById('expenseBreakdownChart');
  if (!ctx) return;

  if (expenseBreakdownChart) {
    expenseBreakdownChart.destroy();
  }

  const defaultLabels = ['Food', 'Shopping', 'Bills', 'Education', 'Transport', 'Entertainment', 'Other'];
  const defaultData = [4200, 5500, 4020, 3000, 2100, 2500, 8000];

  const labels = (breakdown && breakdown.labels && breakdown.labels.length) ? breakdown.labels : defaultLabels;
  const data = (breakdown && breakdown.data && breakdown.data.length) ? breakdown.data : defaultData;

  expenseBreakdownChart = new Chart(ctx, {
    type: 'doughnut',
    data: {
      labels: labels,
      datasets: [{
        data: data,
        backgroundColor: [
          '#6366f1', '#ec4899', '#f59e0b', '#10b981', '#38bdf8', '#8b5cf6', '#64748b'
        ],
        borderWidth: 2,
        borderColor: '#131c2e'
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          position: 'bottom',
          labels: { color: '#cbd5e1', boxWidth: 12, padding: 12, font: { size: 11, family: 'Plus Jakarta Sans' } }
        },
        tooltip: {
          callbacks: {
            label: function(context) {
              return ` ${context.label}: ₹${Number(context.raw).toLocaleString('en-IN')}`;
            }
          }
        }
      },
      cutout: '72%'
    }
  });
}

// Monthly Budget Progress Bars
function renderMonthlyBudgets(budgets) {
  const container = document.getElementById('dashboardBudgetList');
  if (!container) return;

  if (!budgets.length) {
    container.innerHTML = `<div class="text-muted" style="text-align: center; padding: 20px;">No active budgets found.</div>`;
    return;
  }

  container.innerHTML = budgets.map(b => {
    const limit = Number(b.limit || b.limitAmount || 1000);
    const spent = Number(b.spent || b.spentAmount || 0);
    const pct = Math.min(Math.round((spent / limit) * 100), 100);

    let progressClass = 'progress-normal';
    let statusBadge = '';

    if (spent > limit) {
      progressClass = 'progress-exceeded';
      statusBadge = `<span class="badge badge-expense" style="font-size: 0.65rem;">Exceeded</span>`;
    } else if (pct >= 80) {
      progressClass = 'progress-warning';
      statusBadge = `<span class="badge badge-warning" style="font-size: 0.65rem;">Approaching Limit</span>`;
    }

    return `
      <div class="budget-item">
        <div class="budget-info">
          <div class="budget-cat-name">
            <span>${b.category}</span>
            ${statusBadge}
          </div>
          <div class="budget-values">
            <span class="spent">₹${spent.toLocaleString('en-IN')}</span> / ₹${limit.toLocaleString('en-IN')}
            <span class="text-muted" style="margin-left: 8px;">(${pct}%)</span>
          </div>
        </div>
        <div class="progress-container">
          <div class="progress-bar ${progressClass}" style="width: ${pct}%;"></div>
        </div>
      </div>
    `;
  }).join('');
}

// Recent Transactions List
function renderRecentTransactions(txs) {
  const tbody = document.getElementById('recentTransactionsBody');
  if (!tbody) return;

  if (!txs.length) {
    tbody.innerHTML = `<tr><td colspan="5" class="text-muted" style="text-align: center; padding: 24px;">No recent transactions available.</td></tr>`;
    return;
  }

  tbody.innerHTML = txs.map(tx => {
    const isIncome = tx.type === 'income';
    const amountStr = isIncome
      ? `+₹${Number(tx.amount).toLocaleString('en-IN')}`
      : `-₹${Number(tx.amount).toLocaleString('en-IN')}`;
    const amountClass = isIncome ? 'text-income' : 'text-expense';
    const badgeClass = isIncome ? 'badge-income' : 'badge-expense';
    const icon = getCategoryIcon(tx.category);

    return `
      <tr>
        <td>
          <div class="table-tx-title">
            <div class="tx-cat-icon" style="background: ${isIncome ? 'rgba(16, 185, 129, 0.12)' : 'rgba(239, 68, 68, 0.12)'}; color: ${isIncome ? '#10b981' : '#ef4444'};">
              ${icon}
            </div>
            <div>
              <div style="font-weight: 700; color: #ffffff;">${tx.description || tx.category}</div>
              <div style="font-size: 0.78rem; color: var(--text-muted);">${tx.paymentMethod || 'UPI'}</div>
            </div>
          </div>
        </td>
        <td><span style="font-weight: 600;">${tx.category}</span></td>
        <td><span class="badge ${badgeClass}">${tx.type}</span></td>
        <td><span class="text-secondary">${FinTrackAPI.formatDate(tx.date)}</span></td>
        <td style="text-align: right;"><span class="${amountClass} mono" style="font-weight: 800; font-size: 1rem;">${amountStr}</span></td>
      </tr>
    `;
  }).join('');
}

function getCategoryIcon(cat) {
  const lower = (cat || '').toLowerCase();
  if (lower.includes('salary')) return '💼';
  if (lower.includes('freelance')) return '💻';
  if (lower.includes('food') || lower.includes('grocery')) return '🍔';
  if (lower.includes('transport') || lower.includes('metro')) return '🚇';
  if (lower.includes('shopping') || lower.includes('amazon')) return '🛍️';
  if (lower.includes('entertainment') || lower.includes('movie')) return '🎬';
  if (lower.includes('education') || lower.includes('course')) return '📚';
  if (lower.includes('bill') || lower.includes('electricity')) return '⚡';
  if (lower.includes('health') || lower.includes('insurance')) return '🏥';
  return '💳';
}

// Global Quick "+ Add Transaction" Modal
function setupAddTransactionModal() {
  const modal = document.getElementById('transactionModal');
  const openBtns = document.querySelectorAll('.open-add-tx-modal');
  const closeBtn = document.getElementById('closeTxModal');
  const cancelBtn = document.getElementById('cancelTxBtn');
  const form = document.getElementById('transactionForm');

  if (!modal || !form) return;

  const openModal = () => {
    form.reset();
    document.getElementById('txDate').value = new Date().toISOString().split('T')[0];
    modal.classList.add('active');
  };

  const closeModal = () => modal.classList.remove('active');

  openBtns.forEach(btn => btn.addEventListener('click', openModal));
  if (closeBtn) closeBtn.addEventListener('click', closeModal);
  if (cancelBtn) cancelBtn.addEventListener('click', closeModal);

  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    const type = document.getElementById('txType').value;
    const amount = parseFloat(document.getElementById('txAmount').value);
    const category = document.getElementById('txCategory').value;
    const description = document.getElementById('txDescription').value;
    const date = document.getElementById('txDate').value;
    const paymentMethod = document.getElementById('txPaymentMethod').value;

    if (!amount || amount <= 0) {
      FinTrackAPI.showToast('Transaction amount must be greater than zero.', 'error');
      return;
    }

    try {
      await FinTrackAPI.post('/api/transactions', {
        type,
        amount,
        category,
        description,
        date,
        paymentMethod
      });

      FinTrackAPI.showToast('Transaction saved successfully!', 'success');
      closeModal();
      loadDashboardData();
    } catch (err) {
      FinTrackAPI.showToast(err.message || 'Unable to save transaction.', 'error');
    }
  });
}

