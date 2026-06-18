// ─── Background: crystalline gem shards ─────────────────────────────────────
function initHeroCanvas() {
    const canvas = document.getElementById('hero-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    function resize() {
        canvas.width  = canvas.offsetWidth;
        canvas.height = canvas.offsetHeight;
    }
    resize();
    window.addEventListener('resize', resize, { passive: true });

    // Shard definition — a polygon with position, velocity, rotation
    const COLORS = [
        'rgba(26,86,219,',    // vivid royal blue
        'rgba(77,159,255,',   // electric sapphire
        'rgba(15,45,138,',    // deep saturated navy
        'rgba(45,110,240,',   // bright mid blue
    ];

    function randomShard() {
        const sides = Math.random() < 0.5 ? 3 : 4; // triangles & quads
        const size  = 18 + Math.random() * 52;
        return {
            x:      Math.random() * canvas.width,
            y:      Math.random() * canvas.height,
            vx:     (Math.random() - 0.5) * 0.25,
            vy:     (Math.random() - 0.5) * 0.18,
            angle:  Math.random() * Math.PI * 2,
            vAngle: (Math.random() - 0.5) * 0.004,
            size,
            sides,
            color:  COLORS[Math.floor(Math.random() * COLORS.length)],
            alpha:  0.04 + Math.random() * 0.10,
        };
    }

    const COUNT = 28;
    const shards = Array.from({ length: COUNT }, randomShard);

    function drawShard(s) {
        ctx.save();
        ctx.translate(s.x, s.y);
        ctx.rotate(s.angle);
        ctx.beginPath();
        for (let i = 0; i < s.sides; i++) {
            const a = (i / s.sides) * Math.PI * 2 - Math.PI / 2;
            const r = i % 2 === 0 ? s.size : s.size * 0.6; // irregular gem shape
            const x = Math.cos(a) * r;
            const y = Math.sin(a) * r;
            i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
        }
        ctx.closePath();
        ctx.strokeStyle = s.color + (s.alpha * 1.8) + ')';
        ctx.lineWidth = 1;
        ctx.stroke();
        ctx.fillStyle = s.color + s.alpha + ')';
        ctx.fill();
        ctx.restore();
    }

    // Soft connection lines between nearby shards
    function drawConnections() {
        const DIST = 180;
        for (let i = 0; i < shards.length; i++) {
            for (let j = i + 1; j < shards.length; j++) {
                const dx = shards[i].x - shards[j].x;
                const dy = shards[i].y - shards[j].y;
                const d  = Math.sqrt(dx * dx + dy * dy);
                if (d < DIST) {
                    const alpha = (1 - d / DIST) * 0.06;
                    ctx.beginPath();
                    ctx.moveTo(shards[i].x, shards[i].y);
                    ctx.lineTo(shards[j].x, shards[j].y);
                    ctx.strokeStyle = `rgba(77,159,255,${alpha})`;
                    ctx.lineWidth = 0.5;
                    ctx.stroke();
                }
            }
        }
    }

    function tick() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        drawConnections();

        shards.forEach(s => {
            s.x     += s.vx;
            s.y     += s.vy;
            s.angle += s.vAngle;

            // Wrap around edges
            if (s.x < -s.size)              s.x = canvas.width  + s.size;
            if (s.x > canvas.width  + s.size) s.x = -s.size;
            if (s.y < -s.size)              s.y = canvas.height + s.size;
            if (s.y > canvas.height + s.size) s.y = -s.size;

            drawShard(s);
        });

        requestAnimationFrame(tick);
    }

    tick();
}

