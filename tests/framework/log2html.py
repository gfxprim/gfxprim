#!/usr/bin/env python3

#
# Script to convert testsuite JSON log into html page
#
from sys import argv
from os import _exit
import json

#
# Test result to html color dict
#
html_colors = {
    'Success': '#008000',
    'Skipped': '#888888',
    'Untested': '#0000bb',
    'Internal Error': '#800000',
    'Segmentation Fault': '#e00000',
    'Timeout': '#800080',
    'Aborted': '#e00000',
    'FP Exception': '#e00000',
    'Memory Leak': '#a0a000',
    'Failed': '#e00000'
}

#
# Convert bytes to human-readable string
#
def bytes_conv(size):
    if (size < 512):
        return "%iB" % (size)

    if (size < 1024 * 512):
        return "%.2fkB" % (float(size) / 1024)

    if (size < 1024 * 1024 * 512):
        return "%.2fMB" % (float(size) / 1024 / 1024)

    return "%.2fGB" % (float(size) / 1024 / 1024 / 1024)

#
# Malloc statistics Class created from JSON dict
#
class MallocStats:
    def __init__(self, malloc_stats):
        self.total_size = malloc_stats["Total Size"]
        self.total_chunks = malloc_stats["Total Chunks"]
        self.max_size = malloc_stats["Max Size"]
        self.max_chunks = malloc_stats["Max Chunks"]
        self.lost_size = malloc_stats["Lost Size"]
        self.lost_chunks = malloc_stats["Lost Chunks"]

    def html(self):
        print('   <tr>')
        print('    <td bgcolor="#ffffcc" colspan="3">')
        print('     <center>')
        print('      <table>')

        # Table header
        print('       <tr>')

        print('        <td bgcolor="#ffffaa">')
        print('         <small>Total size</small>')
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <small>Total chunks</small>')
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <small>Max size</small>')
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <small>Max chunks</small>')
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <small>Lost size</small>')
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <small>Lost chunks</small>')
        print('        </td>')

        print('       </tr>')

        # Malloc data
        print('       <tr>')

        print('        <td bgcolor="#ffffaa">')
        print('         <center><small>%s</small></center>' %
              bytes_conv(self.total_size))
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <center><small>%s</small></center>' %
              self.total_chunks)
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <center><small>%s</small></center>' %
              bytes_conv(self.max_size))
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <center><small>%s</small></center>' %
              self.max_chunks)
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <center><small>%s</small></center>' %
              bytes_conv(self.lost_size))
        print('        </td>')

        print('        <td bgcolor="#ffffaa">')
        print('         <center><small>%s</small></center>' %
              self.lost_chunks)
        print('        </td>')

        print('       </tr>')

        print('      </table>')
        print('     </center>')
        print('    </td>')
        print('   </tr>')

#
# Benchmark statistics Class created from JSON dict
#
class BenchmarkData:
    def __init__(self, bench_data):
        self.time_mean = bench_data["Time Mean"]
        self.time_variance = bench_data["Time Variance"]
        self.iterations = bench_data["Iterations"]

    def html(self):
        print('   <tr>')
        print('    <td bgcolor="#fd8" colspan="3">')
        print('     <center>')
        print('      <table>')

        # Table header
        print('       <tr>')

        print('        <td colspan="2" bgcolor="#fb2">')
        print('         <center><small>Benchmark data</small></center>')
        print('        </td>')

        print('       </tr>')

        print('       <tr>')

        print('        <td bgcolor="#fc4">')
        print('         <center><small>Iterations</small></center>')
        print('        </td>')

        print('        <td bgcolor="#fc4">')
        print('         <center><small>Mean &#x2213; Variance</small></center>')
        print('        </td>')

        print('       </tr>')

        # Benchmark data
        print('       <tr>')

        print('        <td bgcolor=\"#fc4\">')
        print('         <small>%i</small>' % (self.iterations))
        print('        </td>')

        print('        <td bgcolor=\"#fc4\">')
        print('         <small>%.6fs &#x2213; %.6fs</small>' %
              (self.time_mean, self.time_variance))
        print('        </td>')

        print('       </tr>')

        print('      </table>')
        print('     </center>')
        print('    </td>')
        print('   </tr>')

