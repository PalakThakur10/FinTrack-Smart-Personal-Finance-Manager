/**
 * FINTRACK - API Communication & Utility Layer
 * Communicates with Crow C++ Backend via standard Fetch API.
 * Falls back to built-in demo data if server is unreachable.
 */

const API_BASE = '';

// ─── BUILT-IN DEMO DATA (used when C++ server is offline) ───────────────────
const DEMO_DATA = {
  user: { id: '65e8a1f2b3c4d5e6f7a8b901', name: 'Alex Mercer', email: 'demo@fintrack.com', currency: 'INR' },
  transactions: [
    { id:'tx001', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'income',  amount:50000, category:'Salary',        description:'Monthly Software Engineer Salary',         date:'2026-11-01', paymentMethod:'Bank Transfer' },
    { id:'tx002', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'income',  amount:15000, category:'Freelance',      description:'UI/UX Client Project Milestone',            date:'2026-11-03', paymentMethod:'UPI' },
    { id:'tx003', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'income',  amount:10000, category:'Other',          description:'Quarterly Mutual Fund Dividend',            date:'2026-11-04', paymentMethod:'Bank Transfer' },
    { id:'tx004', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'expense', amount:4200,  category:'Food',           description:'Supermarket Monthly Grocery',               date:'2026-11-02', paymentMethod:'UPI' },
    { id:'tx005', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'expense', amount:2100,  category:'Transport',      description:'Metro SmartCard & Fuel Recharge',           date:'2026-11-02', paymentMethod:'Debit Card' },
    { id:'tx006', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'expense', amount:5500,  category:'Shopping',       description:'Amazon Ergonomic Chair & Tech Accessories', date:'2026-11-03', paymentMethod:'Credit Card' },
    { id:'tx007', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'expense', amount:2500,  category:'Entertainment',  description:'Weekend Movies & Netflix Subscription',     date:'2026-11-04', paymentMethod:'UPI' },
    { id:'tx008', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'expense', amount:3000,  category:'Education',      description:'C++ Advanced Systems & Algorithms Course',  date:'2026-11-04', paymentMethod:'Credit Card' },
    { id:'tx009', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'expense', amount:4020,  category:'Bills',          description:'Fiber Broadband & Electricity Bills',       date:'2026-11-05', paymentMethod:'Bank Transfer' },
    { id:'tx010', userId:'65e8a1f2b3c4d5e6f7a8b901', type:'expense', amount:8000,  category:'Health',         description:'Quarterly Health Insurance Premium',        date:'2026-11-05', paymentMethod:'Bank Transfer' }
  ],
  budgets: [
    { id:'b001', userId:'65e8a1f2b3c4d5e6f7a8b901', category:'Food',          limit:6000, spent:4200, month:11, year:2026 },
    { id:'b002', userId:'65e8a1f2b3c4d5e6f7a8b901', category:'Transport',     limit:4000, spent:2100, month:11, year:2026 },
    { id:'b003', userId:'65e8a1f2b3c4d5e6f7a8b901', category:'Shopping',      limit:8000, spent:5500, month:11, year:2026 },
    { id:'b004', userId:'65e8a1f2b3c4d5e6f7a8b901', category:'Entertainment', limit:3000, spent:2500, month:11, year:2026 },
    { id:'b005', userId:'65e8a1f2b3c4d5e6f7a8b901', category:'Education',     limit:5000, spent:3000, month:11, year:2026 }
  ],
  savingsGoals: [
    { id:'sg001', userId:'65e8a1f2b3c4d5e6f7a8b901', name:'New Laptop',     targetAmount:80000, currentAmount:45000, targetDate:'2027-03-31', category:'Electronics' },
    { id:'sg002', userId:'65e8a1f2b3c4d5e6f7a8b901', name:'Emergency Fund', targetAmount:50000, currentAmount:30000, targetDate:'2027-06-30', category:'Safety' },
    { id:'sg003', userId:'65e8a1f2b3c4d5e6f7a8b901', name:'New Phone',      targetAmount:40000, currentAmount:18000, targetDate:'2026-12-31', category:'Gadgets' }
  ]
};

// ─── OFFLINE STORE: persisted in sessionStorage ──────────────────────────────
let offlineMode = false;

