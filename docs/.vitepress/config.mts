import { readFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'
import { withMermaid } from 'vitepress-plugin-mermaid'

// Browsers auto-request /favicon.ico at the server root before Vue
// hydration injects the data-URI icon link. With base '/fmtpp/' Vite
// answers that with a 404 page, so serve the file directly (dev only;
// prod HTML is pre-rendered with the icon link, no request happens).
const rootFavicon = () => ({
  name: 'fmtpp-root-favicon',
  configureServer(server) {
    server.middlewares.use('/favicon.ico', (_req, res) => {
      try {
        const file = join(dirname(fileURLToPath(import.meta.url)), '..', 'public', 'favicon.ico')
        const data = readFileSync(file)
        res.writeHead(200, { 'Content-Type': 'image/x-icon', 'Content-Length': data.length })
        res.end(data)
      }
      catch {
        res.writeHead(404)
        res.end()
      }
    })
  },
})

const config = withMermaid({
  title: 'fmtpp',
  description: 'Format++ — a professional multi-language code formatter',
  base: '/fmtpp/',
  head: [
    ['link', { rel: 'icon', type: 'image/svg+xml', href: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'%3E%3Crect width='100' height='100' rx='20' fill='%23c2410c'/%3E%3Ctext x='50' y='70' font-size='58' text-anchor='middle' fill='white' font-family='sans-serif' font-weight='bold'%3EF%3C/text%3E%3C/svg%3E" }]
  ],
  mermaid: {
    theme: 'base',
    themeVariables: {
      primaryColor: '#ffedd5',
      primaryBorderColor: '#c2410c',
      primaryTextColor: '#7c2d12',
      secondaryColor: '#fed7aa',
      tertiaryColor: '#fff7ed',
      lineColor: '#c2410c',
      fontSize: '16px',
    },
    flowchart: {
      htmlLabels: true,
      padding: 10,
      wrappingWidth: 240,
    },
    sequence: {
      width: 160,
      actorMargin: 60,
      messageMargin: 40,
      boxMargin: 12,
      boxTextMargin: 8,
    },
  },
  vite: {
    plugins: [rootFavicon()],
    optimizeDeps: {
      // Pre-bundle mermaid itself so esbuild resolves dayjs from
      // mermaid's own directory. Without this, Vite dev serves
      // mermaid's raw CJS dayjs import as native ESM and the page
      // crashes with "dayjs.min.js does not provide an export
      // named 'default'". (pnpm strict isolation; prod unaffected.)
      include: ['mermaid', 'dayjs'],
    },
    resolve: {
      alias: [
        // Fallback: serve dayjs ESM sources if anything still
        // imports a deep dayjs/*.js path at runtime.
        { find: /^dayjs\/(.*)\.js$/, replacement: 'dayjs/esm/$1' },
      ],
    },
  },
  themeConfig: {
    nav: [
      { text: 'Guide', link: '/guide/getting-started' },
      { text: 'Languages', link: '/guide/languages' },
      { text: 'Config', link: '/guide/configuration' }
    ],
    sidebar: [
      {
        text: 'Guide',
        items: [
          { text: 'Getting started', link: '/guide/getting-started' },
          { text: 'Installation', link: '/guide/installation' },
          { text: 'Usage', link: '/guide/usage' },
          { text: 'CLI reference', link: '/guide/cli' },
          { text: 'Configuration', link: '/guide/configuration' },
          { text: 'Languages', link: '/guide/languages' },
          { text: 'Style reference', link: '/guide/style-reference' },
          { text: 'Migrating', link: '/guide/migrating' },
          { text: 'Node API', link: '/guide/node-api' }
        ]
      },
      {
        text: 'Development',
        items: [
          { text: 'Building', link: '/guide/building' },
          { text: 'Testing', link: '/guide/testing' },
          { text: 'Benchmarking', link: '/guide/benchmarking' },
          { text: 'Architecture', link: '/guide/architecture' },
          { text: 'Contributing', link: '/guide/contributing' },
          { text: 'Troubleshooting', link: '/guide/troubleshooting' }
        ]
      }
    ],
    socialLinks: [
      { icon: 'github', link: 'https://github.com/fmtpp/fmtpp' }
    ],
    search: { provider: 'local' }
  }
})

// withMermaid appends its own transitive pre-bundle hints
// (sanitize-url, debug, cytoscape, ...). Under pnpm those are not
// resolvable from the docs root, so Vite prints "Failed to resolve
// dependency" warnings and skips them. Keep only what resolves.
config.vite ??= {}
config.vite.optimizeDeps ??= {}
config.vite.optimizeDeps.include = ['mermaid', 'dayjs']

export default config
