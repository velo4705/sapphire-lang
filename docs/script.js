// Simple particle background
function createParticles() {
    const hero = document.querySelector('.hero');
    const particlesContainer = document.createElement('div');
    particlesContainer.className = 'particles';
    particlesContainer.style.cssText = `
        position: absolute;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        overflow: hidden;
        pointer-events: none;
        z-index: 1;
    `;
    hero.appendChild(particlesContainer);

    for (let i = 0; i < 30; i++) {
        const particle = document.createElement('div');
        particle.style.cssText = `
            position: absolute;
            width: ${Math.random() * 3 + 1}px;
            height: ${Math.random() * 3 + 1}px;
            background: rgba(64, 224, 208, ${Math.random() * 0.4 + 0.2});
            border-radius: 50%;
            left: ${Math.random() * 100}%;
            top: ${Math.random() * 100}%;
            animation: particleFloat ${Math.random() * 10 + 10}s linear infinite;
            pointer-events: none;
        `;
        particlesContainer.appendChild(particle);
    }
}

// Add particle animation CSS
const style = document.createElement('style');
style.textContent = `
    @keyframes particleFloat {
        0% {
            transform: translateY(0) translateX(0);
            opacity: 0;
        }
        10% {
            opacity: 1;
        }
        90% {
            opacity: 1;
        }
        100% {
            transform: translateY(-100vh) translateX(${Math.random() * 100 - 50}px);
            opacity: 0;
        }
    }
`;
document.head.appendChild(style);

createParticles();

// Anime.js animations
document.addEventListener('DOMContentLoaded', () => {
    // Hero animations
    anime({
        targets: '.hero-logo',
        opacity: [0, 1],
        scale: [0.5, 1],
        duration: 1000,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.hero-title',
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
        targets: '.hero-description',
        opacity: [0, 1],
        translateY: [20, 0],
        duration: 800,
        delay: 600,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.hero-btn',
        opacity: [0, 1],
        scale: [0.9, 1],
        duration: 600,
        delay: 800,
        easing: 'easeOutExpo'
    });

    anime({
        targets: '.stat',
        opacity: [0, 1],
        translateY: [20, 0],
        duration: 800,
        delay: anime.stagger(100, {start: 1000}),
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

    document.querySelectorAll('.card, .code-card').forEach(el => {
        fadeInObserver.observe(el);
    });

    // Section title animations
    const titleObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                anime({
                    targets: entry.target,
                    opacity: [0, 1],
                    translateY: [30, 0],
                    duration: 1000,
                    easing: 'easeOutExpo'
                });
                titleObserver.unobserve(entry.target);
            }
        });
    }, observerOptions);

    document.querySelectorAll('.section-title').forEach(el => {
        titleObserver.observe(el);
    });
});

// Smooth scroll for navigation links
document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function (e) {
        e.preventDefault();
        const targetId = this.getAttribute('href');
        const target = document.querySelector(targetId);
        if (target) {
            const navHeight = document.querySelector('.nav').offsetHeight;
            const targetPosition = target.getBoundingClientRect().top + window.pageYOffset - navHeight;
            
            window.scrollTo({
                top: targetPosition,
                behavior: 'smooth'
            });
        }
    });
});
