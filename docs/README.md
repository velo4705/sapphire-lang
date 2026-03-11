# Sapphire Website

Official website for the Sapphire programming language, hosted on GitHub Pages.

## 🌐 Live Site

Visit: [https://velo4705.github.io/sapphire-lang](https://velo4705.github.io/sapphire-lang)

## 📁 Structure

```
docs-site/
├── index.html      # Main landing page
├── style.css       # Styles and theme
├── script.js       # Interactive functionality
└── README.md       # This file
```

## 🎨 Features

- **Modern Design** - Clean, professional interface with dark theme
- **Responsive** - Works on desktop, tablet, and mobile
- **Interactive** - Tabbed code examples and smooth scrolling
- **Fast** - Static site with no dependencies
- **Accessible** - Semantic HTML and proper contrast ratios

## 🚀 Local Development

To preview the website locally:

```bash
# Option 1: Python
cd docs-site
python3 -m http.server 8000

# Option 2: Node.js
npx serve docs-site

# Option 3: PHP
php -S localhost:8000 -t docs-site
```

Then open http://localhost:8000 in your browser.

## 📝 Updating Content

### Update Version Number

Edit `index.html` and change the version badge:

```html
<span class="version">beta.3</span>
```

### Add New Code Examples

Add a new tab button and content in `index.html`:

```html
<!-- Add button -->
<button class="tab-btn" data-tab="newexample">New Example</button>

<!-- Add content -->
<div class="tab-content" id="newexample">
    <div class="code-window">
        <!-- Your code here -->
    </div>
</div>
```

### Update Features

Edit the features grid in `index.html`:

```html
<div class="feature-card">
    <div class="feature-icon">🎯</div>
    <h3>Feature Title</h3>
    <p>Feature description</p>
</div>
```

### Modify Styles

Edit `style.css` to change colors, fonts, or layout:

```css
:root {
    --primary: #3b82f6;  /* Change primary color */
    --bg-dark: #0f172a;  /* Change background */
}
```

## 🎨 Color Palette

- **Primary**: `#3b82f6` (Blue)
- **Secondary**: `#8b5cf6` (Purple)
- **Accent**: `#06b6d4` (Cyan)
- **Background**: `#020617` (Dark)
- **Text**: `#f1f5f9` (Light)

## 📱 Responsive Breakpoints

- **Desktop**: > 968px
- **Tablet**: 640px - 968px
- **Mobile**: < 640px

## 🔧 Technologies

- **HTML5** - Semantic markup
- **CSS3** - Modern styling with Grid and Flexbox
- **Vanilla JavaScript** - No frameworks needed
- **Google Fonts** - Inter and JetBrains Mono

## 📄 License

MIT License - Same as Sapphire language

## 🤝 Contributing

To contribute to the website:

1. Edit files in `docs-site/`
2. Test locally
3. Submit a pull request

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/velo4705/sapphire-lang/issues)
- **Discussions**: [GitHub Discussions](https://github.com/velo4705/sapphire-lang/discussions)

---

Made with 💎 by the Sapphire community