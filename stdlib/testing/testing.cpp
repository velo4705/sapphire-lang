#include "testing.h"
#include <cmath>
#include <sstream>

namespace sapphire {
namespace stdlib {

// Static member initialization
int Testing::total_tests = 0;
int Testing::passed_tests = 0;
int Testing::failed_tests = 0;
std::vector<std::string> Testing::failures;

void Testing::run_test(const std::string& name, std::function<void()> test_func) {
    total_tests++;
    try {
        test_func();
        passed_tests++;
        std::cout << "  ✓ " << name << std::endl;
    } catch (const std::exception& e) {
        failed_tests++;
        std::string failure = name + ": " + e.what();
        failures.push_back(failure);
        std::cout << "  ✗ " << name << " - " << e.what() << std::endl;
    }
}

void Testing::assert_true(bool condition, const std::string& message) {
    if (!condition) {
        std::string msg = message.empty() ? "Expected true, got false" : message;
        throw std::runtime_error(msg);
    }
}

void Testing::assert_false(bool condition, const std::string& message) {
    if (condition) {
        std::string msg = message.empty() ? "Expected false, got true" : message;
        throw std::runtime_error(msg);
    }
}

void Testing::assert_equal(int actual, int expected, const std::string& message) {
    if (actual != expected) {
        std::ostringstream oss;
        if (!message.empty()) {
            oss << message << " - ";
        }
        oss << "Expected " << expected << ", got " << actual;
        throw std::runtime_error(oss.str());
    }
}

void Testing::assert_equal(double actual, double expected, const std::string& message, double epsilon) {
    if (std::abs(actual - expected) > epsilon) {
        std::ostringstream oss;
        if (!message.empty()) {
            oss << message << " - ";
        }
        oss << "Expected " << expected << ", got " << actual;
        throw std::runtime_error(oss.str());
    }
}

void Testing::assert_equal(const std::string& actual, const std::string& expected, const std::string& message) {
    if (actual != expected) {
        std::ostringstream oss;
        if (!message.empty()) {
            oss << message << " - ";
        }
        oss << "Expected \"" << expected << "\", got \"" << actual << "\"";
        throw std::runtime_error(oss.str());
    }
}

void Testing::assert_not_equal(int actual, int expected, const std::string& message) {
    if (actual == expected) {
        std::ostringstream oss;
        if (!message.empty()) {
            oss << message << " - ";
        }
        oss << "Expected not equal to " << expected << ", but got " << actual;
        throw std::runtime_error(oss.str());
    }
}

void Testing::assert_not_equal(const std::string& actual, const std::string& expected, const std::string& message) {
    if (actual == expected) {
        std::ostringstream oss;
        if (!message.empty()) {
            oss << message << " - ";
        }
        oss << "Expected not equal to \"" << expected << "\", but got \"" << actual << "\"";
        throw std::runtime_error(oss.str());
    }
}

void Testing::assert_null(void* ptr, const std::string& message) {
    if (ptr != nullptr) {
        std::string msg = message.empty() ? "Expected null pointer" : message;
        throw std::runtime_error(msg);
    }
}

void Testing::assert_not_null(void* ptr, const std::string& message) {
    if (ptr == nullptr) {
        std::string msg = message.empty() ? "Expected non-null pointer" : message;
        throw std::runtime_error(msg);
    }
}

void Testing::print_summary() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Test Results                                                 ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Passed: " << passed_tests << "\n";
    std::cout << "Failed: " << failed_tests << "\n";
    std::cout << "Total:  " << total_tests << "\n\n";
    
    if (failed_tests > 0) {
        std::cout << "Failures:\n";
        for (const auto& failure : failures) {
            std::cout << "  - " << failure << "\n";
        }
        std::cout << "\n";
    }
    
    if (failed_tests == 0) {
        std::cout << "✓ All tests passed!\n";
    }
}

void Testing::reset() {
    total_tests = 0;
    passed_tests = 0;
    failed_tests = 0;
    failures.clear();
}

} // namespace stdlib
} // namespace sapphire

// C API implementation
extern "C" {
    using namespace sapphire::stdlib;
    
    void sapphire_test_assert_true(bool condition, const char* message) {
        Testing::assert_true(condition, message ? message : "");
    }
    
    void sapphire_test_assert_false(bool condition, const char* message) {
        Testing::assert_false(condition, message ? message : "");
    }
    
    void sapphire_test_assert_equal_int(int actual, int expected, const char* message) {
        Testing::assert_equal(actual, expected, message ? message : "");
    }
    
    void sapphire_test_assert_equal_str(const char* actual, const char* expected, const char* message) {
        Testing::assert_equal(
            std::string(actual ? actual : ""),
            std::string(expected ? expected : ""),
            message ? message : ""
        );
    }
    
    void sapphire_test_print_summary() {
        Testing::print_summary();
    }
}
