#include "fmtpp/TuiConfigurator.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

// Easter egg: the TUI configurator - where ASCII art meets configuration

// ANSI escape code helpers
#define ESC "\x1b"
#define CSI ESC "["

static const char* BOLD_ON  = CSI "1m";
static const char* BOLD_OFF = CSI "22m";
static const char* REVERSE_ON  = CSI "7m";
static const char* REVERSE_OFF = CSI "27m";
static const char* GREEN_FG  = CSI "32m";
static const char* CYAN_FG   = CSI "36m";
static const char* YELLOW_FG = CSI "33m";
static const char* WHITE_FG  = CSI "37m";
static const char* DIM_FG    = CSI "2m";
static const char* RESET_ALL  = CSI "0m";
static const char* CLEAR_SCREEN = CSI "2J" CSI "H";
static const char* HIDE_CURSOR  = CSI "?25l";
static const char* SHOW_CURSOR  = CSI "?25h";

// Easter egg: the color palette of a true craftsman

TuiConfigurator::TuiConfigurator()
    : m_selectedIndex(0)
    , m_terminalWidth(80)
    , m_terminalHeight(24)
    , m_running(false)
    , m_saved(false)
    , m_modified(false)
{
}

TuiConfigurator::~TuiConfigurator()
{
    // Easter egg: TUI configurator shutting down - back to the command line
}

bool TuiConfigurator::run(Config& config)
{
    m_config = config;
    m_selectedIndex = 0;
    m_running = true;
    m_saved = false;
    m_modified = false;

    // Detect terminal size
    m_terminalWidth = getTerminalWidth();
    m_terminalHeight = getTerminalHeight();

    if (m_terminalWidth < 60)
    {
        m_terminalWidth = 80;
    }
    if (m_terminalHeight < 20)
    {
        m_terminalHeight = 24;
    }

    buildMenu(m_config);

    // Hide cursor and clear screen
    printf("%s", HIDE_CURSOR);
    printf("%s", CLEAR_SCREEN);
    fflush(stdout);

    // Main event loop
    int key = 0;
    while (m_running)
    {
        render();
        key = readKey();
        if (key == 0)
        {
            continue;
        }
        m_running = (handleKey(key) == 0);
    }

    // Restore cursor and clear screen
    printf("%s", SHOW_CURSOR);
    printf("%s", CLEAR_SCREEN);
    fflush(stdout);

    if (m_saved)
    {
        applyToConfig();
        config = m_config;
    }

    return m_saved;
}

const Config& TuiConfigurator::getConfig() const
{
    return m_config;
}

