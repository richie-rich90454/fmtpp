#include "fmtpp/FormatterFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/FormatterVisitor.hpp"
#include "fmtpp/JSFormatter.hpp"
#include "fmtpp/TSFormatter.hpp"
#include "fmtpp/JavaFormatter.hpp"
#include "fmtpp/CFormatter.hpp"
#include "fmtpp/CppFormatter.hpp"
#include "fmtpp/CSharpFormatter.hpp"
#include "fmtpp/GoFormatter.hpp"
#include "fmtpp/RustFormatter.hpp"
#include "fmtpp/KotlinFormatter.hpp"
#include "fmtpp/SwiftFormatter.hpp"
#include "fmtpp/DartFormatter.hpp"
#include "fmtpp/PhpFormatter.hpp"
#include "fmtpp/ScalaFormatter.hpp"
#include "fmtpp/ObjectiveCFormatter.hpp"
#include "fmtpp/ZigFormatter.hpp"
#include "fmtpp/DFormatter.hpp"
#include "fmtpp/GroovyFormatter.hpp"
#include "fmtpp/SolidityFormatter.hpp"
#include "fmtpp/VFormatter.hpp"
#include "fmtpp/GlslFormatter.hpp"
#include "fmtpp/HtmlFormatter.hpp"
#include "fmtpp/CssFormatter.hpp"
#include "fmtpp/VueFormatter.hpp"
#include "fmtpp/PythonFormatter.hpp"
#include "fmtpp/RubyFormatter.hpp"
#include <cstring>

// Easter egg: one factory to format them all, one factory to indent them

FormatterFactory::FormatterFactory() {}
FormatterFactory::~FormatterFactory() {}

std::string FormatterFactory::detectLanguage(const char* filepath)
{
    if (filepath == NULL)
    {
        return "js";
    }

    const char* dot = strrchr(filepath, '.');
    if (dot == NULL)
    {
        return "js";
    }

    const char* ext = dot + 1;

    if (strcmp(ext, "js") == 0 || strcmp(ext, "mjs") == 0
        || strcmp(ext, "cjs") == 0 || strcmp(ext, "jsx") == 0)
    {
        return "js";
    }
    if (strcmp(ext, "ts") == 0 || strcmp(ext, "tsx") == 0
        || strcmp(ext, "mts") == 0 || strcmp(ext, "cts") == 0)
    {
        return "ts";
    }
    if (strcmp(ext, "java") == 0)
    {
        return "java";
    }
    if (strcmp(ext, "c") == 0 || strcmp(ext, "h") == 0)
    {
        return "c";
    }
    if (strcmp(ext, "cpp") == 0 || strcmp(ext, "cc") == 0
        || strcmp(ext, "cxx") == 0 || strcmp(ext, "hpp") == 0
        || strcmp(ext, "hh") == 0 || strcmp(ext, "hxx") == 0
        || strcmp(ext, "ino") == 0 || strcmp(ext, "cu") == 0
        || strcmp(ext, "cuh") == 0)
    {
        return "cpp";
    }
    if (strcmp(ext, "cs") == 0)
    {
        return "cs";
    }
    if (strcmp(ext, "go") == 0)
    {
        return "go";
    }
    if (strcmp(ext, "rs") == 0)
    {
        return "rs";
    }
    if (strcmp(ext, "kt") == 0 || strcmp(ext, "kts") == 0)
    {
        return "kotlin";
    }
    if (strcmp(ext, "swift") == 0)
    {
        return "swift";
    }
    if (strcmp(ext, "dart") == 0)
    {
        return "dart";
    }
    if (strcmp(ext, "php") == 0 || strcmp(ext, "phtml") == 0)
    {
        return "php";
    }
    if (strcmp(ext, "scala") == 0 || strcmp(ext, "sc") == 0)
    {
        return "scala";
    }
    if (strcmp(ext, "m") == 0 || strcmp(ext, "mm") == 0)
    {
        return "objc";
    }
    if (strcmp(ext, "zig") == 0 || strcmp(ext, "zon") == 0)
    {
        return "zig";
    }
    if (strcmp(ext, "d") == 0 || strcmp(ext, "di") == 0)
    {
        return "d";
    }
    if (strcmp(ext, "groovy") == 0 || strcmp(ext, "gvy") == 0)
    {
        return "groovy";
    }
    if (strcmp(ext, "sol") == 0)
    {
        return "solidity";
    }
    if (strcmp(ext, "v") == 0 || strcmp(ext, "vsh") == 0 || strcmp(ext, "vv") == 0)
    {
        return "v";
    }
    if (strcmp(ext, "glsl") == 0 || strcmp(ext, "vert") == 0
        || strcmp(ext, "frag") == 0 || strcmp(ext, "geom") == 0
        || strcmp(ext, "comp") == 0 || strcmp(ext, "tesc") == 0
        || strcmp(ext, "tese") == 0 || strcmp(ext, "vs") == 0
        || strcmp(ext, "fs") == 0)
    {
        return "glsl";
    }
    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0
        || strcmp(ext, "xhtml") == 0 || strcmp(ext, "svg") == 0
        || strcmp(ext, "xml") == 0)
    {
        return "html";
    }
    if (strcmp(ext, "css") == 0 || strcmp(ext, "scss") == 0
        || strcmp(ext, "less") == 0)
    {
        return "css";
    }
    if (strcmp(ext, "json") == 0)
    {
        return "json";
    }
    if (strcmp(ext, "vue") == 0)
    {
        return "vue";
    }
    if (strcmp(ext, "py") == 0 || strcmp(ext, "pyw") == 0
        || strcmp(ext, "pyi") == 0)
    {
        return "python";
    }
    if (strcmp(ext, "rb") == 0)
    {
        return "ruby";
    }

    return "js";
}

