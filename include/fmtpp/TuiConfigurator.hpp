#pragma once
#include "fmtpp/Config.hpp"
#include <string>
#include <vector>

/**
 * Interactive TUI configurator for fmtpp.
 * Uses ANSI escape codes to render a terminal UI for editing
 * formatting preferences. No external dependencies required.
 *
 * Easter egg: the TUI configurator - because GUIs are overrated
 */
class TuiConfigurator
{
public:
    /** Menu item types for the configurator. */
    enum MenuType
    {
        MENU_BOOL,      // Boolean toggle
        MENU_INT,       // Integer input
        MENU_STRING,    // String selector
        MENU_ACTION,    // Action button (save, exit, etc.)
        MENU_LABEL       // Read-only label
    };

    /** Represents a single selectable menu item. */
    struct MenuItem
    {
        std::string label;
        std::string value;
        MenuType type;
        int minValue;
        int maxValue;
        std::vector<std::string> options; // For string selectors
        int optionIndex;
    };

    TuiConfigurator();
    ~TuiConfigurator();

    /**
     * Runs the interactive TUI configurator.
     * @param config The current config to edit.
     * @return true if the user saved changes.
     */
    bool run(Config& config);

    /**
     * Gets the final config after user edits.
     */
    const Config& getConfig() const;

private:
    TuiConfigurator(const TuiConfigurator& other);
    TuiConfigurator& operator=(const TuiConfigurator& other);

    /** Builds the menu items from the config. */
    void buildMenu(const Config& config);

    /** Renders the full TUI screen. */
    void render();

    /** Renders the menu panel (left side). */
    void renderMenu();

    /** Renders the preview panel (right side). */
    void renderPreview();

    /** Handles a keypress. */
    int handleKey(int key);

    /** Toggles a boolean menu item. */
    void toggleBool(int index);

    /** Cycles through options for a string selector. */
    void cycleOption(int index);

    /** Reads an integer value from the user. */
    void readInteger(int index);

    /** Saves the config to disk. */
    bool saveConfig();

    /** Reads a single key from stdin without echo. */
    int readKey();

    /** Clears the screen. */
    void clearScreen();

    /** Moves cursor to position. */
    void moveCursor(int row, int col);

    /** Gets terminal width. */
    int getTerminalWidth();

    /** Gets terminal height. */
    int getTerminalHeight();

    /** Applies menu changes back to the config object. */
    void applyToConfig();

    Config m_config;
    std::vector<MenuItem> m_menuItems;
    int m_selectedIndex;
    int m_terminalWidth;
    int m_terminalHeight;
    bool m_running;
    bool m_saved;
    bool m_modified;

    // Easter egg: the TUI configurator - formatting with style
};