void TuiConfigurator::buildMenu(const Config& config)
{
    m_menuItems.clear();

    // Section label
    {
        MenuItem item;
        item.label = "--- General Settings ---";
        item.value = "";
        item.type = MENU_LABEL;
        m_menuItems.push_back(item);
    }

    // printWidth
    {
        MenuItem item;
        item.label = "Print Width";
        char buf[32];
        sprintf(buf, "%d", config.printWidth);
        item.value = buf;
        item.type = MENU_INT;
        item.minValue = 40;
        item.maxValue = 200;
        m_menuItems.push_back(item);
    }

    // useTabs
    {
        MenuItem item;
        item.label = "Use Tabs";
        item.value = config.useTabs ? "true" : "false";
        item.type = MENU_BOOL;
        m_menuItems.push_back(item);
    }

    // tabWidth
    {
        MenuItem item;
        item.label = "Tab Width";
        char buf[32];
        sprintf(buf, "%d", config.tabWidth);
        item.value = buf;
        item.type = MENU_INT;
        item.minValue = 1;
        item.maxValue = 8;
        m_menuItems.push_back(item);
    }

    // Section label
    {
        MenuItem item;
        item.label = "--- Syntax Settings ---";
        item.value = "";
        item.type = MENU_LABEL;
        m_menuItems.push_back(item);
    }

    // semi
    {
        MenuItem item;
        item.label = "Semicolons";
        item.value = config.semi ? "true" : "false";
        item.type = MENU_BOOL;
        m_menuItems.push_back(item);
    }

    // singleQuote
    {
        MenuItem item;
        item.label = "Single Quotes";
        item.value = config.singleQuote ? "true" : "false";
        item.type = MENU_BOOL;
        m_menuItems.push_back(item);
    }

    // trailingComma
    {
        MenuItem item;
        item.label = "Trailing Comma";
        item.value = config.trailingComma;
        item.type = MENU_STRING;
        item.options.push_back("none");
        item.options.push_back("es5");
        item.options.push_back("all");
        item.optionIndex = 0;
        size_t j;
        for (j = 0; j < item.options.size(); j++)
        {
            if (item.options[j] == config.trailingComma)
            {
                item.optionIndex = static_cast<int>(j);
                break;
            }
        }
        m_menuItems.push_back(item);
    }

    // bracketSpacing
    {
        MenuItem item;
        item.label = "Bracket Spacing";
        item.value = config.bracketSpacing ? "true" : "false";
        item.type = MENU_BOOL;
        m_menuItems.push_back(item);
    }

    // arrowParens
    {
        MenuItem item;
        item.label = "Arrow Parens";
        item.value = config.arrowParens;
        item.type = MENU_STRING;
        item.options.push_back("always");
        item.options.push_back("avoid");
        item.optionIndex = 0;
        size_t j;
        for (j = 0; j < item.options.size(); j++)
        {
            if (item.options[j] == config.arrowParens)
            {
                item.optionIndex = static_cast<int>(j);
                break;
            }
        }
        m_menuItems.push_back(item);
    }

    // Section label
    {
        MenuItem item;
        item.label = "--- Formatting Style ---";
        item.value = "";
        item.type = MENU_LABEL;
        m_menuItems.push_back(item);
    }

    // endOfLine
    {
        MenuItem item;
        item.label = "End of Line";
        item.value = config.endOfLine;
        item.type = MENU_STRING;
        item.options.push_back("lf");
        item.options.push_back("crlf");
        item.options.push_back("auto");
        item.optionIndex = 0;
        size_t j;
        for (j = 0; j < item.options.size(); j++)
        {
            if (item.options[j] == config.endOfLine)
            {
                item.optionIndex = static_cast<int>(j);
                break;
            }
        }
        m_menuItems.push_back(item);
    }

    // braceStyle
    {
        MenuItem item;
        item.label = "Brace Style";
        item.value = config.braceStyle;
        item.type = MENU_STRING;
        item.options.push_back("same-line");
        item.options.push_back("next-line");
        item.optionIndex = 0;
        size_t j;
        for (j = 0; j < item.options.size(); j++)
        {
            if (item.options[j] == config.braceStyle)
            {
                item.optionIndex = static_cast<int>(j);
                break;
            }
        }
        m_menuItems.push_back(item);
    }

    // spaceBeforeBrace
    {
        MenuItem item;
        item.label = "Space Before Brace";
        item.value = config.spaceBeforeBrace ? "true" : "false";
        item.type = MENU_BOOL;
        m_menuItems.push_back(item);
    }

    // spaceAfterKeyword
    {
        MenuItem item;
        item.label = "Space After Keyword";
        item.value = config.spaceAfterKeyword ? "true" : "false";
        item.type = MENU_BOOL;
        m_menuItems.push_back(item);
    }

    // spaceAroundOperators
    {
        MenuItem item;
        item.label = "Space Around Ops";
        item.value = config.spaceAroundOperators ? "true" : "false";
        item.type = MENU_BOOL;
        m_menuItems.push_back(item);
    }

    // Section label
    {
        MenuItem item;
        item.label = "--- Actions ---";
        item.value = "";
        item.type = MENU_LABEL;
        m_menuItems.push_back(item);
    }

    // Save
    {
        MenuItem item;
        item.label = "[ Save Config ]";
        item.value = "";
        item.type = MENU_ACTION;
        m_menuItems.push_back(item);
    }

    // Exit without saving
    {
        MenuItem item;
        item.label = "[ Exit (Discard) ]";
        item.value = "";
        item.type = MENU_ACTION;
        m_menuItems.push_back(item);
    }

    // Skip past labels to first interactive item
    size_t i;
    for (i = 0; i < m_menuItems.size(); i++)
    {
        if (m_menuItems[i].type != MENU_LABEL)
        {
            m_selectedIndex = static_cast<int>(i);
            break;
        }
    }
}