FormatterVisitor* FormatterFactory::create(const std::string& language, OutputBuffer& out, const Config& config)
{
    // ponytail: one dispatch table; aliases normalized here, not at call sites
    if (language == "js" || language == "javascript" || language == "json")
    {
        return new JSFormatter(out, config);
    }
    if (language == "ts" || language == "typescript")
    {
        return new TSFormatter(out, config);
    }
    if (language == "java")
    {
        return new JavaFormatter(out, config);
    }
    if (language == "c")
    {
        return new CFormatter(out, config);
    }
    if (language == "cpp" || language == "c++")
    {
        return new CppFormatter(out, config);
    }
    if (language == "cs" || language == "csharp" || language == "c#")
    {
        return new CSharpFormatter(out, config);
    }
    if (language == "go")
    {
        return new GoFormatter(out, config);
    }
    if (language == "rs" || language == "rust")
    {
        return new RustFormatter(out, config);
    }
    if (language == "kotlin")
    {
        return new KotlinFormatter(out, config);
    }
    if (language == "swift")
    {
        return new SwiftFormatter(out, config);
    }
    if (language == "dart")
    {
        return new DartFormatter(out, config);
    }
    if (language == "php")
    {
        return new PhpFormatter(out, config);
    }
    if (language == "scala")
    {
        return new ScalaFormatter(out, config);
    }
    if (language == "objc" || language == "objective-c" || language == "objectivec")
    {
        return new ObjectiveCFormatter(out, config);
    }
    if (language == "zig")
    {
        return new ZigFormatter(out, config);
    }
    if (language == "d")
    {
        return new DFormatter(out, config);
    }
    if (language == "groovy")
    {
        return new GroovyFormatter(out, config);
    }
    if (language == "solidity")
    {
        return new SolidityFormatter(out, config);
    }
    if (language == "v" || language == "vlang")
    {
        return new VFormatter(out, config);
    }
    if (language == "glsl")
    {
        return new GlslFormatter(out, config);
    }
    if (language == "html" || language == "xml")
    {
        return new HtmlFormatter(out, config);
    }
    if (language == "css")
    {
        return new CssFormatter(out, config);
    }
    if (language == "vue")
    {
        return new VueFormatter(out, config);
    }
    if (language == "python" || language == "py")
    {
        return new PythonFormatter(out, config);
    }
    if (language == "ruby" || language == "rb")
    {
        return new RubyFormatter(out, config);
    }
    return new JSFormatter(out, config);
}

// Easter egg: the factory floor keeps expanding - 25 languages and counting
