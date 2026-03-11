// Anime.js animations
document.addEventListener('DOMContentLoaded', () => {
    // Home animations
    anime({
        targets: '.diamond-icon',
        opacity: [0, 1],
        scale: [0.5, 1],
        rotate: [-90, 0],
        duration: 1500,
        easing: 'easeOutElastic(1, .6)'
    });

    anime({
        targets: '.main-title',
        opacity: [0, 1],
        translateY: [40, 0],
        duration: 1000,
        delay: 300,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.main-tagline',
        opacity: [0, 1],
        translateY: [30, 0],
        duration: 800,
        delay: 500,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.main-description',
        opacity: [0, 1],
        translateY: [20, 0],
        duration: 800,
        delay: 700,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.quick-stat',
        opacity: [0, 1],
        translateY: [20, 0],
        duration: 800,
        delay: anime.stagger(100, {start: 900}),
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.main-cta',
        opacity: [0, 1],
        scale: [0.9, 1],
        duration: 600,
        delay: 1200,
        easing: 'easeOutExpo'
    });

    // Scroll animations
    const observerOptions = {
        threshold: 0.2,
        rootMargin: '0px 0px -100px 0px'
    };

    const fadeInObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                if (entry.target.classList.contains('card')) {
                    anime({
                        targets: entry.target,
                        opacity: [0, 1],
                        translateY: [50, 0],
                        duration: 800,
                        easing: 'easeOutExpo'
                    });
                } else if (entry.target.classList.contains('example-item')) {
                    anime({
                        targets: entry.target,
                        opacity: [0, 1],
                        scale: [0.95, 1],
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

    document.querySelectorAll('.card, .example-item, .install-section, .tools-section, .docs-section').forEach(el => {
        fadeInObserver.observe(el);
    });

    document.querySelectorAll('.panel-title').forEach(el => {
        fadeInObserver.observe(el);
    });
});

// Active nav item on scroll
const sections = document.querySelectorAll('.panel');
const navItems = document.querySelectorAll('.nav-item');

window.addEventListener('scroll', () => {
    let current = '';
    
    sections.forEach(section => {
        const sectionTop = section.offsetTop;
        const sectionHeight = section.clientHeight;
        if (window.pageYOffset >= sectionTop - 200) {
            current = section.getAttribute('id');
        }
    });

    navItems.forEach(item => {
        item.classList.remove('active');
        if (item.getAttribute('href') === `#${current}`) {
            item.classList.add('active');
        }
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
