// Hero Canvas Animation
const canvas = document.getElementById('hero-canvas');
const ctx = canvas.getContext('2d');

canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

class Particle {
    constructor() {
        this.x = Math.random() * canvas.width;
        this.y = Math.random() * canvas.height;
        this.size = Math.random() * 2 + 1;
        this.speedX = Math.random() * 0.5 - 0.25;
        this.speedY = Math.random() * 0.5 - 0.25;
        this.opacity = Math.random() * 0.5 + 0.2;
    }

    update() {
        this.x += this.speedX;
        this.y += this.speedY;

        if (this.x > canvas.width) this.x = 0;
        if (this.x < 0) this.x = canvas.width;
        if (this.y > canvas.height) this.y = 0;
        if (this.y < 0) this.y = canvas.height;
    }

    draw() {
        ctx.fillStyle = `rgba(64, 224, 208, ${this.opacity})`;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
        ctx.fill();
    }
}

const particles = [];
for (let i = 0; i < 100; i++) {
    particles.push(new Particle());
}

function animateParticles() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    particles.forEach(particle => {
        particle.update();
        particle.draw();
    });

    // Draw connections
    particles.forEach((p1, i) => {
        particles.slice(i + 1).forEach(p2 => {
            const dx = p1.x - p2.x;
            const dy = p1.y - p2.y;
            const distance = Math.sqrt(dx * dx + dy * dy);

            if (distance < 100) {
                ctx.strokeStyle = `rgba(64, 224, 208, ${0.1 * (1 - distance / 100)})`;
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.moveTo(p1.x, p1.y);
                ctx.lineTo(p2.x, p2.y);
                ctx.stroke();
            }
        });
    });

    requestAnimationFrame(animateParticles);
}

animateParticles();

window.addEventListener('resize', () => {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
});

// Anime.js Animations
document.addEventListener('DOMContentLoaded', () => {
    // Hero title animation
    anime({
        targets: '.hero-title-new',
        opacity: [0, 1],
        translateY: [50, 0],
        duration: 1200,
        easing: 'easeOutExpo'
    });

    // Hero tagline animation
    anime({
        targets: '.hero-tagline',
        opacity: [0, 1],
        translateY: [30, 0],
        duration: 1000,
        delay: 300,
        easing: 'easeOutExpo'
    });

    // Hero description animation
    anime({
        targets: '.hero-description',
        opacity: [0, 1],
        translateY: [30, 0],
        duration: 1000,
        delay: 500,
        easing: 'easeOutExpo'
    });

    // Hero buttons animation
    anime({
        targets: '.hero-buttons-new .btn-new',
        opacity: [0, 1],
        translateY: [30, 0],
        duration: 1000,
        delay: anime.stagger(150, {start: 700}),
        easing: 'easeOutExpo'
    });

    // Hero icon animation
    anime({
        targets: '.hero-icon',
        opacity: [0, 1],
        scale: [0.5, 1],
        rotate: [180, 0],
        duration: 1500,
        easing: 'easeOutElastic(1, .8)'
    });

    // Stats counter animation
    const observerOptions = {
        threshold: 0.5
    };

    const statsObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                const statNumbers = entry.target.querySelectorAll('.stat-number');
                statNumbers.forEach(stat => {
                    const target = parseInt(stat.getAttribute('data-target'));
                    anime({
                        targets: stat,
                        innerHTML: [0, target],
                        duration: 2000,
                        round: 1,
                        easing: 'easeOutExpo'
                    });
                });
                statsObserver.unobserve(entry.target);
            }
        });
    }, observerOptions);

    const statsBar = document.querySelector('.stats-bar');
    if (statsBar) {
        statsObserver.observe(statsBar);
    }

    // Feature cards animation
    const featureObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                anime({
                    targets: entry.target,
                    opacity: [0, 1],
                    translateY: [50, 0],
                    duration: 800,
                    easing: 'easeOutExpo'
                });
                featureObserver.unobserve(entry.target);
            }
        });
    }, observerOptions);

    document.querySelectorAll('.feature-card-new').forEach(card => {
        featureObserver.observe(card);
    });

    // Doc cards animation
    const docObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                anime({
                    targets: entry.target,
                    opacity: [0, 1],
                    translateY: [30, 0],
                    duration: 600,
                    easing: 'easeOutExpo'
                });
                docObserver.unobserve(entry.target);
            }
        });
    }, observerOptions);

    document.querySelectorAll('.doc-card-new').forEach(card => {
        docObserver.observe(card);
    });
});

// Showcase tabs functionality
const showcaseTabs = document.querySelectorAll('.showcase-tab');
const showcaseExamples = document.querySelectorAll('.showcase-example');

showcaseTabs.forEach(tab => {
    tab.addEventListener('click', () => {
        const exampleId = tab.getAttribute('data-example');
        
        // Remove active class from all tabs and examples
        showcaseTabs.forEach(t => t.classList.remove('active'));
        showcaseExamples.forEach(e => e.classList.remove('active'));
        
        // Add active class to clicked tab and corresponding example
        tab.classList.add('active');
        document.getElementById(exampleId).classList.add('active');

        // Animate the code window
        anime({
            targets: `#${exampleId} .code-window-new`,
            opacity: [0, 1],
            translateX: [50, 0],
            duration: 500,
            easing: 'easeOutExpo'
        });
    });
});

// Copy install command
function copyInstallCommand() {
    const command = 'curl -fsSL https://raw.githubusercontent.com/velo4705/sapphire-lang/main/install.sh | bash';
    navigator.clipboard.writeText(command).then(() => {
        const btn = document.querySelector('.copy-btn');
        const originalText = btn.textContent;
        btn.textContent = 'Copied!';
        setTimeout(() => {
            btn.textContent = originalText;
        }, 2000);
    });
}

// Smooth scroll for navigation links
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
    const navbar = document.querySelector('.navbar');
    if (window.scrollY > 50) {
        navbar.style.background = 'rgba(10, 14, 26, 0.95)';
    } else {
        navbar.style.background = 'rgba(10, 14, 26, 0.8)';
    }
});
