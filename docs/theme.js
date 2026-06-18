// Theme toggle — dark/light mode with localStorage persistence
(function () {
    const STORAGE_KEY = 'sapphire-theme';

    function getPreferredTheme() {
        const saved = localStorage.getItem(STORAGE_KEY);
        if (saved === 'light' || saved === 'dark') return saved;
        return window.matchMedia('(prefers-color-scheme: light)').matches ? 'light' : 'dark';
    }

    function setTheme(theme) {
        document.documentElement.setAttribute('data-theme', theme);
        localStorage.setItem(STORAGE_KEY, theme);
        const btn = document.getElementById('theme-toggle');
        if (btn) {
            btn.textContent = theme === 'dark' ? '\u{1F319}' : '\u{2600}\u{FE0F}';
            btn.setAttribute('aria-label', theme === 'dark' ? 'Switch to light mode' : 'Switch to dark mode');
        }
    }

    function toggleTheme() {
        const current = document.documentElement.getAttribute('data-theme') || 'dark';
        setTheme(current === 'dark' ? 'light' : 'dark');
    }

    // Apply theme on load
    document.addEventListener('DOMContentLoaded', () => {
        setTheme(getPreferredTheme());
        const btn = document.getElementById('theme-toggle');
        if (btn) btn.addEventListener('click', toggleTheme);
    });
})();
