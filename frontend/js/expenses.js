/**
 * FINTRACK - Expenses Management Controller
 */

let expensesChart = null;
let expenseList = [];
let editingExpenseId = null;

document.addEventListener('DOMContentLoaded', () => {
  FinTrackAuth.checkAuth(true);
  loadExpenseData();
  setupExpenseModal();
});

async function loadExpenseData() {
  try {
    const res = await FinTrackAPI.get('/api/expenses');
    if (!res) return;

    expenseList = res.expenseList || [];

    // Stats Cards
    const cur = '₹';
    document.getElementById('totalExpensesDisplay').textContent = FinTrackAPI.formatCurrency(res.totalExpenses, cur);
    document.getElementById('thisMonthExpensesDisplay').textContent = FinTrackAPI.formatCurrency(res.thisMonth, cur);
    document.getElementById('largestExpenseDisplay').textContent = FinTrackAPI.formatCurrency(res.largestExpense, cur);
    document.getElementById('avgDailyExpensesDisplay').textContent = FinTrackAPI.formatCurrency(res.averageDaily, cur);

    renderCategoryChart(res.categoryBreakdown || []);
    renderExpensesTable(expenseList);
  } catch (err) {
    console.error('Failed to load expenses:', err);
    FinTrackAPI.showToast('Unable to fetch expenses data.', 'error');
  }
}

function renderCategoryChart(breakdown) {
  const ctx = document.getElementById('expenseCategoryChart');
  if (!ctx) return;

  if (expensesChart) {
    expensesChart.destroy();
  }

  const labels = breakdown.map(x => x.category);
  const data = breakdown.map(x => x.amount);

  expensesChart = new Chart(ctx, {
    type: 'doughnut',
    data: {
      labels: labels.length ? labels : ['Food', 'Shopping', 'Transport'],
      datasets: [{
        data: data.length ? data : [4200, 5500, 2100],
        backgroundColor: [
          '#6366f1', '#ec4899', '#f59e0b', '#10b981', '#38bdf8', '#8b5cf6', '#ef4444', '#14b8a6', '#64748b'
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
          position: 'right',
          labels: { color: '#cbd5e1', font: { family: 'Plus Jakarta Sans', size: 12 } }
        },
        tooltip: {
          callbacks: {
            label: function(context) {
              return ` ${context.label}: ₹${Number(context.raw).toLocaleString('en-IN')}`;
            }
          }
        }
      },
      cutout: '70%'
    }
  });
}

function renderExpensesTable(list) {
  const tbody = document.getElementById('expensesTableBody');
  if (!tbody) return;

  if (!list.length) {
    tbody.innerHTML = `<tr><td colspan="6" class="text-muted" style="text-align: center; padding: 32px;">No expense records found.</td></tr>`;
    return;
  }

  tbody.innerHTML = list.map(item => {
    return `
      <tr>
        <td><span class="mono text-secondary">${FinTrackAPI.formatDate(item.date)}</span></td>
        <td>
          <div style="font-weight: 700; color: #ffffff;">${escapeHtml(item.description || item.category)}</div>
          <div style="font-size: 0.75rem; color: var(--text-muted);">${item.merchant ? 'Merchant: ' + escapeHtml(item.merchant) : (item.paymentMethod || 'UPI')}</div>
        </td>
        <td><span style="font-weight: 600;">${item.category}</span></td>
        <td><span class="badge badge-expense">${item.paymentMethod || 'UPI'}</span></td>
        <td><span class="text-expense mono" style="font-weight: 800; font-size: 1.05rem;">-₹${Number(item.amount).toLocaleString('en-IN')}</span></td>
        <td style="text-align: right;">
          <div style="display: inline-flex; gap: 8px;">
            <button class="btn btn-secondary btn-sm edit-expense-btn" data-id="${item.id}">Edit</button>
            <button class="btn btn-danger btn-sm delete-expense-btn" data-id="${item.id}">Delete</button>
          </div>
        </td>
      </tr>
    `;
  }).join('');

  document.querySelectorAll('.edit-expense-btn').forEach(btn => {
    btn.addEventListener('click', () => openEditExpenseModal(btn.dataset.id));
  });

  document.querySelectorAll('.delete-expense-btn').forEach(btn => {
    btn.addEventListener('click', () => deleteExpense(btn.dataset.id));
  });
}

function setupExpenseModal() {
  const modal = document.getElementById('expenseModal');
  const openBtn = document.getElementById('openAddExpenseBtn');
  const closeBtn = document.getElementById('closeExpenseModal');
  const cancelBtn = document.getElementById('cancelExpenseBtn');
  const form = document.getElementById('expenseForm');

  if (openBtn && modal) {
    openBtn.addEventListener('click', () => {
      editingExpenseId = null;
      document.getElementById('expenseModalTitle').textContent = 'Add New Expense';
      form.reset();
      document.getElementById('expenseDate').value = new Date().toISOString().split('T')[0];
      modal.classList.add('active');
    });
  }

  const closeModal = () => {
    if (modal) modal.classList.remove('active');
    editingExpenseId = null;
  };

  if (closeBtn) closeBtn.addEventListener('click', closeModal);
  if (cancelBtn) cancelBtn.addEventListener('click', closeModal);

  if (form) {
    form.addEventListener('submit', async (e) => {
      e.preventDefault();
      const amount = parseFloat(document.getElementById('expenseAmount').value);
      const category = document.getElementById('expenseCategory').value;
      const description = document.getElementById('expenseDescription').value;
      const date = document.getElementById('expenseDate').value;
      const paymentMethod = document.getElementById('expensePaymentMethod').value;
      const merchant = document.getElementById('expenseMerchant').value;

      if (!amount || amount <= 0) {
        FinTrackAPI.showToast('Expense amount must be greater than zero.', 'error');
        return;
      }

      const payload = {
        amount,
        category,
        description,
        date,
        paymentMethod,
        merchant
      };

      try {
        if (editingExpenseId) {
          await FinTrackAPI.put(`/api/expenses/${editingExpenseId}`, payload);
          FinTrackAPI.showToast('Expense updated successfully!', 'success');
        } else {
          await FinTrackAPI.post('/api/expenses', payload);
          FinTrackAPI.showToast('Expense recorded successfully!', 'success');
        }
        closeModal();
        loadExpenseData();
      } catch (err) {
        FinTrackAPI.showToast(err.message || 'Unable to save expense.', 'error');
      }
    });
  }
}

function openEditExpenseModal(id) {
  const item = expenseList.find(x => x.id === id);
  if (!item) return;

  editingExpenseId = id;
  const modal = document.getElementById('expenseModal');
  document.getElementById('expenseModalTitle').textContent = 'Edit Expense Record';

  document.getElementById('expenseAmount').value = item.amount;
  document.getElementById('expenseCategory').value = item.category;
  document.getElementById('expenseDescription').value = item.description || '';
  document.getElementById('expenseDate').value = item.date;
  document.getElementById('expensePaymentMethod').value = item.paymentMethod || 'UPI';
  document.getElementById('expenseMerchant').value = item.merchant || '';

  modal.classList.add('active');
}

async function deleteExpense(id) {
  if (!confirm('Are you sure you want to delete this expense record?')) return;

  try {
    await FinTrackAPI.delete(`/api/expenses/${id}`);
    FinTrackAPI.showToast('Expense deleted successfully.', 'success');
    loadExpenseData();
  } catch (err) {
    FinTrackAPI.showToast(err.message || 'Unable to delete expense.', 'error');
  }
}

function escapeHtml(str) {
  if (!str) return '';
  return str.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