// ─── Code snippets to cycle through in the hero preview ─────────────────────
const SNIPPETS = [
    {
        filename: 'hello.spp',
        code: [
            { text: 'fn ', cls: 'k' },
            { text: 'main', cls: 'f' },
            { text: '() {\n' },
            { text: '    ' },
            { text: 'println', cls: 'f' },
            { text: '(' },
            { text: '"Hello, Sapphire! 💎"', cls: 's' },
            { text: ')\n}' },
        ]
    },
    {
        filename: 'fibonacci.spp',
        code: [
            { text: '@cache\n', cls: 'attr' },
            { text: 'fn ', cls: 'k' },
            { text: 'fib', cls: 'f' },
            { text: '(n: ' },
            { text: 'int', cls: 't' },
            { text: ') -> ' },
            { text: 'int', cls: 't' },
            { text: ' {\n' },
            { text: '    ' },
            { text: 'if ', cls: 'k' },
            { text: 'n <= ' },
            { text: '1', cls: 'n' },
            { text: ' { ' },
            { text: 'return ', cls: 'k' },
            { text: 'n', cls: 'n' },
            { text: ' }\n' },
            { text: '    ' },
            { text: 'return ', cls: 'k' },
            { text: 'fib', cls: 'f' },
            { text: '(n-' },
            { text: '1', cls: 'n' },
            { text: ') + ' },
            { text: 'fib', cls: 'f' },
            { text: '(n-' },
            { text: '2', cls: 'n' },
            { text: ')\n}' },
        ]
    },
    {
        filename: 'pattern.spp',
        code: [
            { text: 'fn ', cls: 'k' },
            { text: 'classify', cls: 'f' },
            { text: '(n: ' },
            { text: 'int', cls: 't' },
            { text: ') -> ' },
            { text: 'str', cls: 't' },
            { text: ' {\n' },
            { text: '    ' },
            { text: 'return ', cls: 'k' },
            { text: 'match ', cls: 'k' },
            { text: 'n {\n' },
            { text: '        ' },
            { text: '0', cls: 'n' },
            { text: '     => ' },
            { text: '"zero"', cls: 's' },
            { text: '\n' },
            { text: '        ' },
            { text: '1', cls: 'n' },
            { text: '     => ' },
            { text: '"one"', cls: 's' },
            { text: '\n' },
            { text: '        ' },
            { text: '2', cls: 'n' },
            { text: '..10 => ' },
            { text: '"small"', cls: 's' },
            { text: '\n' },
            { text: '        _ => ' },
            { text: '"large"', cls: 's' },
            { text: '\n    }\n}' },
        ]
    },
    {
        filename: 'loops.spp',
        code: [
            { text: 'fn ', cls: 'k' },
            { text: 'main', cls: 'f' },
            { text: '() {\n' },
            { text: '    ' },
            { text: 'let ', cls: 'k' },
            { text: 'nums = [' },
            { text: '1', cls: 'n' },
            { text: ', ' },
            { text: '2', cls: 'n' },
            { text: ', ' },
            { text: '3', cls: 'n' },
            { text: ', ' },
            { text: '4', cls: 'n' },
            { text: ', ' },
            { text: '5', cls: 'n' },
            { text: ']\n' },
            { text: '    ' },
            { text: 'for ', cls: 'k' },
            { text: 'n ' },
            { text: 'in ', cls: 'k' },
            { text: 'nums {\n' },
            { text: '        ' },
            { text: 'println', cls: 'f' },
            { text: '(n * ' },
            { text: '2', cls: 'n' },
            { text: ')\n    }\n}' },
        ]
    },
];

// ─── Typewriter engine ───────────────────────────────────────────────────────
const COLOR_MAP = {
    k:    '#818cf8',   // keywords  — indigo
    f:    '#60a5fa',   // functions — blue
    t:    '#38bdf8',   // types     — sky
    n:    '#a78bfa',   // numbers   — violet
    s:    '#86efac',   // strings   — green
    attr: '#f9a8d4',   // decorators — pink
};

function tokenToHtml(token) {
    const escaped = token.text
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;');
    if (token.cls && COLOR_MAP[token.cls]) {
        return `<span style="color:${COLOR_MAP[token.cls]}">${escaped}</span>`;
    }
    return `<span style="color:#e2e8f0">${escaped}</span>`;
}

class Typewriter {
    constructor(el, filenameEl) {
        this.el = el;
        this.filenameEl = filenameEl;
        this.snippetIndex = 0;
        this.running = false;
    }

    async start() {
        this.running = true;
        while (this.running) {
            const snippet = SNIPPETS[this.snippetIndex % SNIPPETS.length];
            this.filenameEl.textContent = snippet.filename;
            await this._type(snippet.code);
            await this._pause(2800);
            await this._erase();
            await this._pause(400);
            this.snippetIndex++;
        }
    }

    stop() { this.running = false; }

    async _type(tokens) {
        this.el.innerHTML = '';
        let html = '';
        const cursor = '<span class="tw-cursor">▋</span>';

        for (const token of tokens) {
            const chars = [...token.text]; // unicode-safe split
            for (const ch of chars) {
                if (!this.running) return;
                const partial = { ...token, text: ch };
                html += tokenToHtml(partial);
                this.el.innerHTML = html + cursor;
                await this._sleep(ch === '\n' ? 60 : 38 + Math.random() * 30);
            }
        }
        // leave cursor blinking after done
        this.el.innerHTML = html + cursor;
    }

    async _erase() {
        // Fade out the whole block instead of deleting char-by-char
        this.el.style.transition = 'opacity 0.35s';
        this.el.style.opacity = '0';
        await this._sleep(380);
        this.el.innerHTML = '';
        this.el.style.opacity = '1';
        this.el.style.transition = '';
    }

    _pause(ms) { return this._sleep(ms); }
    _sleep(ms) { return new Promise(r => setTimeout(r, ms)); }
}

