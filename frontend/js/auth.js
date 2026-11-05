/**
 * FINTRACK - Authentication & Session Handler
 */

document.addEventListener('DOMContentLoaded', () => {
  // Mobile Sidebar Toggle
  const toggleBtn = document.getElementById('mobileMenuToggle');
  const sidebar = document.querySelector('.sidebar');
  const overlay = document.querySelector('.sidebar-overlay');

  if (toggleBtn && sidebar) {
    toggleBtn.addEventListener('click', () => {
      sidebar.classList.toggle('mobile-open');
      if (overlay) overlay.classList.toggle('active');
    });
  }

  if (overlay) {
    overlay.addEventListener('click', () => {
      if (sidebar) sidebar.classList.remove('mobile-open');
      overlay.classList.remove('active');
    });
  }

  // Populate user profile if present
  const user = FinTrackAPI.getUser();
  if (user) {
    const userNameEls = document.querySelectorAll('.user-name-display');
    const userEmailEls = document.querySelectorAll('.user-email-display');
    const userAvatarEls = document.querySelectorAll('.user-avatar-display');

    userNameEls.forEach(el => el.textContent = user.name || 'Alex Mercer');
    userEmailEls.forEach(el => el.textContent = user.email || 'demo@fintrack.com');
    userAvatarEls.forEach(el => {
      const initial = (user.name || 'Alex').charAt(0).toUpperCase();
      el.textContent = initial;
    });
  }

  // Logout Handler
  const logoutBtns = document.querySelectorAll('.btn-logout, #logoutBtn');
  logoutBtns.forEach(btn => {
    btn.addEventListener('click', async (e) => {
      e.preventDefault();
      try {
        await FinTrackAPI.post('/api/auth/logout');
      } catch (err) {
        console.warn('Logout API notification error:', err);
      } finally {
        FinTrackAPI.clearSession();
        window.location.href = '../login.html';
      }
    });
  });
});

const FinTrackAuth = {
  checkAuth(requireAuth = true) {
    const token = FinTrackAPI.getToken();
    const isAuthPage = window.location.pathname.includes('login.html') || 
                       window.location.pathname.includes('register.html') ||
                       window.location.pathname === '/' ||
                       window.location.pathname.endsWith('index.html');

    if (requireAuth && !token) {
      window.location.href = '../login.html';
      return false;
    }

    if (!requireAuth && token && isAuthPage) {
      window.location.href = '../pages/dashboard.html';
      return true;
    }

    return true;
  },

  async login(email, password) {
    if (!email || !password) {
      FinTrackAPI.showToast('Please enter both email and password.', 'error');
      return false;
    }

    try {
      const res = await FinTrackAPI.post('/api/auth/login', { email, password });
      if (res.status === 'success' && res.token) {
        FinTrackAPI.setToken(res.token);
        FinTrackAPI.setUser(res.user);
        FinTrackAPI.showToast('Welcome back to FinTrack!', 'success');
        setTimeout(() => {
          window.location.href = '../pages/dashboard.html';
        }, 600);
        return true;
      }
    } catch (err) {
      FinTrackAPI.showToast(err.message || 'Invalid email or password.', 'error');
      return false;
    }
  },

  async register(name, email, password, confirmPassword) {
    if (!name || !email || !password) {
      FinTrackAPI.showToast('Please fill out all required fields.', 'error');
      return false;
    }
    if (password !== confirmPassword) {
      FinTrackAPI.showToast('Passwords do not match.', 'error');
      return false;
    }

    try {
      const res = await FinTrackAPI.post('/api/auth/register', {
        name,
        email,
        password,
        confirmPassword
      });

      if (res.status === 'success' && res.token) {
        FinTrackAPI.setToken(res.token);
        FinTrackAPI.setUser(res.user);
        FinTrackAPI.showToast('Account created successfully!', 'success');
        setTimeout(() => {
          window.location.href = '../pages/dashboard.html';
        }, 600);
        return true;
      }
    } catch (err) {
      FinTrackAPI.showToast(err.message || 'Registration failed.', 'error');
      return false;
    }
  },

  fillDemoCredentials() {
    const emailInput = document.getElementById('email');
    const passInput = document.getElementById('password');
    if (emailInput && passInput) {
      emailInput.value = 'demo@fintrack.com';
      passInput.value = 'Demo@123';
      FinTrackAPI.showToast('Demo credentials autofilled!', 'info');
    }
  }
};

