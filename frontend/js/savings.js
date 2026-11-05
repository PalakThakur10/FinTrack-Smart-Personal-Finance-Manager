/**
 * FINTRACK - Savings Goals Controller
 */

let savingsList = [];
let editingGoalId = null;
let depositGoalId = null;

document.addEventListener('DOMContentLoaded', () => {
  FinTrackAuth.checkAuth(true);
  loadSavingsGoals();
  setupSavingsModals();
});

async function loadSavingsGoals() {
  try {
    const res = await FinTrackAPI.get('/api/savings-goals');
    savingsList = res.savingsGoals || [];
    renderSavingsGoals(savingsList);
  } catch (err) {
    console.error('Failed to load savings goals:', err);
    FinTrackAPI.showToast('Unable to fetch savings goals.', 'error');
  }
}

function renderSavingsGoals(goals) {
  const container = document.getElementById('savingsContainer');
  const totalSavedEl = document.getElementById('totalSavingsAccumulated');
  const totalTargetEl = document.getElementById('totalSavingsTarget');

  let totalSaved = 0;
  let totalTarget = 0;

  goals.forEach(g => {
    totalSaved += Number(g.currentAmount || 0);
    totalTarget += Number(g.targetAmount || 0);
  });

  if (totalSavedEl) totalSavedEl.textContent = FinTrackAPI.formatCurrency(totalSaved, '₹');
  if (totalTargetEl) totalTargetEl.textContent = FinTrackAPI.formatCurrency(totalTarget, '₹');

  if (!container) return;

  if (!goals.length) {
    container.innerHTML = `
      <div style="grid-column: 1 / -1; text-align: center; padding: 48px; background: var(--bg-card); border-radius: var(--radius-lg); border: 1px solid var(--border-color);">
        <div style="font-size: 2.5rem; margin-bottom: 12px;">🎯</div>
        <div style="font-weight: 700; font-size: 1.15rem; color: #ffffff;">No Savings Goals Yet</div>
        <div class="text-muted" style="margin-top: 4px;">Start by setting your first savings target (e.g. Emergency Fund or New Laptop).</div>
      </div>
    `;
    return;
  }

  container.innerHTML = goals.map(g => {
    const target = Number(g.targetAmount || 1000);
    const saved = Number(g.currentAmount || 0);
    const remaining = Math.max(0, target - saved);
    const rawPct = Math.round((saved / target) * 100);
    const barPct = Math.min(rawPct, 100);
    const isDone = saved >= target;

    return `
      <div class="card savings-card">
        <div class="savings-card-top">
          <div class="savings-title">${escapeHtml(g.name)}</div>
          <span class="badge ${isDone ? 'badge-income' : 'badge-safe'}">
            ${isDone ? 'Goal Reached! 🎉' : `${rawPct}% Complete`}
          </span>
        </div>

        <div class="savings-numbers">
          <span class="savings-current">₹${saved.toLocaleString('en-IN')}</span>
          <span class="savings-target">/ ₹${target.toLocaleString('en-IN')}</span>
        </div>

        <div class="progress-container" style="height: 10px; margin-bottom: 12px;">
          <div class="progress-bar ${isDone ? 'progress-normal' : 'progress-normal'}" style="width: ${barPct}%; background: linear-gradient(90deg, #8b5cf6, #6366f1);"></div>
        </div>

        <div style="display: flex; justify-content: space-between; font-size: 0.8rem; color: var(--text-muted); margin-bottom: 18px;">
          <span>To go: <strong style="color: #cbd5e1;">₹${remaining.toLocaleString('en-IN')}</strong></span>
          <span>Target Date: <strong style="color: #cbd5e1;">${FinTrackAPI.formatDate(g.targetDate) || 'Open Target'}</strong></span>
        </div>

        <div style="display: flex; gap: 8px; margin-top: auto; padding-top: 14px; border-top: 1px solid var(--border-color);">
          <button class="btn btn-primary btn-sm add-money-btn" style="flex: 1;" data-id="${g.id}" data-name="${escapeHtml(g.name)}">
            + Add Money
          </button>
          <button class="btn btn-secondary btn-sm edit-goal-btn" data-id="${g.id}">Edit</button>
          <button class="btn btn-danger btn-sm delete-goal-btn" data-id="${g.id}">Delete</button>
        </div>
      </div>
    `;
  }).join('');

  document.querySelectorAll('.add-money-btn').forEach(btn => {
    btn.addEventListener('click', () => openDepositModal(btn.dataset.id, btn.dataset.name));
  });

  document.querySelectorAll('.edit-goal-btn').forEach(btn => {
    btn.addEventListener('click', () => openEditGoalModal(btn.dataset.id));
  });

  document.querySelectorAll('.delete-goal-btn').forEach(btn => {
    btn.addEventListener('click', () => deleteGoal(btn.dataset.id));
  });
}