function getOfflineStore() {
  try {
    const s = sessionStorage.getItem('fintrack_offline');
    return s ? JSON.parse(s) : JSON.parse(JSON.stringify(DEMO_DATA));
  } catch { return JSON.parse(JSON.stringify(DEMO_DATA)); }
}
function saveOfflineStore(store) {
  sessionStorage.setItem('fintrack_offline', JSON.stringify(store));
}

function generateId() {
  return Date.now().toString(36) + Math.random().toString(36).substr(2, 5);
}

// ─── OFFLINE API HANDLER ─────────────────────────────────────────────────────
function handleOfflineRequest(method, endpoint, body) {
  const store = getOfflineStore();
  const userId = '65e8a1f2b3c4d5e6f7a8b901';

  // AUTH
  if (endpoint === '/api/auth/login' && method === 'POST') {
    if (body.email === 'demo@fintrack.com' && body.password === 'Demo@123') {
      return { status: 'success', token: 'demo-offline-token', user: store.user };
    }
    throw new Error('Invalid email or password.');
  }
  if (endpoint === '/api/auth/register' && method === 'POST') {
    return { status: 'success', token: 'demo-offline-token', user: { ...store.user, name: body.name, email: body.email } };
  }
  if (endpoint === '/api/auth/logout') return { status: 'success' };

  // DASHBOARD
  if (endpoint === '/api/dashboard') {
    const txs = store.transactions;
    const income = txs.filter(t => t.type === 'income').reduce((s, t) => s + t.amount, 0);
    const expenses = txs.filter(t => t.type === 'expense').reduce((s, t) => s + t.amount, 0);
    const savings = income - expenses;
    const catMap = {};
    txs.filter(t => t.type === 'expense').forEach(t => { catMap[t.category] = (catMap[t.category] || 0) + t.amount; });
    const monthlyLabels = ['Jun','Jul','Aug','Sep','Oct','Nov'];
    const monthlyIncome =   [38000, 42000, 55000, 47000, 60000, income];
    const monthlyExpenses = [22000, 25000, 30000, 28000, 35000, expenses];
    return {
      summary: {
        totalIncome: income, totalExpenses: expenses,
        totalBalance: savings, totalSavings: savings,
        savingsRate: income > 0 ? ((savings / income) * 100).toFixed(1) : 0
      },
      recentTransactions: txs.slice().reverse().slice(0, 8),
      budgets: store.budgets,
      monthlyTrend: {
        labels: monthlyLabels,
        income: monthlyIncome,
        expenses: monthlyExpenses
      },
      expenseBreakdown: Object.entries(catMap).map(([k,v]) => ({ category: k, amount: v }))
    };
  }

  // TRANSACTIONS
  if (endpoint.startsWith('/api/transactions') || endpoint.startsWith('/api/income') || endpoint.startsWith('/api/expenses')) {
    const isIncome = endpoint.startsWith('/api/income');
    const isExpense = endpoint.startsWith('/api/expenses');
    let txs = store.transactions;

    if (method === 'GET') {
      let result = txs;
      if (isIncome) result = txs.filter(t => t.type === 'income');
      if (isExpense) result = txs.filter(t => t.type === 'expense');
      const totalIncome = result.filter(t => t.type === 'income').reduce((s,t) => s+t.amount, 0);
      const totalExpense = result.filter(t => t.type === 'expense').reduce((s,t) => s+t.amount, 0);
      return { transactions: result, total: result.length, totalIncome, totalExpenses: totalExpense };
    }

    if (method === 'POST') {
      const newTx = { ...body, id: generateId(), userId, createdAt: new Date().toISOString() };
      store.transactions.push(newTx);
      saveOfflineStore(store);
      return { status: 'success', transaction: newTx };
    }

    // PUT /api/transactions/:id
    const idMatch = endpoint.match(/\/(\w+)$/);
    if (idMatch) {
      const id = idMatch[1];
      if (method === 'PUT') {
        const idx = store.transactions.findIndex(t => t.id === id);
        if (idx !== -1) { store.transactions[idx] = { ...store.transactions[idx], ...body }; saveOfflineStore(store); }
        return { status: 'success', transaction: store.transactions[idx] };
      }
      if (method === 'DELETE') {
        store.transactions = store.transactions.filter(t => t.id !== id);
        saveOfflineStore(store);
        return { status: 'success' };
      }
    }
  }

  // BUDGETS
  if (endpoint.startsWith('/api/budgets')) {
    if (method === 'GET') return { budgets: store.budgets };
    if (method === 'POST') {
      const nb = { ...body, id: generateId(), userId, createdAt: new Date().toISOString() };
      store.budgets.push(nb); saveOfflineStore(store);
      return { status: 'success', budget: nb };
    }
    const idM = endpoint.match(/\/(\w+)$/);
    if (idM) {
      const id = idM[1];
      if (method === 'PUT') {
        const idx = store.budgets.findIndex(b => b.id === id);
        if (idx !== -1) { store.budgets[idx] = { ...store.budgets[idx], ...body }; saveOfflineStore(store); }
        return { status: 'success' };
      }
      if (method === 'DELETE') {
        store.budgets = store.budgets.filter(b => b.id !== id);
        saveOfflineStore(store);
        return { status: 'success' };
      }
    }
  }

  // SAVINGS GOALS
  if (endpoint.startsWith('/api/savings')) {
    if (method === 'GET') return { goals: store.savingsGoals };
    if (method === 'POST') {
      const ng = { ...body, id: generateId(), userId, currentAmount: body.currentAmount || 0, createdAt: new Date().toISOString() };
      store.savingsGoals.push(ng); saveOfflineStore(store);
      return { status: 'success', goal: ng };
    }
    const idM = endpoint.match(/\/(\w+)$/);
    if (idM) {
      const id = idM[1];
      if (method === 'PUT') {
        const idx = store.savingsGoals.findIndex(g => g.id === id);
        if (idx !== -1) { store.savingsGoals[idx] = { ...store.savingsGoals[idx], ...body }; saveOfflineStore(store); }
        return { status: 'success' };
      }
      if (method === 'DELETE') {
        store.savingsGoals = store.savingsGoals.filter(g => g.id !== id);
        saveOfflineStore(store);
        return { status: 'success' };
      }
    }
  }

  // ANALYTICS
  if (endpoint.startsWith('/api/analytics')) {
    const txs = store.transactions;
    const income = txs.filter(t => t.type === 'income').reduce((s, t) => s + t.amount, 0);
    const expenses = txs.filter(t => t.type === 'expense').reduce((s, t) => s + t.amount, 0);
    const catMap = {};
    txs.filter(t => t.type === 'expense').forEach(t => { catMap[t.category] = (catMap[t.category] || 0) + t.amount; });
    return {
      totalIncome: income, totalExpenses: expenses,
      monthlyLabels: ['Jun','Jul','Aug','Sep','Oct','Nov'],
      monthlyIncome:   [38000,42000,55000,47000,60000,income],
      monthlyExpenses: [22000,25000,30000,28000,35000,expenses],
      monthlySavings:  [16000,17000,25000,19000,25000,income-expenses],
      expenseByCategory: Object.entries(catMap).map(([k,v]) => ({ category: k, amount: v })),
      budgetPerformance: store.budgets
    };
  }

  // REPORTS
  if (endpoint.startsWith('/api/reports')) {
    const txs = store.transactions;
    const income = txs.filter(t => t.type === 'income').reduce((s, t) => s + t.amount, 0);
    const expenses = txs.filter(t => t.type === 'expense').reduce((s, t) => s + t.amount, 0);
    const catMap = {};
    txs.filter(t => t.type === 'expense').forEach(t => { catMap[t.category] = (catMap[t.category] || 0) + t.amount; });
    const topCat = Object.entries(catMap).sort((a,b) => b[1]-a[1])[0];
    return {
      totalIncome: income, totalExpenses: expenses, netSavings: income - expenses,
      savingsRate: income > 0 ? ((income - expenses) / income * 100).toFixed(1) : 0,
      largestExpense: Math.max(...txs.filter(t => t.type === 'expense').map(t => t.amount), 0),
      topSpendingCategory: topCat ? topCat[0] : 'N/A',
      transactions: txs
    };
  }

  if (endpoint.startsWith('/api/demo/reset')) {
    sessionStorage.removeItem('fintrack_offline');
    return { status: 'success', message: 'Demo data reset successfully.' };
  }

  return { status: 'success', data: [] };
}

