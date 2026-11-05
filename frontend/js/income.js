/**
 * FINTRACK - Income Management Controller
 */

let incomeChart = null;
let incomeList = [];
let editingIncomeId = null;

document.addEventListener('DOMContentLoaded', () => {
  FinTrackAuth.checkAuth(true);
  loadIncomeData();
  setupIncomeModal();
});

async function loadIncomeData() {
  try {
    const res = await FinTrackAPI.get('/api/income');
    if (!res) return;

    incomeList = res.incomeList || [];

    // Stats Cards
    const cur = '₹';
    document.getElementById('totalIncomeDisplay').textContent = FinTrackAPI.formatCurrency(res.totalIncome, cur);
    document.getElementById('thisMonthIncomeDisplay').textContent = FinTrackAPI.formatCurrency(res.thisMonth, cur);
    document.getElementById('avgMonthlyIncomeDisplay').textContent = FinTrackAPI.formatCurrency(res.averageMonthly, cur);

    renderIncomeChart(incomeList);
    renderIncomeTable(incomeList);
  } catch (err) {
    console.error('Failed to load income:', err);
    FinTrackAPI.showToast('Unable to fetch income data.', 'error');
  }
}

function renderIncomeChart(list) {
  const ctx = document.getElementById('incomeSourceChart');
  if (!ctx) return;

  if (incomeChart) {
    incomeChart.destroy();
  }

  // Aggregate by source
  const sourceMap = {};
  list.forEach(item => {
    const src = item.source || item.category || 'Salary';
    sourceMap[src] = (sourceMap[src] || 0) + Number(item.amount || 0);
  });

  const labels = Object.keys(sourceMap).length ? Object.keys(sourceMap) : ['Salary', 'Freelance', 'Other'];
  const data = Object.keys(sourceMap).length ? Object.values(sourceMap) : [50000, 15000, 10000];

  incomeChart = new Chart(ctx, {
    type: 'pie',
    data: {
      labels: labels,
      datasets: [{
        data: data,
        backgroundColor: ['#10b981', '#3b82f6', '#8b5cf6', '#f59e0b', '#ec4899', '#64748b'],
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
      }
    }
  });
}

function renderIncomeTable(list) {
  const tbody = document.getElementById('incomeTableBody');
  if (!tbody) return;

  if (!list.length) {
    tbody.innerHTML = `<tr><td colspan="5" class="text-muted" style="text-align: center; padding: 32px;">No income records recorded yet.</td></tr>`;
    return;
  }

  tbody.innerHTML = list.map(item => {
    return `
      <tr>
        <td><span class="mono text-secondary">${FinTrackAPI.formatDate(item.date)}</span></td>
        <td>
          <div style="font-weight: 700; color: #ffffff;">${escapeHtml(item.description || item.category)}</div>
          <div style="font-size: 0.75rem; color: var(--text-muted);">${item.paymentMethod || 'Bank Transfer'}</div>
        </td>
        <td><span class="badge badge-safe">${item.source || item.category}</span></td>
        <td><span class="text-income mono" style="font-weight: 800; font-size: 1.05rem;">+₹${Number(item.amount).toLocaleString('en-IN')}</span></td>
        <td style="text-align: right;">
          <div style="display: inline-flex; gap: 8px;">
            <button class="btn btn-secondary btn-sm edit-income-btn" data-id="${item.id}">Edit</button>
            <button class="btn btn-danger btn-sm delete-income-btn" data-id="${item.id}">Delete</button>
          </div>
        </td>
      </tr>
    `;
  }).join('');

  document.querySelectorAll('.edit-income-btn').forEach(btn => {
    btn.addEventListener('click', () => openEditIncomeModal(btn.dataset.id));
  });

  document.querySelectorAll('.delete-income-btn').forEach(btn => {
    btn.addEventListener('click', () => deleteIncome(btn.dataset.id));
  });
}

function setupIncomeModal() {
  const modal = document.getElementById('incomeModal');
  const openBtn = document.getElementById('openAddIncomeBtn');
  const closeBtn = document.getElementById('closeIncomeModal');
  const cancelBtn = document.getElementById('cancelIncomeBtn');
  const form = document.getElementById('incomeForm');

  if (openBtn && modal) {
    openBtn.addEventListener('click', () => {
      editingIncomeId = null;
      document.getElementById('incomeModalTitle').textContent = 'Add New Income';
      form.reset();
      document.getElementById('incomeDate').value = new Date().toISOString().split('T')[0];
      modal.classList.add('active');
    });
  }

  const closeModal = () => {
    if (modal) modal.classList.remove('active');
    editingIncomeId = null;
  };

  if (closeBtn) closeBtn.addEventListener('click', closeModal);
  if (cancelBtn) cancelBtn.addEventListener('click', closeModal);

  if (form) {
    form.addEventListener('submit', async (e) => {
      e.preventDefault();
      const amount = parseFloat(document.getElementById('incomeAmount').value);
      const source = document.getElementById('incomeSource').value;
      const description = document.getElementById('incomeDescription').value;
      const date = document.getElementById('incomeDate').value;
      const paymentMethod = document.getElementById('incomePaymentMethod').value;

      if (!amount || amount <= 0) {
        FinTrackAPI.showToast('Income amount must be greater than zero.', 'error');
        return;
      }

      const payload = {
        amount,
        source,
        category: source,
        description,
        date,
        paymentMethod
      };

      try {
        if (editingIncomeId) {
          await FinTrackAPI.put(`/api/income/${editingIncomeId}`, payload);
          FinTrackAPI.showToast('Income updated successfully!', 'success');
        } else {
          await FinTrackAPI.post('/api/income', payload);
          FinTrackAPI.showToast('Income recorded successfully!', 'success');
        }
        closeModal();
        loadIncomeData();
      } catch (err) {
        FinTrackAPI.showToast(err.message || 'Unable to save income.', 'error');
      }
    });
  }
}

function openEditIncomeModal(id) {
  const item = incomeList.find(x => x.id === id);
  if (!item) return;

  editingIncomeId = id;
  const modal = document.getElementById('incomeModal');
  document.getElementById('incomeModalTitle').textContent = 'Edit Income Record';

  document.getElementById('incomeAmount').value = item.amount;
  document.getElementById('incomeSource').value = item.source || item.category || 'Salary';
  document.getElementById('incomeDescription').value = item.description || '';
  document.getElementById('incomeDate').value = item.date;
  document.getElementById('incomePaymentMethod').value = item.paymentMethod || 'Bank Transfer';

  modal.classList.add('active');
}

async function deleteIncome(id) {
  if (!confirm('Are you sure you want to delete this income record?')) return;

  try {
    await FinTrackAPI.delete(`/api/income/${id}`);
    FinTrackAPI.showToast('Income deleted successfully.', 'success');
    loadIncomeData();
  } catch (err) {
    FinTrackAPI.showToast(err.message || 'Unable to delete income.', 'error');
  }
}

function escapeHtml(str) {
  if (!str) return '';
  return str.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

