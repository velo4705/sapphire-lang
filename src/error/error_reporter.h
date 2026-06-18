#pragma once
#include "error_codes.h"
#include "exception.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <unistd.h>

namespace sapphire {

// ── ANSI colours (disabled if not a tty) ──────────────────────────────────
static inline bool use_color() {
    static int cached = -1;
    if (cached < 0) cached = isatty(fileno(stderr)) ? 1 : 0;
    return cached == 1;
}
#define C_RESET  (sapphire::use_color() ? "\033[0m"     : "")
#define C_BOLD   (sapphire::use_color() ? "\033[1m"     : "")
#define C_RED    (sapphire::use_color() ? "\033[1;31m"  : "")
#define C_YELLOW (sapphire::use_color() ? "\033[1;33m"  : "")
#define C_CYAN   (sapphire::use_color() ? "\033[1;36m"  : "")
#define C_BLUE   (sapphire::use_color() ? "\033[1;34m"  : "")
#define C_DIM    (sapphire::use_color() ? "\033[2m"     : "")

// ── Split source into lines ────────────────────────────────────────────────
static inline std::vector<std::string> split_lines(const std::string& src) {
    std::vector<std::string> lines;
    std::istringstream ss(src);
    std::string line;
    while (std::getline(ss, line)) lines.push_back(line);
    return lines;
}

// ── Main formatter ─────────────────────────────────────────────────────────
//
// Produces output like:
//
//   error[E201]: undefined variable 'foo'
//    --> script.spp:12:5
//     |
//  12 |     x = foo + 1
//     |         ^^^ not defined here
//     |
//     = note: check the spelling, or make sure the variable is defined before use
//     = see:  https://sapphire-lang.org/errors/E201
//
class ErrorFormatter {
public:
    static void print(
        const std::string& error_code_str,   // e.g. "E201"  (empty = no code)
        const std::string& message,
        const std::string& filename,
        int line,                             // 1-based, -1 = unknown
        int col,                              // 1-based, -1 = unknown
        const std::string& source_text,       // full source (may be empty)
        const std::string& extra_note = "",   // override note (empty = use code's note)
        bool is_warning = false
    ) {
        const ErrorCode* ec = error_code_str.empty()
                              ? nullptr
                              : lookup_error_code(error_code_str);

        // ── Header line ───────────────────────────────────────────────────
        std::string severity_color = is_warning ? C_YELLOW : C_RED;
        std::string severity_label = is_warning ? "warning" : "error";

        std::cerr << C_BOLD << severity_color << severity_label;
        if (ec) std::cerr << "[" << ec->code << "]";
        std::cerr << C_RESET << C_BOLD << ": " << message << C_RESET << "\n";

        // ── Location arrow ────────────────────────────────────────────────
        if (!filename.empty() && line > 0) {
            std::cerr << C_BLUE << " --> " << C_RESET
                      << filename << ":" << line;
            if (col > 0) std::cerr << ":" << col;
            std::cerr << "\n";
        }

        // ── Source snippet with caret ─────────────────────────────────────
        if (!source_text.empty() && line > 0) {
            auto lines = split_lines(source_text);
            int idx = line - 1;
            if (idx >= 0 && idx < (int)lines.size()) {
                std::string line_no = std::to_string(line);
                std::string pad(line_no.size(), ' ');

                std::cerr << C_BLUE << "  " << pad << " |" << C_RESET << "\n";
                std::cerr << C_BLUE << "  " << line_no << " | " << C_RESET
                          << lines[idx] << "\n";

                // caret
                std::string caret_pad(line_no.size(), ' ');
                std::cerr << C_BLUE << "  " << caret_pad << " | " << C_RESET;
                if (col > 1) std::cerr << std::string(col - 1, ' ');
                std::cerr << C_RED << "^" << C_RESET << "\n";

                std::cerr << C_BLUE << "  " << pad << " |" << C_RESET << "\n";
            }
        }

        // ── Note ──────────────────────────────────────────────────────────
        std::string note = extra_note;
        if (note.empty() && ec) note = ec->note;
        if (!note.empty()) {
            std::cerr << C_BLUE << "  = " << C_RESET
                      << C_BOLD << "note: " << C_RESET
                      << note << "\n";
        }

        // ── Docs link ─────────────────────────────────────────────────────
        if (ec && ec->docs_url && ec->docs_url[0]) {
            std::cerr << C_BLUE << "  = " << C_RESET
                      << C_DIM << "see:  " << ec->docs_url << C_RESET << "\n";
        }

        std::cerr << "\n";
    }

    // Convenience: print from a SapphireException
    static void print(const SapphireException& e,
                      const std::string& source_text = "",
                      const std::string& error_code_str = "") {
        print(error_code_str,
              e.getMessage(),
              e.getFileName(),
              e.getLineNumber(),
              e.getColumnNumber(),
              source_text);
    }

    // Simple one-liner (no source snippet)
    static void print_simple(const std::string& error_code_str,
                             const std::string& message,
                             bool is_warning = false) {
        print(error_code_str, message, "", -1, -1, "", "", is_warning);
    }
};

} // namespace sapphire
