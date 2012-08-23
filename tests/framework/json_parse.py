#!/usr/bin/env python

#
# Sample python code to print testsuite name and test results from the JSON logfile
#

import json

#
# Create classes from JSON dictionary
#
class TestResult:
    def __init__(self, test_result):
        self.name = test_result["Test Name"]
        self.result = test_result["Test Result"]
        self.reports = test_result["Test Reports"]
        self.cpu_time = test_result["CPU Time"]
        self.run_time = test_result["Run Time"]

        if ("Malloc Stats" in test_result):
            self.malloc_stats = 1

    def __str__(self):
        return "Test '%s' ended with '%s' (CPU %is) (TIME %is)" % \
               (self.name, self.result, self.cpu_time, self.run_time)


class TestSuite:
    def __init__(self, testsuite_result):
        self.name = testsuite_result["Suite Name"]
        self.test_results = []

        for test_result in testsuite_result["Test Results"]:
             self.test_results.append(TestResult(test_result))

    def __str__(self):
        ret = '\t\t' + self.name + '\n\n'
        
        max_len = 0
        for i in self.test_results:
            max_len = max(max_len, len(i.name))

        for i in self.test_results:
            ret += i.name
        
            padds = (max_len - len(i.name))

            while padds > 0:
                ret += ' '
                padds-=1

            ret += "  | %.3fs/%.3fs |  %s\n" % \
                   (i.cpu_time, i.run_time, i.result)
            
            for j in i.reports:
                ret += " (%s)\n" % j

        return ret

def main():
    # parse JSON
    f = open('log.json')
    data = json.load(f)
    f.close()

    # convert to python objects
    test_suite = TestSuite(data)
    
    print(test_suite)

if __name__ == '__main__': 
    main()
