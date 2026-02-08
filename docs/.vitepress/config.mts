import { defineConfig } from 'vitepress'
import { readFileSync } from 'node:fs'
import { resolve, dirname } from 'node:path'
import { fileURLToPath } from 'node:url'

const __dirname = dirname(fileURLToPath(import.meta.url))

// Load Flux TextMate grammar for Shiki syntax highlighting
const fluxGrammar = JSON.parse(
  readFileSync(resolve(__dirname, 'flux.tmLanguage.json'), 'utf-8')
)

export default defineConfig({
  title: 'Flux Language',
  description: 'Documentation for the Flux programming language',
  base: '/',

  head: [
    ['link', { rel: 'icon', type: 'image/svg+xml', href: '/logo.svg' }],
  ],

  themeConfig: {
    logo: '/logo.svg',
    siteTitle: 'Flux',

    nav: [
      { text: 'Guide', link: '/guide/' },
      { text: 'Reference', link: '/reference/' },
      { text: 'Examples', link: '/examples/' },
      { text: 'GitHub', link: 'https://github.com/flux-lang/flux' },
    ],

    sidebar: {
      '/guide/': [
        {
          text: 'Getting Started',
          items: [
            { text: 'Introduction', link: '/guide/' },
            { text: 'Installation', link: '/guide/installation' },
            { text: 'Hello World', link: '/guide/hello-world' },
          ],
        },
        {
          text: 'Language Basics',
          items: [
            { text: 'Variables & Types', link: '/guide/variables' },
            { text: 'Functions', link: '/guide/functions' },
            { text: 'Control Flow', link: '/guide/control-flow' },
            { text: 'Structs & Enums', link: '/guide/structs-enums' },
          ],
        },
        {
          text: 'Advanced',
          items: [
            { text: 'Ownership & Borrowing', link: '/guide/ownership' },
            { text: 'Traits & Generics', link: '/guide/traits' },
            { text: 'Pattern Matching', link: '/guide/pattern-matching' },
            { text: 'Async & Concurrency', link: '/guide/async' },
            { text: 'Error Handling', link: '/guide/error-handling' },
          ],
        },
      ],
      '/reference/': [
        {
          text: 'Reference',
          items: [
            { text: 'Overview', link: '/reference/' },
            { text: 'Keywords', link: '/reference/keywords' },
            { text: 'Operators', link: '/reference/operators' },
            { text: 'Built-in Types', link: '/reference/types' },
          ],
        },
      ],
      '/examples/': [
        {
          text: 'Examples',
          items: [
            { text: 'Overview', link: '/examples/' },
            { text: 'Hello World', link: '/examples/hello-world' },
            { text: 'Ownership', link: '/examples/ownership' },
            { text: 'Shapes (Traits)', link: '/examples/shapes' },
          ],
        },
      ],
    },

    socialLinks: [
      { icon: 'github', link: 'https://github.com/flux-lang/flux' },
    ],

    footer: {
      message: 'Released under the MIT License.',
      copyright: 'Copyright © 2025 Flux Contributors',
    },

    search: {
      provider: 'local',
    },

    outline: {
      level: [2, 3],
    },
  },

  markdown: {
    theme: {
      light: 'github-light',
      dark: 'github-dark',
    },
    languages: [
      {
        ...fluxGrammar,
        name: 'flux',
        aliases: ['fl'],
      } as any,
    ],
  },
})