#
# Test Result Class created from JSON dict
#
class TestResult:
    def __init__(self, test_result):
        self.name = test_result["Test Name"]
        self.result = test_result["Test Result"]
        self.reports = test_result["Test Reports"]
        self.cpu_time = test_result["CPU Time"]
        self.run_time = test_result["Run Time"]
        self.test_reports = test_result["Test Reports"]

        if ("Malloc Stats" in test_result):
            self.malloc_stats = MallocStats(test_result["Malloc Stats"])

        if ("Benchmark" in test_result):
            self.bench_data = BenchmarkData(test_result["Benchmark"])

    def html(self, bg_color):
        # Print test result
        print('   <tr>')

        print('    <td bgcolor="%s">%s&nbsp;</td>' % (bg_color, self.name))

        print('    <td bgcolor="%s">' % (bg_color))
        print('     <center><small><font color=\"#222\">')
        print('      %.3fs %.3fs' % (self.run_time, self.cpu_time))
        print('     </font></small></center>')
        print('    </td>')

        print('    <td bgcolor="%s">' % (html_colors[self.result]))
        print('     <center><font color="white">&nbsp;%s&nbsp;</center>' %
              (self.result))
        print('    </td>')

        print('   </tr>')

        # Add malloc statistics, if present
        if (hasattr(self, 'malloc_stats')):
            self.malloc_stats.html()

        # And benchmark data
        if (hasattr(self, 'bench_data')):
            self.bench_data.html()

        # And test messages
        if (self.test_reports):
            print('   <tr>')
            print('    <td colspan=\"3\" bgcolor=\"#eeeeee\">')

        for msg in self.test_reports:
            print('     &nbsp;&nbsp;<small>%s</small><br>' % (msg))

        if (self.test_reports):
            print('    </td>')
            print('   </tr>')

class TestSuite:
    def __init__(self, testsuite_result):
        self.suite_name = testsuite_result["Suite Name"]
        self.test_results = []

        for test_result in testsuite_result["Test Results"]:
             self.test_results.append(TestResult(test_result))

    def html(self):
        print('<html>')
        print(' <head>')
        print(' </head>')
        print(' <body>')

        print('  <table bgcolor="#99a">')
        print('   <tr>')
        print('    <td colspan="3" bgcolor="#bbbbff">');
        print('     <center><b>%s</b></center>' % (self.suite_name))
        print('    </td>')
        print('   </tr>')

        flag = False;

        for tst in self.test_results:
            if (flag):
                bg_color = '#ccccee'
            else:
                bg_color = '#ddddee'

            flag = not flag

            tst.html(bg_color)

        print('  </table>')

        print(' </body>')
        print('</html>')

    def results(self):
        res_dict = {}

        for i in html_colors:
            res_dict[i] = 0

        for tst in self.test_results:
            res_dict[tst.result] = res_dict[tst.result] + 1;

        return res_dict

    def retval(self):
        res_dict = self.results()

        succ = res_dict['Success']
        skip = res_dict['Skipped']
        untested = res_dict['Untested']
        fail = res_dict['Failed']
        if fail > 0:
            return 1;

        return 0;

    # Creates table row with a link to results page
    def html_summary(self, link):
        print('    <tr>')

        res_dict = self.results()

        test_ok  = res_dict['Success']
        test_ok += res_dict['Skipped']
        test_ok += res_dict['Untested']

        test_all = len(self.test_results)

        if (test_ok < test_all):
            bg_color = html_colors['Failed']
        else:
            bg_color = html_colors['Success']

        print('     <td bgcolor="#ccccee">%s</td>' % (self.suite_name))
        print('     <td bgcolor="%s">%i</td>' % (bg_color, test_all - test_ok))

        test_skipped = res_dict['Skipped']

        if (test_skipped > 0):
            bg_color = html_colors['Skipped']
        else:
            bg_color = '#ccccee'

        print('     <td bgcolor="%s">%i</td>' % (bg_color, test_skipped))
        print('     <td bgcolor="#ccccee">%i</td>' % (test_all))
        print('     <td bgcolor="#ccccee"><small><a href="%s">Details</a></small></td>' % (link))
        print('    </tr>')

def main():
    filename = 'log.json'
    summary = False
    just_exit = False
    pars = 1
    link = ''

    if (len(argv) > 1):
        if (argv[1] == '-s'):
            link = argv[2]
            pars = 3
            summary = True

        if (argv[1] == '-e'):
            pars = 2
            just_exit = True

    if (len(argv) > pars):
        filename = argv[pars]

    # parse JSON
    f = open(filename)
    data = json.load(f)
    f.close()

    # convert to python objects
    test_suite = TestSuite(data)

    if (just_exit):
        retval = test_suite.retval();
        _exit(retval);

    if (summary):
        test_suite.html_summary(link)
    else:
        test_suite.html()

if __name__ == '__main__':
    main()
