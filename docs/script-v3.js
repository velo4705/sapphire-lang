// Anime.js animations
document.addEventListener('DOMContentLoaded', () => {
    // Hero animations
    anime({
        targets: '.hero-icon-large img',
        opacity: [0, 1],
        scale: [0.3, 1],
        rotate: [-180, 0],
        duration: 1800,
        easing: 'easeOutElastic(1, .5)'
    });

    anime({
        targets: '.hero-title-large',
        opacity: [0, 1],
        translateY: [50, 0],
        duration: 1200,
        delay: 400,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.hero-slogan',
        opacity: [0, 1],
        translateY: [30, 0],
        duration: 1000,
        delay: 700,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.cta-button',
        opacity: [0, 1],
        scale: [0.8, 1],
        duration: 800,
        delay: 1000,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.scroll-hint',
        opacity: [0, 1],
        translateY: [20, 0],
        duration: 800,
        delay: 1200,
        easing: 'easeOutExpo'
    });

    // Scroll-triggered animations
    const observerOptions = {
        threshold: 0.15,
        rootMargin: '0px 0px -100px 0px'
    };

    const fadeInObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                if (entry.target.classList.contains('stat-box')) {
                    anime({
                        targets: entry.target,
                        opacity: [0, 1],
                        translateY: [40, 0],
                        duration: 800,
                        easing: 'easeOutExpo'
                    });
                } else if (entry.target.classList.contains('feature-item')) {
                    anime({
                        targets: entry.target,
                        opacity: [0, 1],
                        translateX: [-50, 0],
                        duration: 800,
                        easing: 'easeOutExpo'
                    });
                } else if (entry.target.classList.contains('example-card')) {
                    anime({
                        targets: entry.target,
                        opacity: [0, 1],
                        translateY: [50, 0],
                        duration: 800,
                        easing: 'easeOutExpo'
                    });
                } else {
                    anime({
                        targets: entry.target,
                        opacity: [0, 1],
                        translateY: [40, 0],
                        duration: 800,
                        easing: 'easeOutExpo'
                    });
                }
                fadeInObserver.unobserve(entry.target);
            }
        });
    }, observerOptions);

    // Observe elements
    document.querySelectorAll('.stat-box, .feature-item, .example-card, .install-card, .doc-link-item').forEach(el => {
        fadeInObserver.observe(el);
    });

    // Section headers
    document.querySelectorAll('.section-header').forEach(el => {
        fadeInObserver.observe(el);
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
    const nav = document.querySelector('.top-nav');
    if (window.scrollY > 100) {
        nav.style.background = 'rgba(10, 14, 26, 0.95)';
    } else {
        nav.style.background = 'rgba(10, 14, 26, 0.8)';
    }
});
