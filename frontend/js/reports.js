/**
 * FINTRACK - Financial Reports Controller
 * Monthly, Quarterly, and Yearly financial statements with PDF print formatting.
 */

document.addEventListener('DOMContentLoaded', () => {
  FinTrackAuth.checkAuth(true);
  loadReport();

  const generateBtn = document.getElementById('generateReportBtn');
  const printBtn = document.getElementById('printReportBtn');

  if (generateBtn) {
    generateBtn.addEventListener('click', loadReport);
  }

  if (printBtn) {
    printBtn.addEventListener('click', () => {
      window.print();
    });
  }
});

async function loadReport() {
  const period = document.getElementById('reportPeriod')?.value || 'monthly';
  const year = document.getElementById('reportYear')?.value || '2026';
  const month = document.getElementById('reportMonth')?.value || '11';

  try {
    const res = await FinTrackAPI.get('/api/reports', { period, year, month });
    if (!res || !res.report) return;

    renderReport(res.report);
  } catch (err) {
    console.error('Failed to generate report:', err);
    FinTrackAPI.showToast('Unable to generate financial report.', 'error');
  }
}

function renderReport(report) {
  const cur = '₹';
  const summary = report.summary || {};

  // Executive Metrics
  document.getElementById('reportTotalIncome').textContent = FinTrackAPI.formatCurrency(summary.totalIncome, cur);
  document.getElementById('reportTotalExpenses').textContent = FinTrackAPI.formatCurrency(summary.totalExpenses, cur);
  document.getElementById('reportNetSavings').textContent = FinTrackAPI.formatCurrency(summary.netSavings, cur);
  document.getElementById('reportSavingsRate').textContent = `${Number(summary.savingsRate).toFixed(1)}%`;

  document.getElementById('reportLargestExpense').textContent = `${summary.largestExpenseDescription || 'N/A'} (₹${Number(summary.largestExpenseAmount || 0).toLocaleString('en-IN')})`;
  document.getElementById('reportTopCategory').textContent = `${summary.topSpendingCategory || 'N/A'} (₹${Number(summary.topSpendingCategoryAmount || 0).toLocaleString('en-IN')})`;

  // Health Score & Advice
  const scoreEl = document.getElementById('reportHealthScore');
  const adviceEl = document.getElementById('reportRecommendation');

  if (scoreEl) scoreEl.textContent = report.healthScore || 'Grade A';
  if (adviceEl) adviceEl.textContent = report.recommendation || 'Solid financial balance.';

  // Report Period Header Info
  const periodBadge = document.getElementById('reportPeriodDisplay');
  if (periodBadge) {
    let pText = `${report.period.toUpperCase()} STATEMENT — `;
    if (report.period === 'monthly') pText += `November ${report.year}`;
    else if (report.period === 'quarterly') pText += `Q4 ${report.year}`;
    else pText += `Annual ${report.year}`;
    periodBadge.textContent = pText;
  }

  // Category Breakdown Table
  const catTbody = document.getElementById('reportCategoryBody');
  if (catTbody) {
    const cats = report.categoryBreakdown || [];
    if (!cats.length) {
      catTbody.innerHTML = `<tr><td colspan="3" class="text-muted" style="text-align: center; padding: 20px;">No expense categories recorded in this period.</td></tr>`;
    } else {
      catTbody.innerHTML = cats.map(c => `
        <tr>
          <td><strong style="color: #ffffff;">${c.category}</strong></td>
          <td class="mono" style="font-weight: 700; color: #ef4444;">₹${Number(c.amount).toLocaleString('en-IN')}</td>
          <td>
            <div style="display: flex; align-items: center; gap: 8px;">
              <div class="progress-container" style="flex: 1; height: 6px;">
                <div class="progress-bar progress-normal" style="width: ${Math.round(c.percentage)}%;"></div>
              </div>
              <span class="mono" style="font-size: 0.8rem; color: var(--text-muted); width: 40px; text-align: right;">${Math.round(c.percentage)}%</span>
            </div>
          </td>
        </tr>
      `).join('');
    }
  }

  // Transactions Audit Table
  const txTbody = document.getElementById('reportTransactionsBody');
  if (txTbody) {
    const txs = report.transactions || [];
    if (!txs.length) {
      txTbody.innerHTML = `<tr><td colspan="5" class="text-muted" style="text-align: center; padding: 20px;">No transactions logged in this period.</td></tr>`;
    } else {
      txTbody.innerHTML = txs.map(tx => {
        const isInc = tx.type === 'income';
        return `
          <tr>
            <td class="mono text-secondary">${FinTrackAPI.formatDate(tx.date)}</td>
            <td style="color: #ffffff; font-weight: 600;">${tx.description || tx.category}</td>
            <td>${tx.category}</td>
            <td><span class="badge ${isInc ? 'badge-income' : 'badge-expense'}">${tx.type}</span></td>
            <td class="mono ${isInc ? 'text-income' : 'text-expense'}" style="font-weight: 800; text-align: right;">
              ${isInc ? '+' : '-'}₹${Number(tx.amount).toLocaleString('en-IN')}
            </td>
          </tr>
        `;
      }).join('');
    }
  }
}