void TuiConfigurator::render()
{
    printf("%s", CLEAR_SCREEN);

    // Title bar
    printf("%s", BOLD_ON);
    printf("%s", CYAN_FG);
    printf("  fmtpp TUI Configurator v0.1.0");
    printf("%s\n", RESET_ALL);

    // Separator
    printf("%s", DIM_FG);
    int w;
    for (w = 0; w < m_terminalWidth; w++)
    {
        printf("-");
    }
    printf("%s\n", RESET_ALL);

    renderMenu();

    // Separator
    printf("%s", DIM_FG);
    for (w = 0; w < m_terminalWidth; w++)
    {
        printf("-");
    }
    printf("%s\n", RESET_ALL);

    // Footer
    printf("%s", DIM_FG);
    printf("  [UP/DOWN] Navigate  [ENTER/Space] Edit  [S] Save  [Q] Quit");
    printf("%s\n", RESET_ALL);

    renderPreview();

    fflush(stdout);
}

void TuiConfigurator::renderMenu()
{
    size_t i;
    for (i = 0; i < m_menuItems.size(); i++)
    {
        const MenuItem& item = m_menuItems[i];

        if (item.type == MENU_LABEL)
        {
            printf("  %s%s%s\n", BOLD_ON, item.label.c_str(), RESET_ALL);
            continue;
        }

        // Highlight selected item
        if (static_cast<int>(i) == m_selectedIndex)
        {
            printf("  %s", REVERSE_ON);
        }

        // Print label
        printf("  %-24s  ", item.label.c_str());

        // Print value with type-specific formatting
        switch (item.type)
        {
        case MENU_BOOL:
            if (item.value == "true")
            {
                printf("%s[YES]%s", GREEN_FG, RESET_ALL);
            }
            else
            {
                printf("%s[NO]%s", DIM_FG, RESET_ALL);
            }
            break;

        case MENU_INT:
            printf("%s%s%s", YELLOW_FG, item.value.c_str(), RESET_ALL);
            break;

        case MENU_STRING:
            printf("%s<%s>%s", CYAN_FG, item.value.c_str(), RESET_ALL);
            break;

        case MENU_ACTION:
            printf("%s%s%s", BOLD_ON, item.label.c_str(), RESET_ALL);
            break;

        default:
            break;
        }

        if (static_cast<int>(i) == m_selectedIndex)
        {
            printf("%s", REVERSE_OFF);
        }

        printf("\n");
    }
}

void TuiConfigurator::renderPreview()
{
    printf("\n%s  Preview:%s\n", BOLD_ON, RESET_ALL);
    printf("  ");

    // Show a sample of what the formatted output would look like
    const char* brace = " {";
    if (m_config.braceStyle == "next-line")
    {
        brace = "\n  {";
    }

    const char* semiMarker = m_config.semi ? ";" : "";
    const char* quoteChar = m_config.singleQuote ? "'" : "\"";
    const char* spBeforeBrace = m_config.spaceBeforeBrace ? " " : "";
    const char* spAround = m_config.spaceAroundOperators ? " " : "";

    printf("function hello(world)%s%s\n", spBeforeBrace, brace);
    printf("  const greeting%s= %s%sHello%s%s%s\n",
        spAround, quoteChar, quoteChar, spAround, quoteChar, semiMarker);
    printf("  const name%s= %s%sworld%s%s%s\n",
        spAround, quoteChar, quoteChar, spAround, quoteChar, semiMarker);
    printf("  return greeting%s+%sname%s\n", spAround, spAround, semiMarker);

    if (m_config.braceStyle == "same-line")
    {
        printf("}\n");
    }
    else
    {
        printf("}\n");
    }

    // Show config summary
    printf("\n%s  Config Summary:%s\n", BOLD_ON, RESET_ALL);
    printf("  %sWidth: %d  Tabs: %s  Semi: %s%s\n",
        DIM_FG,
        m_config.printWidth,
        m_config.useTabs ? "yes" : "no",
        m_config.semi ? "yes" : "no",
        RESET_ALL);
    printf("  %sQuotes: %s  Brace: %s  EOL: %s%s\n",
        DIM_FG,
        m_config.singleQuote ? "single" : "double",
        m_config.braceStyle.c_str(),
        m_config.endOfLine.c_str(),
        RESET_ALL);
}