// ─── MAIN API OBJECT ─────────────────────────────────────────────────────────
const FinTrackAPI = {
  getToken() { return localStorage.getItem('fintrack_token') || ''; },
  setToken(token) {
    if (token) localStorage.setItem('fintrack_token', token);
    else localStorage.removeItem('fintrack_token');
  },
  getUser() {
    try { const d = localStorage.getItem('fintrack_user'); return d ? JSON.parse(d) : null; }
    catch { return null; }
  },
  setUser(user) {
    if (user) localStorage.setItem('fintrack_user', JSON.stringify(user));
    else localStorage.removeItem('fintrack_user');
  },
  clearSession() {
    localStorage.removeItem('fintrack_token');
    localStorage.removeItem('fintrack_user');
  },

  async request(endpoint, options = {}) {
    const method = (options.method || 'GET').toUpperCase();
    const body = options.body ? JSON.parse(options.body) : {};

    // If already in offline mode, skip fetch entirely
    if (offlineMode) {
      try { return handleOfflineRequest(method, endpoint, body); }
      catch(e) { throw e; }
    }

    const url = `${API_BASE}${endpoint}`;
    const headers = { 'Content-Type': 'application/json', ...(options.headers || {}) };
    const token = this.getToken();
    if (token) headers['Authorization'] = `Bearer ${token}`;

    try {
      const response = await fetch(url, { ...options, headers });

      if (response.status === 401 && !endpoint.includes('/api/auth/login')) {
        this.clearSession();
        window.location.href = '../login.html';
        throw new Error('Session expired. Please log in again.');
      }

      const contentType = response.headers.get('content-type') || '';
      let data = {};
      if (contentType.includes('application/json')) {
        data = await response.json();
      } else {
        const text = await response.text();
        try { data = JSON.parse(text); } catch { data = { message: text }; }
      }

      if (!response.ok) throw new Error(data.message || data.error || `HTTP ${response.status} Error`);
      return data;

    } catch (err) {
      // Network error — switch to offline mode
      if (err.name === 'TypeError' || err.message.includes('fetch') || err.message.includes('NetworkError') || err.message.includes('Failed to fetch')) {
        offlineMode = true;
        console.warn('[FinTrack] Server unreachable — switching to offline demo mode.');
        try { return handleOfflineRequest(method, endpoint, body); }
        catch(e2) { throw e2; }
      }
      console.error(`[API Error] ${endpoint}:`, err);
      throw err;
    }
  },

  get(endpoint, params = {}) {
    const query = new URLSearchParams(params).toString();
    const url = query ? `${endpoint}?${query}` : endpoint;
    return this.request(url, { method: 'GET' });
  },
  post(endpoint, body = {}) {
    return this.request(endpoint, { method: 'POST', body: JSON.stringify(body) });
  },
  put(endpoint, body = {}) {
    return this.request(endpoint, { method: 'PUT', body: JSON.stringify(body) });
  },
  delete(endpoint) {
    return this.request(endpoint, { method: 'DELETE' });
  },

  formatCurrency(amount, currency = '₹') {
    const num = Number(amount) || 0;
    const formatted = new Intl.NumberFormat('en-IN', {
      maximumFractionDigits: 2,
      minimumFractionDigits: (num % 1 !== 0) ? 2 : 0
    }).format(num);
    return `${currency}${formatted}`;
  },

  formatDate(dateStr) {
    if (!dateStr) return '';
    const date = new Date(dateStr);
    if (isNaN(date.getTime())) return dateStr;
    return date.toLocaleDateString('en-GB', { day: '2-digit', month: 'short', year: 'numeric' });
  },

  showToast(message, type = 'info') {
    let container = document.getElementById('toast-container');
    if (!container) {
      container = document.createElement('div');
      container.id = 'toast-container';
      document.body.appendChild(container);
    }
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    let icon = 'ℹ️';
    if (type === 'success') icon = '✓';
    if (type === 'error') icon = '✕';
    if (type === 'warning') icon = '⚠';
    toast.innerHTML = `<span style="font-weight:bold;">${icon}</span><div style="flex:1;">${message}</div>`;
    container.appendChild(toast);
    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateX(100%)';
      toast.style.transition = 'all 0.3s ease';
      setTimeout(() => toast.remove(), 300);
    }, 4000);
  }
};
