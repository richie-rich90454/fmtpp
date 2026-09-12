import { defineConfig } from 'vitepress'

export default defineConfig({
  title: 'fmtpp',
  description: 'Format++ — a professional multi-language code formatter',
  base: '/fmtpp/',
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
          { text: 'CLI reference', link: '/guide/cli' },
          { text: 'Configuration', link: '/guide/configuration' },
          { text: 'Languages', link: '/guide/languages' }
        ]
      },
      {
        text: 'Development',
        items: [
          { text: 'Building', link: '/guide/building' },
          { text: 'Testing', link: '/guide/testing' },
          { text: 'Architecture', link: '/guide/architecture' }
        ]
      }
    ],
    socialLinks: [
      { icon: 'github', link: 'https://github.com/fmtpp/fmtpp' }
    ],
    search: { provider: 'local' }
  }
})
