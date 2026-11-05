/**
 * FINTRACK - Budget Management Controller
 */

let budgetsList = [];
let editingBudgetId = null;

document.addEventListener('DOMContentLoaded', () => {
  FinTrackAuth.checkAuth(true);
  loadBudgets();
  setupBudgetModal();
});

async function loadBudgets() {
  try {
    const res = await FinTrackAPI.get('/api/budgets');
    budgetsList = res.budgets || [];
    renderBudgets(budgetsList);
  } catch (err) {
    console.error('Failed to load budgets:', err);
    FinTrackAPI.showToast('Unable to fetch budget goals.', 'error');
  }
}

function renderBudgets(budgets) {
  const container = document.getElementById('budgetsContainer');
  const totalBudgetedEl = document.getElementById('totalBudgetedAmount');
  const totalSpentEl = document.getElementById('totalBudgetSpentAmount');

  let totalLimit = 0;
  let totalSpent = 0;

  budgets.forEach(b => {
    totalLimit += Number(b.limit || b.limitAmount || 0);
    totalSpent += Number(b.spent || b.spentAmount || 0);
  });

  if (totalBudgetedEl) totalBudgetedEl.textContent = FinTrackAPI.formatCurrency(totalLimit, '₹');
  if (totalSpentEl) totalSpentEl.textContent = FinTrackAPI.formatCurrency(totalSpent, '₹');

  if (!container) return;

  if (!budgets.length) {
    container.innerHTML = `
      <div style="grid-column: 1 / -1; text-align: center; padding: 48px; background: var(--bg-card); border-radius: var(--radius-lg); border: 1px solid var(--border-color);">
        <div style="font-size: 2.5rem; margin-bottom: 12px;">📊</div>
        <div style="font-weight: 700; font-size: 1.15rem; color: #ffffff;">No Budgets Defined</div>
        <div class="text-muted" style="margin-top: 4px;">Click "+ Create Budget" to set monthly spending caps for Food, Shopping, etc.</div>
      </div>
    `;
    return;
  }

  container.innerHTML = budgets.map(b => {
    const limit = Number(b.limit || b.limitAmount || 1000);
    const spent = Number(b.spent || b.spentAmount || 0);
    const remaining = Math.max(0, limit - spent);
    const rawPct = Math.round((spent / limit) * 100);
    const barPct = Math.min(rawPct, 100);

    let progressClass = 'progress-normal';
    let statusBadge = '<span class="badge badge-safe">Healthy</span>';
    let alertBanner = '';

    if (spent > limit) {
      progressClass = 'progress-exceeded';
      statusBadge = '<span class="badge badge-expense">Exceeded</span>';
      alertBanner = `
        <div style="margin-top: 12px; padding: 8px 12px; background: rgba(239, 68, 68, 0.12); border: 1px solid rgba(239, 68, 68, 0.3); border-radius: 8px; font-size: 0.8rem; color: #f87171; display: flex; align-items: center; gap: 6px;">
          <span>⚠️</span> <strong>Exceeded by ₹${(spent - limit).toLocaleString('en-IN')}!</strong> Limit breached.
        </div>
      `;
    } else if (rawPct >= 80) {
      progressClass = 'progress-warning';
      statusBadge = '<span class="badge badge-warning">Approaching Limit</span>';
      alertBanner = `
        <div style="margin-top: 12px; padding: 8px 12px; background: rgba(245, 158, 11, 0.12); border: 1px solid rgba(245, 158, 11, 0.3); border-radius: 8px; font-size: 0.8rem; color: #fbbf24; display: flex; align-items: center; gap: 6px;">
          <span>⚡</span> <strong>Warning:</strong> You have consumed ${rawPct}% of this budget.
        </div>
      `;
    }

    return `
      <div class="card budget-card">
        <div class="card-header" style="margin-bottom: 12px;">
          <div class="card-title" style="font-size: 1.1rem;">
            <span>${b.category}</span>
          </div>
          <div>${statusBadge}</div>
        </div>

        <div style="margin-bottom: 14px;">
          <div style="display: flex; justify-content: space-between; font-size: 0.85rem; margin-bottom: 6px;">
            <span class="text-muted">Spent so far:</span>
            <span style="font-weight: 700; color: #ffffff;">₹${spent.toLocaleString('en-IN')}</span>
          </div>
          <div style="display: flex; justify-content: space-between; font-size: 0.85rem; margin-bottom: 6px;">
            <span class="text-muted">Allocated Cap:</span>
            <span style="font-weight: 600; color: var(--text-light);">₹${limit.toLocaleString('en-IN')}</span>
          </div>
          <div style="display: flex; justify-content: space-between; font-size: 0.85rem;">
            <span class="text-muted">Remaining Balance:</span>
            <span style="font-weight: 700; color: ${spent > limit ? '#ef4444' : '#10b981'};">₹${remaining.toLocaleString('en-IN')}</span>
          </div>
        </div>

        <div class="progress-container" style="height: 10px; margin-bottom: 8px;">
          <div class="progress-bar ${progressClass}" style="width: ${barPct}%;"></div>
        </div>

        <div style="display: flex; justify-content: space-between; font-size: 0.75rem; color: var(--text-muted);">
          <span>0%</span>
          <span>${rawPct}% Consumed</span>
          <span>100%</span>
        </div>

        ${alertBanner}

        <div style="margin-top: 18px; padding-top: 14px; border-top: 1px solid var(--border-color); display: flex; justify-content: flex-end; gap: 8px;">
          <button class="btn btn-secondary btn-sm edit-budget-btn" data-id="${b.id}">Edit</button>
          <button class="btn btn-danger btn-sm delete-budget-btn" data-id="${b.id}">Delete</button>
        </div>
      </div>
    `;
  }).join('');

  document.querySelectorAll('.edit-budget-btn').forEach(btn => {
    btn.addEventListener('click', () => openEditBudgetModal(btn.dataset.id));
  });

  document.querySelectorAll('.delete-budget-btn').forEach(btn => {
    btn.addEventListener('click', () => deleteBudget(btn.dataset.id));
  });
}

