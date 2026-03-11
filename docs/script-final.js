// Anime.js animations
document.addEventListener('DOMContentLoaded', () => {
    // Hero animations
    anime({
        targets: '.hero-badge',
        opacity: [0, 1],
        translateY: [-20, 0],
        duration: 800,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.hero-h1',
        opacity: [0, 1],
        translateY: [30, 0],
        duration: 1000,
        delay: 200,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.hero-subtitle',
        opacity: [0, 1],
        translateY: [20, 0],
        duration: 800,
        delay: 400,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.hero-desc',
        opacity: [0, 1],
        translateY: [20, 0],
        duration: 800,
        delay: 600,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.hero-actions a',
        opacity: [0, 1],
        translateY: [20, 0],
        duration: 800,
        delay: anime.stagger(100, {start: 800}),
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.hero-stats-inline .stat-inline',
        opacity: [0, 1],
        translateY: [20, 0],
        duration: 800,
        delay: anime.stagger(100, {start: 1000}),
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.code-preview',
        opacity: [0, 1],
        translateX: [50, 0],
        duration: 1200,
        delay: 400,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.hero-diamond',
        opacity: [0, 1],
        scale: [0.5, 1],
        rotate: [-180, 0],
        duration: 1500,
        delay: 200,
        easing: 'easeOutElastic(1, .6)'
    });

    // Scroll animations
    const observerOptions = {
        threshold: 0.2,
        rootMargin: '0px 0px -100px 0px'
    };

    const fadeInObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                anime({
                    targets: entry.target,
                    opacity: [0, 1],
                    translateY: [40, 0],
                    duration: 800,
                    easing: 'easeOutExpo'
                });
                fadeInObserver.unobserve(entry.target);
            }
        });
    }, observerOptions);

    document.querySelectorAll('.feature-box, .doc-link').forEach(el => {
        fadeInObserver.observe(el);
    });
});

// Tab functionality
const tabs = document.querySelectorAll('.tab');
const tabPanels = document.querySelectorAll('.tab-panel');

tabs.forEach(tab => {
    tab.addEventListener('click', () => {
        const targetId = tab.getAttribute('data-tab');
        
        tabs.forEach(t => t.classList.remove('active'));
        tabPanels.forEach(p => p.classList.remove('active'));
        
        tab.classList.add('active');
        document.getElementById(targetId).classList.add('active');
    });
});

// Smooth scroll
document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function (e) {
        e.preventDefault();
        const target = document.querySelector(this.getAttribute('href'));
        if (target) {
            target.scrollIntoView({
                behavior: 'smooth',
                block: 'start'
            });
        }
    });
});

// Navbar background on scroll
window.addEventListener('scroll', () => {
    const nav = document.querySelector('.nav');
    if (window.scrollY > 50) {
        nav.style.background = 'rgba(10, 14, 26, 0.95)';
    } else {
        nav.style.background = 'rgba(10, 14, 26, 0.9)';
    }
});
