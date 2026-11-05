/**
 * FINTRACK - Transactions Page Controller
 * Handles full transaction CRUD, filtering, searching, and pagination.
 */

let allTransactions = [];
let editingTransactionId = null;

document.addEventListener('DOMContentLoaded', () => {
  FinTrackAuth.checkAuth(true);
  loadTransactions();
  setupTransactionEvents();
});

async function loadTransactions() {
  const typeFilter = document.getElementById('filterType')?.value || 'all';
  const categoryFilter = document.getElementById('filterCategory')?.value || 'all';
  const searchKeyword = document.getElementById('searchTransactions')?.value || '';
  const startDate = document.getElementById('filterStartDate')?.value || '';
  const endDate = document.getElementById('filterEndDate')?.value || '';

  try {
    const params = {};
    if (typeFilter !== 'all') params.type = typeFilter;
    if (categoryFilter !== 'all') params.category = categoryFilter;
    if (searchKeyword) params.search = searchKeyword;
    if (startDate) params.startDate = startDate;
    if (endDate) params.endDate = endDate;

    const res = await FinTrackAPI.get('/api/transactions', params);
    allTransactions = res.transactions || [];
    renderTransactionsTable(allTransactions);
  } catch (err) {
    console.error('Failed to load transactions:', err);
    FinTrackAPI.showToast('Unable to fetch transaction history.', 'error');
  }
}

function renderTransactionsTable(transactions) {
  const tbody = document.getElementById('transactionsTableBody');
  const countEl = document.getElementById('transactionCount');
  if (countEl) countEl.textContent = `${transactions.length} record${transactions.length === 1 ? '' : 's'}`;

  if (!tbody) return;

  if (!transactions.length) {
    tbody.innerHTML = `
      <tr>
        <td colspan="6" style="text-align: center; padding: 48px 20px;">
          <div style="font-size: 2.5rem; margin-bottom: 12px;">📂</div>
          <div style="font-weight: 700; font-size: 1.1rem; color: #ffffff;">No Transactions Found</div>
          <div class="text-muted" style="margin-top: 4px;">Try clearing filters or add a new transaction.</div>
        </td>
      </tr>
    `;
    return;
  }

  tbody.innerHTML = transactions.map(tx => {
    const isIncome = tx.type === 'income';
    const amountStr = isIncome
      ? `+₹${Number(tx.amount).toLocaleString('en-IN')}`
      : `-₹${Number(tx.amount).toLocaleString('en-IN')}`;
    const amountClass = isIncome ? 'text-income' : 'text-expense';
    const badgeClass = isIncome ? 'badge-income' : 'badge-expense';
    const icon = getCategoryIcon(tx.category);

    return `
      <tr data-id="${tx.id}">
        <td><span class="text-secondary mono" style="font-size: 0.85rem;">${FinTrackAPI.formatDate(tx.date)}</span></td>
        <td>
          <div class="table-tx-title">
            <div class="tx-cat-icon" style="background: ${isIncome ? 'rgba(16, 185, 129, 0.12)' : 'rgba(239, 68, 68, 0.12)'}; color: ${isIncome ? '#10b981' : '#ef4444'};">
              ${icon}
            </div>
            <div>
              <div style="font-weight: 700; color: #ffffff;">${escapeHtml(tx.description || tx.category)}</div>
              <div style="font-size: 0.75rem; color: var(--text-muted);">${tx.paymentMethod || 'UPI'}</div>
            </div>
          </div>
        </td>
        <td><span style="font-weight: 600;">${tx.category}</span></td>
        <td><span class="badge ${badgeClass}">${tx.type}</span></td>
        <td><span class="${amountClass} mono" style="font-weight: 800; font-size: 1.05rem;">${amountStr}</span></td>
        <td style="text-align: right;">
          <div style="display: inline-flex; gap: 8px;">
            <button class="btn btn-secondary btn-sm edit-tx-btn" data-id="${tx.id}">Edit</button>
            <button class="btn btn-danger btn-sm delete-tx-btn" data-id="${tx.id}">Delete</button>
          </div>
        </td>
      </tr>
    `;
  }).join('');

  // Attach event handlers
  document.querySelectorAll('.edit-tx-btn').forEach(btn => {
    btn.addEventListener('click', () => openEditModal(btn.dataset.id));
  });

  document.querySelectorAll('.delete-tx-btn').forEach(btn => {
    btn.addEventListener('click', () => deleteTransaction(btn.dataset.id));
  });
}

