/**
 * @file test_main.cpp
 * @brief Generic test runner main program for CppUnit based test suites
 * @copyright 2019 Sphairon GmbH (a ZyXEL Company)
 *
 * SPDX-License-Identifier: Zyxel
 */

#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/JUnitTestResultCollector.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestRunner.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/JUnitXmlOutputter.h>

#include <unistd.h>
#include <iostream>

#ifndef SAS_CPPUNIT_LOGFILENAME
#define SAS_CPPUNIT_LOGFILENAME "test_results.cppunit.xml"
#endif

#ifndef SAS_JUNIT_LOGFILENAME
#define SAS_JUNIT_LOGFILENAME "test_results.junit.xml"
#endif

using namespace CPPUNIT_NS;

using std::cerr;
using std::cout;
using std::endl;
using std::string;

bool g_bSyslogLevel = true;

static void usage(const char* image, const char* defaultTest)
{
    cout << endl;
    cout << "Usage: " << image << " (-t test)" << endl;
    cout << "       " << image << " [ -l | -h | -c]" << endl;
    cout << endl;

    cout << "    -t Run given test only. Default: \"" << defaultTest << "\"" << endl;
    cout << "    -l List all available tests." << endl;
    cout << "    -c Output syslog to console." << endl;
    cout << "    -h Print this usage message." << endl;

    cout << endl;
}

static void dump(Test* test)
{
    if (0 == test)
        return;

    cout << test->getName() << endl;

    if (0 == test->getChildTestCount())
        return;

    for (int i = 0; i < test->getChildTestCount(); i++)
    {
        dump(test->getChildTestAt(i));
    }
}

static Test* find(Test* test, const string &name)
{
    if (0 == test)
        return 0;

    if (name == test->getName())
        return test;

    if (0 == test->getChildTestCount())
        return 0;

    for (int i = 0; i < test->getChildTestCount(); i++)
    {
        Test* found = find(test->getChildTestAt(i), name);
        if (found)
            return found;
    }

    return 0;
}

int main(int argc, char **argv)
{
    /** Create the event manager and test controller */
    CPPUNIT_NS::TestResult testController;

    /** Add a listener that collects test results */
    CPPUNIT_NS::TestResultCollector testResult;
    testController.addListener(&testResult);

    /** Add a second listener that collects results for output in JUnit format */
    CPPUNIT_NS::JUnitTestResultCollector junitTestResult;
    testController.addListener(&junitTestResult);

    /** Add a listener that prints the progress of all tests */
    CPPUNIT_NS::BriefTestProgressListener testProgressListener;
    testController.addListener(&testProgressListener);

    string runTest = "All Tests";
    char flag = 0;

    while ((flag = getopt(argc, argv, "t:lhc")) != -1)
    {
        switch (flag)
        {
            case 'c':
            {
                g_bSyslogLevel = false;
                break;
            }
            case 'l':
            {
                Test* all = TestFactoryRegistry::getRegistry().makeTest();
                dump(all);
                return 0;
            }
            case 't':
            {
                runTest = optarg;
                break;
            }
            case 'h': /* Falls through. */
            default:
            {
                usage(argv[0], runTest.c_str());
                return -1;
            }
        }
    }

    Test *run = find(TestFactoryRegistry::getRegistry().makeTest(), runTest);
    if (run == 0) 
    {
      cerr << "Unknown test case: " << runTest << endl;
      return -2;
    }

    /** Add the top suite to the test runner */
    CPPUNIT_NS::TestRunner testRunner;
    testRunner.addTest(run);
    testRunner.run(testController);

    /** Print test in a compiler compatible format */
    CPPUNIT_NS::CompilerOutputter compilerOutputter(&testResult, std::cout);

    /** Set format to "filepath:line: XXX" (works well with SlickEdit) */
    compilerOutputter.setLocationFormat("%p:%l: ");
    compilerOutputter.write();

    /** Write test results into xml file */
    std::ofstream xmlOutputFile(SAS_CPPUNIT_LOGFILENAME);
    CPPUNIT_NS::XmlOutputter xmlOutputter(&testResult, xmlOutputFile);
    xmlOutputter.write();
    xmlOutputFile.close();

    /** Write test results into JUnit xml file too */
    std::ofstream junitXmlOutputFile(SAS_JUNIT_LOGFILENAME);
    CPPUNIT_NS::JUnitXmlOutputter junitXmlOutputter(&junitTestResult, junitXmlOutputFile);
    junitXmlOutputter.write();
    junitXmlOutputFile.close();

    return (testResult.wasSuccessful() == true) ? 0 : 1;
}
