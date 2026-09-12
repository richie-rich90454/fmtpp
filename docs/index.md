---
layout: home
hero:
  name: fmtpp
  text: Format++ — one formatter, twenty-six languages
  tagline: A professional-grade, leak-free C++ code formatter. One static binary, zero runtime dependencies.
  actions:
    - theme: brand
      text: Getting started
      link: /guide/getting-started
    - theme: alt
      text: CLI reference
      link: /guide/cli
features:
  - title: 26 languages
    details: C-like braces, Python/Ruby indentation, HTML/Vue/JSON — detected from the file extension.
  - title: Fast by design
    details: Bump-allocator AST, zero-copy source buffers, direct output, multi-file thread pool.
  - title: Leak-free
    details: RAII wrappers for every resource, pool-owned AST nodes, valgrind-clean target.
  - title: Configurable
    details: .fmtpp.toml config, learn-from-example style detection, interactive TUI configurator.
  - title: Verified output
    details: Every style example in these docs is the exact binary output — see the style reference.
    link: /guide/style-reference
    linkText: Browse examples
  - title: Tested to hold
    details: 38 suites, 217 real files, byte-identical double formats — see how testing works.
    link: /guide/testing
    linkText: Read the method
---