function setupTransactionEvents() {
  const searchInput = document.getElementById('searchTransactions');
  const typeFilter = document.getElementById('filterType');
  const categoryFilter = document.getElementById('filterCategory');
  const startDateFilter = document.getElementById('filterStartDate');
  const endDateFilter = document.getElementById('filterEndDate');
  const clearFilterBtn = document.getElementById('clearFiltersBtn');

  // Debounced search
  let debounceTimer;
  if (searchInput) {
    searchInput.addEventListener('input', () => {
      clearTimeout(debounceTimer);
      debounceTimer = setTimeout(loadTransactions, 300);
    });
  }

  [typeFilter, categoryFilter, startDateFilter, endDateFilter].forEach(el => {
    if (el) el.addEventListener('change', loadTransactions);
  });

  if (clearFilterBtn) {
    clearFilterBtn.addEventListener('click', () => {
      if (searchInput) searchInput.value = '';
      if (typeFilter) typeFilter.value = 'all';
      if (categoryFilter) categoryFilter.value = 'all';
      if (startDateFilter) startDateFilter.value = '';
      if (endDateFilter) endDateFilter.value = '';
      loadTransactions();
    });
  }

  // Modal setup
  const modal = document.getElementById('transactionModal');
  const openModalBtn = document.getElementById('openAddTransactionBtn');
  const closeModalBtn = document.getElementById('closeTxModal');
  const cancelBtn = document.getElementById('cancelTxBtn');
  const form = document.getElementById('transactionForm');

  if (openModalBtn && modal) {
    openModalBtn.addEventListener('click', () => {
      editingTransactionId = null;
      document.getElementById('modalTitle').textContent = 'Add New Transaction';
      form.reset();
      document.getElementById('txDate').value = new Date().toISOString().split('T')[0];
      modal.classList.add('active');
    });
  }

  const closeModal = () => {
    if (modal) modal.classList.remove('active');
    editingTransactionId = null;
  };

  if (closeModalBtn) closeModalBtn.addEventListener('click', closeModal);
  if (cancelBtn) cancelBtn.addEventListener('click', closeModal);

  if (form) {
    form.addEventListener('submit', async (e) => {
      e.preventDefault();
      const type = document.getElementById('txType').value;
      const amount = parseFloat(document.getElementById('txAmount').value);
      const category = document.getElementById('txCategory').value;
      const description = document.getElementById('txDescription').value;
      const date = document.getElementById('txDate').value;
      const paymentMethod = document.getElementById('txPaymentMethod').value;

      if (!amount || amount <= 0) {
        FinTrackAPI.showToast('Amount must be greater than zero.', 'error');
        return;
      }
      if (!description.trim()) {
        FinTrackAPI.showToast('Please provide a description.', 'error');
        return;
      }

      const payload = { type, amount, category, description, date, paymentMethod };

      try {
        if (editingTransactionId) {
          await FinTrackAPI.put(`/api/transactions/${editingTransactionId}`, payload);
          FinTrackAPI.showToast('Transaction updated successfully!', 'success');
        } else {
          await FinTrackAPI.post('/api/transactions', payload);
          FinTrackAPI.showToast('Transaction saved successfully!', 'success');
        }
        closeModal();
        loadTransactions();
      } catch (err) {
        FinTrackAPI.showToast(err.message || 'Unable to save transaction.', 'error');
      }
    });
  }
}

function openEditModal(id) {
  const tx = allTransactions.find(t => t.id === id);
  if (!tx) return;

  editingTransactionId = id;
  const modal = document.getElementById('transactionModal');
  document.getElementById('modalTitle').textContent = 'Edit Transaction';

  document.getElementById('txType').value = tx.type;
  document.getElementById('txAmount').value = tx.amount;
  document.getElementById('txCategory').value = tx.category;
  document.getElementById('txDescription').value = tx.description;
  document.getElementById('txDate').value = tx.date;
  document.getElementById('txPaymentMethod').value = tx.paymentMethod || 'UPI';

  modal.classList.add('active');
}

async function deleteTransaction(id) {
  if (!confirm('Are you sure you want to delete this transaction record?')) {
    return;
  }

  try {
    await FinTrackAPI.delete(`/api/transactions/${id}`);
    FinTrackAPI.showToast('Transaction deleted successfully.', 'success');
    loadTransactions();
  } catch (err) {
    FinTrackAPI.showToast(err.message || 'Unable to delete transaction.', 'error');
  }
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

function escapeHtml(str) {
  if (!str) return '';
  return str.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

