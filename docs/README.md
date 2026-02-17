# RFHUtil Documentation

This folder contains detailed technical documentation for the mq-rfhutil project.

## 📚 Documentation Index

### Architecture & Analysis
- **[Architecture Analysis](ARCHITECTURE_ANALYSIS.md)** - Comprehensive analysis of the codebase structure, patterns, and design
- **[Test Environment Info](TEST_ENVIRONMENT_INFO.md)** - Information about the test environment and setup

### Build & Configuration
- **[Build Configuration](BUILD_CONFIG.md)** - MSBuild paths, build commands, and project structure

### IBM MQ Technical Details
- **[MQ HeartBeat Negotiation](MQ_HEARTBEAT_NEGOTIATION.md)** - How MQ HeartBeat negotiation works
- **[MQ HeartBeat vs KeepAlive](MQ_HEARTBEAT_VS_KEEPALIVE.md)** - Comparison and use cases
- **[MQ KeepAlive Detailed](MQ_KEEPALIVE_DETAILED.md)** - Deep dive into TCP KeepAlive configuration

### Implementation Plans

#### Completed ✅
- **[P0 Implementation Plan](P0_IMPLEMENTATION_PLAN.md)** - Overall plan for P0 priorities
- **[P0.2 & P0.3 Implementation Guide](P0_2_AND_P0_3_IMPLEMENTATION_GUIDE.md)** - Detailed guide for auto-reconnect and UI tab

#### In Progress 🚧
- **[P1.2 Dark Mode Implementation](P1_2_DARK_MODE_IMPLEMENTATION.md)** - Complete plan for dark mode support

## 🎯 Quick Links

### For Developers
- Start with [Architecture Analysis](ARCHITECTURE_ANALYSIS.md) to understand the codebase
- Check [Build Configuration](BUILD_CONFIG.md) for build instructions
- Review implementation plans before starting new features

### For MQ Experts
- [MQ HeartBeat vs KeepAlive](MQ_HEARTBEAT_VS_KEEPALIVE.md) - Understanding the differences
- [MQ KeepAlive Detailed](MQ_KEEPALIVE_DETAILED.md) - Advanced configuration

### For Project Planning
- See main [Modernization Roadmap](../MODERNIZATION_ROADMAP.md) for overall progress
- Check individual implementation plans for detailed task breakdowns

## 📝 Document Status

| Document | Status | Last Updated |
|----------|--------|--------------|
| Architecture Analysis | ✅ Complete | Feb 2026 |
| Build Configuration | ✅ Complete | Feb 14, 2026 |
| MQ HeartBeat Negotiation | ✅ Complete | Feb 2026 |
| MQ HeartBeat vs KeepAlive | ✅ Complete | Feb 2026 |
| MQ KeepAlive Detailed | ✅ Complete | Feb 2026 |
| P0 Implementation Plan | ✅ Complete | Feb 2026 |
| P0.2 & P0.3 Guide | ✅ Complete | Feb 2026 |
| P1.2 Dark Mode Plan | 🚧 In Progress | Feb 14, 2026 |
| Test Environment Info | ✅ Complete | Feb 2026 |

## 🔄 Contributing to Documentation

When adding new documentation:

1. **Place it in this folder** - Keep root directory clean
2. **Update this README** - Add your document to the index
3. **Use clear naming** - Follow pattern: `CATEGORY_TOPIC.md`
4. **Link from roadmap** - Update main roadmap if it's an implementation plan
5. **Keep it current** - Update "Last Updated" dates

## 📋 Documentation Standards

- Use Markdown format (`.md`)
- Include table of contents for long documents
- Add code examples where applicable
- Use clear headings and sections
- Include diagrams when helpful (Mermaid or ASCII art)
- Keep technical accuracy high
- Update when implementation changes

## 🏗️ Project Structure

```
mq-rfhutil/
├── README.md                      # Main project readme
├── MODERNIZATION_ROADMAP.md       # Overall modernization plan
├── CHANGELOG.md                   # Version history
├── docs/                          # ← You are here
│   ├── README.md                  # This file
│   ├── ARCHITECTURE_ANALYSIS.md
│   ├── BUILD_CONFIG.md
│   ├── MQ_*.md                    # MQ technical docs
│   ├── P0_*.md                    # P0 implementation docs
│   └── P1_*.md                    # P1 implementation docs
├── RFHUtil/                       # Main application source
├── Client/                        # Client version source
└── mqperf/                        # Performance testing tools
```

## 📞 Questions?

For questions about:
- **Architecture** - See [Architecture Analysis](ARCHITECTURE_ANALYSIS.md)
- **Building** - See [Build Configuration](BUILD_CONFIG.md)
- **MQ Configuration** - See MQ technical docs
- **Implementation** - See relevant implementation plan

---

**Last Updated:** February 14, 2026  
**Maintained By:** Development Team