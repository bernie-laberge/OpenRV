//******************************************************************************
// Copyright (c) 2026 Autodesk Inc. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
//******************************************************************************

//
// Unit tests for TwkUtil::CrashHandler
//
// NOTE: These tests are designed to be run with a testing framework like Google Test.
// They test the CrashHandler API without actually triggering crashes.
//

#include <TwkUtil/CrashHandler.h>
#include <iostream>
#include <cassert>

namespace
{

    void testSingletonBehavior()
    {
        std::cout << "Test: Singleton behavior" << std::endl;

        TwkUtil::CrashHandler& handler1 = TwkUtil::CrashHandler::instance();
        TwkUtil::CrashHandler& handler2 = TwkUtil::CrashHandler::instance();

        // Both should reference the same instance
        assert(&handler1 == &handler2);

        std::cout << "  PASS: Singleton returns same instance" << std::endl;
    }

    void testInitializationWithInvalidPath()
    {
        std::cout << "Test: Initialization with invalid handler path" << std::endl;

        TwkUtil::CrashHandler& handler = TwkUtil::CrashHandler::instance();

        // Try to initialize with non-existent handler
        bool success = handler.initialize("TestApp", "1.0.0", "/nonexistent/path/to/crashpad_handler");

        // Should fail gracefully
        assert(!success);
        assert(!handler.isInitialized());

        std::cout << "  PASS: Invalid path handled correctly" << std::endl;
    }

    void testAnnotationsBeforeInitialization()
    {
        std::cout << "Test: Adding annotations before initialization" << std::endl;

        TwkUtil::CrashHandler& handler = TwkUtil::CrashHandler::instance();

        // Should not crash when adding annotations before initialization
        handler.addAnnotation("test_key", "test_value");

        std::cout << "  PASS: Annotations before initialization handled gracefully" << std::endl;
    }

    void testAttachLogFileBeforeInitialization()
    {
        std::cout << "Test: Attaching log file before initialization" << std::endl;

        TwkUtil::CrashHandler& handler = TwkUtil::CrashHandler::instance();

        // Should not crash when attaching log file before initialization
        handler.attachLogFile("/tmp/test.log");

        std::cout << "  PASS: Log file attachment before initialization handled gracefully" << std::endl;
    }

    void testGetCrashDumpDirectoryBeforeInitialization()
    {
        std::cout << "Test: Getting crash dump directory before initialization" << std::endl;

        TwkUtil::CrashHandler& handler = TwkUtil::CrashHandler::instance();

        std::string dir = handler.getCrashDumpDirectory();

        // Should return empty string if not initialized (on placeholder platforms)
        // or a valid directory if initialized (on implemented platforms)
        std::cout << "  Crash dump directory: " << (dir.empty() ? "(empty)" : dir) << std::endl;
        std::cout << "  PASS: getCrashDumpDirectory() didn't crash" << std::endl;
    }

    void testEnvironmentVariableDisable()
    {
        std::cout << "Test: Disable via environment variable" << std::endl;
        std::cout << "  NOTE: Set RV_CRASH_DUMPS_ENABLED=0 to test disable functionality" << std::endl;

        // This test is informational - actual testing requires setting environment variable
        // and restarting the test process
        std::cout << "  SKIP: Requires manual environment variable configuration" << std::endl;
    }

    void testEnvironmentVariableCustomDirectory()
    {
        std::cout << "Test: Custom directory via environment variable" << std::endl;
        std::cout << "  NOTE: Set RV_CRASH_DUMPS_DIR=/custom/path to test custom directory" << std::endl;

        // This test is informational - actual testing requires setting environment variable
        // and restarting the test process
        std::cout << "  SKIP: Requires manual environment variable configuration" << std::endl;
    }

    // Deliberate crash test functions (commented out by default)
    // Uncomment these for manual crash testing - they WILL terminate the process!

    /*
    void testCrashSegfault()
    {
        std::cout << "Test: Deliberate SIGSEGV crash (THIS WILL CRASH!)" << std::endl;
        int* ptr = nullptr;
        *ptr = 42;  // Segmentation fault
    }

    void testCrashAbort()
    {
        std::cout << "Test: Deliberate abort crash (THIS WILL CRASH!)" << std::endl;
        abort();
    }

    void testCrashDivideByZero()
    {
        std::cout << "Test: Deliberate divide by zero (THIS WILL CRASH!)" << std::endl;
        volatile int x = 0;
        volatile int y = 42 / x;
        (void)y;
    }
    */

} // anonymous namespace

int main(int argc, char* argv[])
{
    std::cout << "========================================" << std::endl;
    std::cout << "CrashHandler Unit Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    try
    {
        testSingletonBehavior();
        testInitializationWithInvalidPath();
        testAnnotationsBeforeInitialization();
        testAttachLogFileBeforeInitialization();
        testGetCrashDumpDirectoryBeforeInitialization();
        testEnvironmentVariableDisable();
        testEnvironmentVariableCustomDirectory();

        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "ERROR: Test failed with unknown exception" << std::endl;
        return 1;
    }
}
