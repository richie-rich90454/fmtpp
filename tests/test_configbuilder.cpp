// Easter egg: config builder tests - because defaults deserve verification too
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "fmtpp/Config.hpp"
#include "fmtpp/ConfigBuilder.hpp"
#include "fmtpp/StyleDetector.hpp"

#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(p) _mkdir(p)
#define TEST_RMDIR(p) _rmdir(p)
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(p) mkdir(p, 0755)
#define TEST_RMDIR(p) rmdir(p)
#endif

static bool writeFile(const char* path, const char* content)
{
    std::ofstream file(path);
    if (!file.is_open())
    {
        return false;
    }
    file << content;
    return file.good();
}

int main()
{
    int failures = 0;

    // Test 1: buildDefault returns Prettier defaults
    {
        Config config = ConfigBuilder::buildDefault();
        if (config.printWidth != 80) { failures++; printf("FAIL: default printWidth should be 80\n"); }
        if (config.useTabs != false) { failures++; printf("FAIL: default useTabs should be false\n"); }
        if (config.tabWidth != 2) { failures++; printf("FAIL: default tabWidth should be 2\n"); }
        if (config.semi != true) { failures++; printf("FAIL: default semi should be true\n"); }
        if (config.singleQuote != false) { failures++; printf("FAIL: default singleQuote should be false\n"); }
        if (config.trailingComma != "es5") { failures++; printf("FAIL: default trailingComma should be es5\n"); }
        if (config.bracketSpacing != true) { failures++; printf("FAIL: default bracketSpacing should be true\n"); }
        if (config.arrowParens != "always") { failures++; printf("FAIL: default arrowParens should be always\n"); }
        if (config.endOfLine != "lf") { failures++; printf("FAIL: default endOfLine should be lf\n"); }
        if (config.braceStyle != "same-line") { failures++; printf("FAIL: default braceStyle should be same-line\n"); }
        if (config.spaceBeforeBrace != true) { failures++; printf("FAIL: default spaceBeforeBrace should be true\n"); }
        if (config.spaceAfterKeyword != true) { failures++; printf("FAIL: default spaceAfterKeyword should be true\n"); }
        if (config.spaceAroundOperators != true) { failures++; printf("FAIL: default spaceAroundOperators should be true\n"); }
        if (config.proseWrap != "preserve") { failures++; printf("FAIL: default proseWrap should be preserve\n"); }
        if (config.htmlWhitespaceSensitivity != "css") { failures++; printf("FAIL: default htmlWhitespaceSensitivity should be css\n"); }
        if (config.vueIndentScriptAndStyle != false) { failures++; printf("FAIL: default vueIndentScriptAndStyle should be false\n"); }
        if (config.embeddedLanguageFormatting != "auto") { failures++; printf("FAIL: default embeddedLanguageFormatting should be auto\n"); }
        if (config.singleAttributePerLine != false) { failures++; printf("FAIL: default singleAttributePerLine should be false\n"); }
    }

    // Test 2: missing file returns defaults
    {
        Config config = ConfigBuilder::buildFromFile("definitely_missing_config_12345.toml");
        if (config.printWidth != 80) { failures++; printf("FAIL: missing file printWidth should default to 80\n"); }
        if (config.tabWidth != 2) { failures++; printf("FAIL: missing file tabWidth should default to 2\n"); }
    }

    // Test 3: valid TOML overrides defaults
    {
        const char* path = "test_config_valid.toml";
        const char* content =
            "printWidth = 120\n"
            "useTabs = true\n"
            "tabWidth = 4\n"
            "semi = false\n"
            "singleQuote = true\n"
            "trailingComma = \"all\"\n"
            "bracketSpacing = false\n"
            "arrowParens = \"avoid\"\n"
            "endOfLine = \"crlf\"\n"
            "braceStyle = \"next-line\"\n"
            "spaceBeforeBrace = false\n"
            "spaceAfterKeyword = false\n"
            "spaceAroundOperators = false\n"
            "proseWrap = \"always\"\n"
            "htmlWhitespaceSensitivity = \"strict\"\n"
            "vueIndentScriptAndStyle = true\n"
            "embeddedLanguageFormatting = \"off\"\n"
            "singleAttributePerLine = true\n";
        if (writeFile(path, content))
        {
            Config config = ConfigBuilder::buildFromFile(path);
            if (config.printWidth != 120) { failures++; printf("FAIL: printWidth override failed\n"); }
            if (config.useTabs != true) { failures++; printf("FAIL: useTabs override failed\n"); }
            if (config.tabWidth != 4) { failures++; printf("FAIL: tabWidth override failed\n"); }
            if (config.semi != false) { failures++; printf("FAIL: semi override failed\n"); }
            if (config.singleQuote != true) { failures++; printf("FAIL: singleQuote override failed\n"); }
            if (config.trailingComma != "all") { failures++; printf("FAIL: trailingComma override failed\n"); }
            if (config.bracketSpacing != false) { failures++; printf("FAIL: bracketSpacing override failed\n"); }
            if (config.arrowParens != "avoid") { failures++; printf("FAIL: arrowParens override failed\n"); }
            if (config.endOfLine != "crlf") { failures++; printf("FAIL: endOfLine override failed\n"); }
            if (config.braceStyle != "next-line") { failures++; printf("FAIL: braceStyle override failed\n"); }
            if (config.spaceBeforeBrace != false) { failures++; printf("FAIL: spaceBeforeBrace override failed\n"); }
            if (config.spaceAfterKeyword != false) { failures++; printf("FAIL: spaceAfterKeyword override failed\n"); }
            if (config.spaceAroundOperators != false) { failures++; printf("FAIL: spaceAroundOperators override failed\n"); }
            if (config.proseWrap != "always") { failures++; printf("FAIL: proseWrap override failed\n"); }
            if (config.htmlWhitespaceSensitivity != "strict") { failures++; printf("FAIL: htmlWhitespaceSensitivity override failed\n"); }
            if (config.vueIndentScriptAndStyle != true) { failures++; printf("FAIL: vueIndentScriptAndStyle override failed\n"); }
            if (config.embeddedLanguageFormatting != "off") { failures++; printf("FAIL: embeddedLanguageFormatting override failed\n"); }
            if (config.singleAttributePerLine != true) { failures++; printf("FAIL: singleAttributePerLine override failed\n"); }
        }
        else
        {
            printf("FAIL: could not write test_config_valid.toml\n");
            failures++;
        }
        remove(path);
    }

    // Test 4: unrecognized keys are ignored
    {
        const char* path = "test_config_unknown.toml";
        const char* content =
            "printWidth = 100\n"
            "unknownKey = 42\n"
            "anotherUnrecognized = \"ignored\"\n";
        if (writeFile(path, content))
        {
            Config config = ConfigBuilder::buildFromFile(path);
            if (config.printWidth != 100) { failures++; printf("FAIL: printWidth should be 100 when mixed with unknown keys\n"); }
            if (config.tabWidth != 2) { failures++; printf("FAIL: tabWidth should remain default when only unknown keys are present\n"); }
        }
        else
        {
            printf("FAIL: could not write test_config_unknown.toml\n");
            failures++;
        }
        remove(path);
    }

    // Test 5: invalid TOML returns defaults
    {
        const char* path = "test_config_invalid.toml";
        const char* content = "printWidth = [this is not valid toml\n";
        if (writeFile(path, content))
        {
            Config config = ConfigBuilder::buildFromFile(path);
            if (config.printWidth != 80) { failures++; printf("FAIL: invalid TOML should return default printWidth\n"); }
        }
        else
        {
            printf("FAIL: could not write test_config_invalid.toml\n");
            failures++;
        }
        remove(path);
    }

    // Test 6: buildForFile finds .fmtpp.toml next to the source file
    {
        TEST_MKDIR("test_cfg_sub");
        if (writeFile("test_cfg_sub/.fmtpp.toml", "semi = false\nprintWidth = 100\n"))
        {
            Config config = ConfigBuilder::buildForFile("test_cfg_sub/dummy.js");
            if (config.semi != false) { failures++; printf("FAIL: buildForFile should pick up semi=false from the file's directory\n"); }
            if (config.printWidth != 100) { failures++; printf("FAIL: buildForFile should pick up printWidth=100 from the file's directory\n"); }
        }
        else
        {
            printf("FAIL: could not write test_cfg_sub/.fmtpp.toml\n");
            failures++;
        }
        remove("test_cfg_sub/.fmtpp.toml");
        TEST_RMDIR("test_cfg_sub");
    }

    // Test 7: buildForFile falls back to defaults when no config exists
    {
        Config config = ConfigBuilder::buildForFile("definitely_missing_dir_12345/dummy.js");
        if (config.semi != true) { failures++; printf("FAIL: buildForFile should default semi to true\n"); }
        if (config.printWidth != 80) { failures++; printf("FAIL: buildForFile should default printWidth to 80\n"); }
    }

    // Test 8: detectFromSource reads 4-space indent, semis, single quotes
    {
        const char* src =
            "import { x } from 'mod';\n"
            "function add(a, b) {\n"
            "    return a + b;\n"
            "}\n";
        StyleDetector detector;
        Config config = detector.detectFromSource(src, strlen(src));
        if (config.tabWidth != 4) { failures++; printf("FAIL: tabWidth should be 4, got %d\n", config.tabWidth); }
        if (config.useTabs != false) { failures++; printf("FAIL: useTabs should be false\n"); }
        if (config.semi != true) { failures++; printf("FAIL: semi should be true\n"); }
        if (config.singleQuote != true) { failures++; printf("FAIL: singleQuote should be true\n"); }
        if (config.braceStyle != "same-line") { failures++; printf("FAIL: braceStyle should be same-line\n"); }
        if (config.endOfLine != "lf") { failures++; printf("FAIL: endOfLine should be lf\n"); }
        if (config.printWidth != 80) { failures++; printf("FAIL: printWidth should stay 80, got %d\n", config.printWidth); }
    }

    // Test 9: detectFromSource reads tabs, missing semis, next-line braces, CRLF
    {
        const char* src =
            "package main\r\n"
            "func main()\r\n"
            "{\r\n"
            "\tprintln(\"hi\")\r\n"
            "}\r\n";
        StyleDetector detector;
        Config config = detector.detectFromSource(src, strlen(src));
        if (config.useTabs != true) { failures++; printf("FAIL: useTabs should be true\n"); }
        if (config.semi != false) { failures++; printf("FAIL: semi should be false\n"); }
        if (config.braceStyle != "next-line") { failures++; printf("FAIL: braceStyle should be next-line\n"); }
        if (config.endOfLine != "crlf") { failures++; printf("FAIL: endOfLine should be crlf\n"); }
    }

    // Test 10: detectFromSource raises printWidth for long lines
    {
        char src[112];
        for (int i = 0; i < 100; i++) { src[i] = 'x'; }
        src[100] = ';';
        src[101] = '\n';
        src[102] = '\0';
        StyleDetector detector;
        Config config = detector.detectFromSource(src, 102);
        if (config.printWidth != 101) { failures++; printf("FAIL: printWidth should be 101, got %d\n", config.printWidth); }
    }

    // Test 11: detectFromSource on empty input keeps defaults
    {
        StyleDetector detector;
        Config config = detector.detectFromSource("", 0);
        if (config.printWidth != 80) { failures++; printf("FAIL: empty input printWidth should be 80\n"); }
        if (config.tabWidth != 2) { failures++; printf("FAIL: empty input tabWidth should be 2\n"); }
        if (config.semi != true) { failures++; printf("FAIL: empty input semi should be true\n"); }
    }

    if (failures == 0)
    {
        printf("ConfigBuilder tests passed.\n");
    }
    else
    {
        printf("ConfigBuilder tests FAILED: %d failure(s)\n", failures);
    }

    return failures;
}