int TuiConfigurator::handleKey(int key)
{
    // Arrow keys send escape sequences
    if (key == 0x48 || key == 'k' || key == 'K')
    {
        // Up arrow or 'k'
        int newIndex = m_selectedIndex - 1;
        while (newIndex >= 0 && m_menuItems[newIndex].type == MENU_LABEL)
        {
            newIndex--;
        }
        if (newIndex >= 0)
        {
            m_selectedIndex = newIndex;
        }
    }
    else if (key == 0x50 || key == 'j' || key == 'J')
    {
        // Down arrow or 'j'
        int newIndex = m_selectedIndex + 1;
        while (static_cast<size_t>(newIndex) < m_menuItems.size() &&
               m_menuItems[newIndex].type == MENU_LABEL)
        {
            newIndex++;
        }
        if (static_cast<size_t>(newIndex) < m_menuItems.size())
        {
            m_selectedIndex = newIndex;
        }
    }
    else if (key == '\r' || key == '\n' || key == ' ')
    {
        // Enter or Space - edit current item
        if (m_selectedIndex >= 0 &&
            static_cast<size_t>(m_selectedIndex) < m_menuItems.size())
        {
            MenuItem& item = m_menuItems[m_selectedIndex];

            switch (item.type)
            {
            case MENU_BOOL:
                toggleBool(m_selectedIndex);
                break;

            case MENU_INT:
                readInteger(m_selectedIndex);
                break;

            case MENU_STRING:
                cycleOption(m_selectedIndex);
                break;

            case MENU_ACTION:
                if (item.label.find("Save") != std::string::npos)
                {
                    applyToConfig();
                    m_saved = saveConfig();
                    if (m_saved)
                    {
                        return 1; // Exit
                    }
                }
                else if (item.label.find("Exit") != std::string::npos ||
                         item.label.find("Discard") != std::string::npos)
                {
                    return 1; // Exit without saving
                }
                break;

            default:
                break;
            }
        }
    }
    else if (key == 's' || key == 'S')
    {
        // Save shortcut
        applyToConfig();
        m_saved = saveConfig();
        if (m_saved)
        {
            return 1; // Exit
        }
    }
    else if (key == 'q' || key == 'Q' || key == 0x1B)
    {
        // Q or Escape - quit
        return 1;
    }

    return 0; // Continue running
}

void TuiConfigurator::toggleBool(int index)
{
    if (index < 0 || static_cast<size_t>(index) >= m_menuItems.size())
    {
        return;
    }

    MenuItem& item = m_menuItems[index];
    if (item.value == "true")
    {
        item.value = "false";
    }
    else
    {
        item.value = "true";
    }
    m_modified = true;
}

void TuiConfigurator::cycleOption(int index)
{
    if (index < 0 || static_cast<size_t>(index) >= m_menuItems.size())
    {
        return;
    }

    MenuItem& item = m_menuItems[index];
    item.optionIndex = (item.optionIndex + 1) % static_cast<int>(item.options.size());
    item.value = item.options[item.optionIndex];
    m_modified = true;
}

void TuiConfigurator::readInteger(int index)
{
    if (index < 0 || static_cast<size_t>(index) >= m_menuItems.size())
    {
        return;
    }

    MenuItem& item = m_menuItems[index];

    // Show cursor for input
    printf("%s", SHOW_CURSOR);

    // Move to bottom of screen for input
    printf("\n  Enter new value for %s (%d-%d): ", item.label.c_str(), item.minValue, item.maxValue);
    fflush(stdout);

    char buf[32];
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        int val = atoi(buf);
        if (val >= item.minValue && val <= item.maxValue)
        {
            sprintf(buf, "%d", val);
            item.value = buf;
            m_modified = true;
        }
    }

    // Hide cursor again
    printf("%s", HIDE_CURSOR);
    fflush(stdout);
}