function setupBudgetModal() {
  const modal = document.getElementById('budgetModal');
  const openBtn = document.getElementById('openAddBudgetBtn');
  const closeBtn = document.getElementById('closeBudgetModal');
  const cancelBtn = document.getElementById('cancelBudgetBtn');
  const form = document.getElementById('budgetForm');

  if (openBtn && modal) {
    openBtn.addEventListener('click', () => {
      editingBudgetId = null;
      document.getElementById('budgetModalTitle').textContent = 'Create Monthly Budget';
      form.reset();
      modal.classList.add('active');
    });
  }

  const closeModal = () => {
    if (modal) modal.classList.remove('active');
    editingBudgetId = null;
  };

  if (closeBtn) closeBtn.addEventListener('click', closeModal);
  if (cancelBtn) cancelBtn.addEventListener('click', closeModal);

  if (form) {
    form.addEventListener('submit', async (e) => {
      e.preventDefault();
      const category = document.getElementById('budgetCategory').value;
      const limit = parseFloat(document.getElementById('budgetLimit').value);

      if (!limit || limit <= 0) {
        FinTrackAPI.showToast('Budget limit must be greater than zero.', 'error');
        return;
      }

      const payload = { category, limit, limitAmount: limit };

      try {
        if (editingBudgetId) {
          await FinTrackAPI.put(`/api/budgets/${editingBudgetId}`, payload);
          FinTrackAPI.showToast('Budget limit updated successfully!', 'success');
        } else {
          await FinTrackAPI.post('/api/budgets', payload);
          FinTrackAPI.showToast('New budget allocated successfully!', 'success');
        }
        closeModal();
        loadBudgets();
      } catch (err) {
        FinTrackAPI.showToast(err.message || 'Unable to save budget.', 'error');
      }
    });
  }
}

function openEditBudgetModal(id) {
  const item = budgetsList.find(b => b.id === id);
  if (!item) return;

  editingBudgetId = id;
  const modal = document.getElementById('budgetModal');
  document.getElementById('budgetModalTitle').textContent = 'Edit Budget Limit';

  document.getElementById('budgetCategory').value = item.category;
  document.getElementById('budgetLimit').value = item.limit || item.limitAmount;

  modal.classList.add('active');
}

async function deleteBudget(id) {
  if (!confirm('Are you sure you want to remove this category budget?')) return;

  try {
    await FinTrackAPI.delete(`/api/budgets/${id}`);
    FinTrackAPI.showToast('Budget removed successfully.', 'success');
    loadBudgets();
  } catch (err) {
    FinTrackAPI.showToast(err.message || 'Unable to delete budget.', 'error');
  }
}

