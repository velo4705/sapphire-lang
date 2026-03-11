# Deploying Sapphire Website to GitHub Pages

## Quick Setup

### 1. Enable GitHub Pages

1. Go to your repository on GitHub
2. Click **Settings** → **Pages**
3. Under **Source**, select:
   - Branch: `main`
   - Folder: `/docs-site`
4. Click **Save**

### 2. Wait for Deployment

GitHub will automatically build and deploy your site. This usually takes 1-2 minutes.

### 3. Access Your Site

Your site will be available at:
```
https://velo4705.github.io/sapphire-lang
```

## Custom Domain (Optional)

### 1. Add CNAME File

Create `docs-site/CNAME` with your domain:

```
sapphire-lang.org
```

### 2. Configure DNS

Add these DNS records at your domain registrar:

**For apex domain (sapphire-lang.org):**
```
A     185.199.108.153
A     185.199.109.153
A     185.199.110.153
A     185.199.111.153
```

**For www subdomain:**
```
CNAME www.sapphire-lang.org velo4705.github.io
```

### 3. Enable HTTPS

In GitHub Pages settings:
- Check **Enforce HTTPS**

## Updating the Website

### Method 1: Direct Edit on GitHub

1. Navigate to `docs-site/` folder
2. Click on file to edit
3. Click pencil icon to edit
4. Commit changes
5. Site updates automatically

### Method 2: Local Development

```bash
# 1. Make changes locally
cd docs-site
# Edit files...

# 2. Test locally
python3 -m http.server 8000
# Visit http://localhost:8000

# 3. Commit and push
git add .
git commit -m "Update website"
git push origin main

# 4. Wait for deployment (1-2 minutes)
```

## Troubleshooting

### Site Not Loading

1. Check GitHub Actions tab for build errors
2. Verify GitHub Pages is enabled in Settings
3. Clear browser cache
4. Wait a few minutes for DNS propagation

### Styles Not Loading

1. Check that `style.css` is in `docs-site/` folder
2. Verify file paths in `index.html` are relative
3. Check browser console for 404 errors

### Custom Domain Not Working

1. Verify CNAME file exists in `docs-site/`
2. Check DNS records are correct
3. Wait 24-48 hours for DNS propagation
4. Ensure HTTPS is enforced in settings

## Monitoring

### Check Build Status

Visit: `https://github.com/velo4705/sapphire-lang/actions`

### View Deployment History

Settings → Pages → View deployments

### Analytics (Optional)

Add Google Analytics to `index.html`:

```html
<!-- Google Analytics -->
<script async src="https://www.googletagmanager.com/gtag/js?id=GA_MEASUREMENT_ID"></script>
<script>
  window.dataLayer = window.dataLayer || [];
  function gtag(){dataLayer.push(arguments);}
  gtag('js', new Date());
  gtag('config', 'GA_MEASUREMENT_ID');
</script>
```

## Performance Optimization

### 1. Minify CSS

```bash
# Install cssnano
npm install -g cssnano-cli

# Minify
cssnano docs-site/style.css docs-site/style.min.css
```

### 2. Minify JavaScript

```bash
# Install terser
npm install -g terser

# Minify
terser docs-site/script.js -o docs-site/script.min.js
```

### 3. Optimize Images

- Use WebP format
- Compress with tools like TinyPNG
- Use appropriate dimensions

### 4. Enable Caching

GitHub Pages automatically sets cache headers.

## Security

### HTTPS

Always enforce HTTPS in GitHub Pages settings.

### Content Security Policy

Add to `index.html` `<head>`:

```html
<meta http-equiv="Content-Security-Policy" 
      content="default-src 'self'; 
               style-src 'self' 'unsafe-inline' fonts.googleapis.com; 
               font-src fonts.gstatic.com;">
```

## Backup

### Download Site

```bash
wget -r -p -k https://velo4705.github.io/sapphire-lang
```

### Version Control

All files are in Git - your backup is the repository itself.

## Support

- **GitHub Pages Docs**: https://docs.github.com/pages
- **Issues**: https://github.com/velo4705/sapphire-lang/issues

---

Made with 💎 by the Sapphire community