bool TuiConfigurator::saveConfig()
{
    FILE* file = fopen(".fmtpp.toml", "w");
    if (file == NULL)
    {
        // Easter egg: failed to save config - the file system is not cooperating
        return false;
    }

    fprintf(file, "# fmtpp configuration\n");
    fprintf(file, "# Generated by fmtpp TUI configurator\n");
    fprintf(file, "# Easter egg: hand-crafted formatting preferences\n\n");

    fprintf(file, "printWidth = %d\n", m_config.printWidth);
    fprintf(file, "useTabs = %s\n", m_config.useTabs ? "true" : "false");
    fprintf(file, "tabWidth = %d\n", m_config.tabWidth);
    fprintf(file, "semi = %s\n", m_config.semi ? "true" : "false");
    fprintf(file, "singleQuote = %s\n", m_config.singleQuote ? "true" : "false");
    fprintf(file, "trailingComma = \"%s\"\n", m_config.trailingComma.c_str());
    fprintf(file, "bracketSpacing = %s\n", m_config.bracketSpacing ? "true" : "false");
    fprintf(file, "arrowParens = \"%s\"\n", m_config.arrowParens.c_str());
    fprintf(file, "endOfLine = \"%s\"\n", m_config.endOfLine.c_str());
    fprintf(file, "braceStyle = \"%s\"\n", m_config.braceStyle.c_str());
    fprintf(file, "spaceBeforeBrace = %s\n", m_config.spaceBeforeBrace ? "true" : "false");
    fprintf(file, "spaceAfterKeyword = %s\n", m_config.spaceAfterKeyword ? "true" : "false");
    fprintf(file, "spaceAroundOperators = %s\n", m_config.spaceAroundOperators ? "true" : "false");

    fclose(file);
    return true;
}

void TuiConfigurator::applyToConfig()
{
    size_t i;
    for (i = 0; i < m_menuItems.size(); i++)
    {
        const MenuItem& item = m_menuItems[i];

        if (item.label == "Print Width")
        {
            m_config.printWidth = atoi(item.value.c_str());
        }
        else if (item.label == "Use Tabs")
        {
            m_config.useTabs = (item.value == "true");
        }
        else if (item.label == "Tab Width")
        {
            m_config.tabWidth = atoi(item.value.c_str());
        }
        else if (item.label == "Semicolons")
        {
            m_config.semi = (item.value == "true");
        }
        else if (item.label == "Single Quotes")
        {
            m_config.singleQuote = (item.value == "true");
        }
        else if (item.label == "Trailing Comma")
        {
            m_config.trailingComma = item.value;
        }
        else if (item.label == "Bracket Spacing")
        {
            m_config.bracketSpacing = (item.value == "true");
        }
        else if (item.label == "Arrow Parens")
        {
            m_config.arrowParens = item.value;
        }
        else if (item.label == "End of Line")
        {
            m_config.endOfLine = item.value;
        }
        else if (item.label == "Brace Style")
        {
            m_config.braceStyle = item.value;
        }
        else if (item.label == "Space Before Brace")
        {
            m_config.spaceBeforeBrace = (item.value == "true");
        }
        else if (item.label == "Space After Keyword")
        {
            m_config.spaceAfterKeyword = (item.value == "true");
        }
        else if (item.label == "Space Around Ops")
        {
            m_config.spaceAroundOperators = (item.value == "true");
        }
    }
}

int TuiConfigurator::readKey()
{
#ifdef _WIN32
    int c = _getch();
    if (c == 0 || c == 0xE0)
    {
        // Extended key - read the scan code
        c = _getch();
        return c;
    }
    return c;
#else
    // Unix: read raw terminal input
    // This is a simplified version - full termios handling would be complex
    int c = getchar();
    if (c == EOF)
    {
        return 0;
    }
    return c;
#endif
}

void TuiConfigurator::clearScreen()
{
    printf("%s", CLEAR_SCREEN);
    fflush(stdout);
}

void TuiConfigurator::moveCursor(int row, int col)
{
    printf(CSI "%d;%dH", row, col);
    fflush(stdout);
}

int TuiConfigurator::getTerminalWidth()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
    {
        return ws.ws_col;
    }
#endif
    return 80;
}

int TuiConfigurator::getTerminalHeight()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
    {
        return ws.ws_row;
    }
#endif
    return 24;
}