// .vitepress/theme/index.ts
// Custom theme that registers the Flux language grammar with Shiki
import DefaultTheme from 'vitepress/theme'
import type { Theme } from 'vitepress'
import './custom.css'

export default {
  extends: DefaultTheme,
} satisfies Theme