// ─── Hero entrance animations ────────────────────────────────────────────────
function heroEntrance() {
    const items = [
        { el: '.hero-title',       delay: 0,   y: 30 },
        { el: '.hero-description', delay: 120, y: 20 },
        { el: '.hero-actions',     delay: 240, y: 20 },
        { el: '.hero-stats',       delay: 380, y: 20 },
        { el: '.hero-right',       delay: 180, y: 0, x: 40 },
    ];

    items.forEach(({ el, delay, y = 0, x = 0 }) => {
        const node = document.querySelector(el);
        if (!node) return;
        node.style.opacity = '0';
        node.style.transform = `translate(${x}px, ${y}px)`;
        node.style.transition = `opacity 0.7s ease ${delay}ms, transform 0.7s ease ${delay}ms`;
        // Trigger
        requestAnimationFrame(() => requestAnimationFrame(() => {
            node.style.opacity = '1';
            node.style.transform = 'translate(0, 0)';
        }));
    });
}

// ─── Scroll-triggered fade-ins ───────────────────────────────────────────────
function initScrollAnimations() {
    const targets = document.querySelectorAll('.card, .code-card, .section-title');
    targets.forEach(el => {
        el.style.opacity = '0';
        el.style.transform = 'translateY(32px)';
        el.style.transition = 'opacity 0.6s ease, transform 0.6s ease';
    });

    const observer = new IntersectionObserver((entries) => {
        entries.forEach((entry, i) => {
            if (entry.isIntersecting) {
                const el = entry.target;
                // stagger siblings
                const siblings = [...el.parentElement.children].filter(c =>
                    c.classList.contains('card') || c.classList.contains('code-card')
                );
                const idx = siblings.indexOf(el);
                const delay = idx >= 0 ? idx * 80 : 0;
                setTimeout(() => {
                    el.style.opacity = '1';
                    el.style.transform = 'translateY(0)';
                }, delay);
                observer.unobserve(el);
            }
        });
    }, { threshold: 0.15, rootMargin: '0px 0px -60px 0px' });

    targets.forEach(el => observer.observe(el));
}

// ─── Smooth scroll ───────────────────────────────────────────────────────────
function initSmoothScroll() {
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            const target = document.querySelector(this.getAttribute('href'));
            if (target) {
                const offset = document.querySelector('.nav').offsetHeight;
                window.scrollTo({
                    top: target.getBoundingClientRect().top + window.pageYOffset - offset,
                    behavior: 'smooth'
                });
            }
        });
    });
}

// ─── Nav scroll effect + back to top ────────────────────────────────────────
function initNavScroll() {
    const nav = document.querySelector('.nav');
    const btn = document.getElementById('backToTop');

    window.addEventListener('scroll', () => {
        const y = window.scrollY;
        nav.style.background    = y > 40 ? 'rgba(2, 5, 16, 0.94)' : '';
        nav.style.backdropFilter = y > 40 ? 'blur(12px)' : '';
        btn && btn.classList.toggle('visible', y > 400);
    }, { passive: true });

    btn && btn.addEventListener('click', () => {
        window.scrollTo({ top: 0, behavior: 'smooth' });
    });
}

// ─── Boot ────────────────────────────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
    initHeroCanvas();
    heroEntrance();
    initScrollAnimations();
    initSmoothScroll();
    initNavScroll();

    // Hamburger menu toggle
    const hamburger = document.getElementById('hamburger');
    const navCenter = document.getElementById('navCenter');
    if (hamburger && navCenter) {
        hamburger.addEventListener('click', () => {
            hamburger.classList.toggle('active');
            navCenter.classList.toggle('open');
        });
        // Close on link click
        navCenter.querySelectorAll('a').forEach(link => {
            link.addEventListener('click', () => {
                hamburger.classList.remove('active');
                navCenter.classList.remove('open');
            });
        });
    }

    // Lazy load sections below the fold
    const lazySections = document.querySelectorAll('.section-delayed');
    if (lazySections.length && 'IntersectionObserver' in window) {
        const observer = new IntersectionObserver((entries) => {
            entries.forEach(entry => {
                if (entry.isIntersecting) {
                    entry.target.classList.add('visible');
                    observer.unobserve(entry.target);
                }
            });
        }, { rootMargin: '200px' });
        lazySections.forEach(s => observer.observe(s));
    }

    const codeEl = document.querySelector('.code-preview-body code');
    const fileEl = document.querySelector('.code-preview-filename');
    if (codeEl && fileEl) {
        const tw = new Typewriter(codeEl, fileEl);
        // Small delay so entrance animation plays first
        setTimeout(() => tw.start(), 600);
    }
});