function setupSavingsModals() {
  // Goal Modal (Create / Edit)
  const goalModal = document.getElementById('savingsGoalModal');
  const openGoalBtn = document.getElementById('openAddGoalBtn');
  const closeGoalBtn = document.getElementById('closeGoalModal');
  const cancelGoalBtn = document.getElementById('cancelGoalBtn');
  const goalForm = document.getElementById('savingsGoalForm');

  if (openGoalBtn && goalModal) {
    openGoalBtn.addEventListener('click', () => {
      editingGoalId = null;
      document.getElementById('goalModalTitle').textContent = 'Create New Savings Goal';
      goalForm.reset();
      goalModal.classList.add('active');
    });
  }

  const closeGoalModal = () => {
    if (goalModal) goalModal.classList.remove('active');
    editingGoalId = null;
  };

  if (closeGoalBtn) closeGoalBtn.addEventListener('click', closeGoalModal);
  if (cancelGoalBtn) cancelGoalBtn.addEventListener('click', closeGoalModal);

  if (goalForm) {
    goalForm.addEventListener('submit', async (e) => {
      e.preventDefault();
      const name = document.getElementById('goalName').value;
      const targetAmount = parseFloat(document.getElementById('goalTargetAmount').value);
      const currentAmount = parseFloat(document.getElementById('goalCurrentAmount').value) || 0.0;
      const targetDate = document.getElementById('goalTargetDate').value;
      const category = document.getElementById('goalCategory').value;

      if (!name.trim()) {
        FinTrackAPI.showToast('Please provide a goal name.', 'error');
        return;
      }
      if (!targetAmount || targetAmount <= 0) {
        FinTrackAPI.showToast('Target amount must be greater than zero.', 'error');
        return;
      }

      const payload = { name, targetAmount, currentAmount, targetDate, category };

      try {
        if (editingGoalId) {
          await FinTrackAPI.put(`/api/savings-goals/${editingGoalId}`, payload);
          FinTrackAPI.showToast('Savings goal updated successfully!', 'success');
        } else {
          await FinTrackAPI.post('/api/savings-goals', payload);
          FinTrackAPI.showToast('New savings target created!', 'success');
        }
        closeGoalModal();
        loadSavingsGoals();
      } catch (err) {
        FinTrackAPI.showToast(err.message || 'Unable to save goal.', 'error');
      }
    });
  }

  // Quick "+ Add Money" Modal
  const depositModal = document.getElementById('addMoneyModal');
  const closeDepositBtn = document.getElementById('closeDepositModal');
  const cancelDepositBtn = document.getElementById('cancelDepositBtn');
  const depositForm = document.getElementById('addMoneyForm');

  const closeDepositModal = () => {
    if (depositModal) depositModal.classList.remove('active');
    depositGoalId = null;
  };

  if (closeDepositBtn) closeDepositBtn.addEventListener('click', closeDepositModal);
  if (cancelDepositBtn) cancelDepositBtn.addEventListener('click', closeDepositModal);

  if (depositForm) {
    depositForm.addEventListener('submit', async (e) => {
      e.preventDefault();
      const depositAmount = parseFloat(document.getElementById('depositAmount').value);

      if (!depositAmount || depositAmount <= 0) {
        FinTrackAPI.showToast('Deposit amount must be greater than zero.', 'error');
        return;
      }

      try {
        await FinTrackAPI.post(`/api/savings-goals/${depositGoalId}/add-money`, {
          amount: depositAmount
        });
        FinTrackAPI.showToast(`₹${depositAmount.toLocaleString('en-IN')} deposited to your goal!`, 'success');
        closeDepositModal();
        loadSavingsGoals();
      } catch (err) {
        FinTrackAPI.showToast(err.message || 'Deposit failed.', 'error');
      }
    });
  }
}

function openDepositModal(id, name) {
  depositGoalId = id;
  const depositModal = document.getElementById('addMoneyModal');
  document.getElementById('depositGoalNameDisplay').textContent = name;
  document.getElementById('depositAmount').value = '';
  depositModal.classList.add('active');
}

function openEditGoalModal(id) {
  const g = savingsList.find(x => x.id === id);
  if (!g) return;

  editingGoalId = id;
  const modal = document.getElementById('savingsGoalModal');
  document.getElementById('goalModalTitle').textContent = 'Edit Savings Goal';

  document.getElementById('goalName').value = g.name;
  document.getElementById('goalTargetAmount').value = g.targetAmount;
  document.getElementById('goalCurrentAmount').value = g.currentAmount;
  document.getElementById('goalTargetDate').value = g.targetDate;
  document.getElementById('goalCategory').value = g.category || 'General';

  modal.classList.add('active');
}

async function deleteGoal(id) {
  if (!confirm('Are you sure you want to delete this savings goal?')) return;

  try {
    await FinTrackAPI.delete(`/api/savings-goals/${id}`);
    FinTrackAPI.showToast('Goal deleted successfully.', 'success');
    loadSavingsGoals();
  } catch (err) {
    FinTrackAPI.showToast(err.message || 'Unable to delete goal.', 'error');
  }
}

function escapeHtml(str) {
  if (!str) return '';
  return str.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

