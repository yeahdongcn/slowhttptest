/*****************************************************************************
*  Copyright 2011 Victor Agababov, Sergey Shekyan
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
* *****************************************************************************/

/*****
 * Authors: Victor Agababov vagababov@gmail.com
 *          Sergey Shekyan shekyan@gmail.com
 *
 * Slow HTTP attack  vulnerability test tool
 *  http://code.google.com/p/slowhttptest/
 *
 *  class StatsDumper and derived classes help to generate
 *  statistics of the test in CSV and Google Chart Tools
 *  based javascript.
 *****/


#include "slowstats.h"
#include "slowlog.h"
#include <stdarg.h>
#include <string>

using std::string;

namespace {

const char* HTML_HEADER =
"<!-- SlowHTTPTest Analysys chart (c) Sergey Shekyan, Victor Agababov 2011  -->\n \
<html>\n \
  <head>\n \
    <script type=\"text/javascript\" src=\"https://www.google.com/jsapi\"></script>\n \
    <script type=\"text/javascript\">\n \
      google.load(\"visualization\", \"1\", {packages:[\"corechart\"]});\n \
      google.setOnLoadCallback(drawChart);\n \
      function drawChart() {\n \
        var data = new google.visualization.DataTable();\n \
        data.addColumn('string', 'Seconds');\n \
        data.addColumn('number', 'Error');\n \
        data.addColumn('number', 'Closed');\n \
        data.addColumn('number', 'Pending');\n \
        data.addColumn('number', 'Connected');\n \
        data.addRows([\n";

const char* HTML_FOOTER = 
      "        ]);\n \
        var chart = new google.visualization.AreaChart(document.getElementById('chart_div'));\n \
        chart.draw(data, {'width': 400, 'height': 240, 'title': '%s', isStacked: true,\n \
          hAxis: {'title': 'Seconds', 'titleTextStyle': {color: '#FF0000'}},\n \
          vAxis: {'title': 'Connections', 'titleTextStyle': {color: '#FF0000'}}\n \
    });\n \
    }\n \
    </script>\n \
    <title>SlowHTTPTest(tm) Connection Results</title>\n \
  </head>\n \
  <body>\n \
    <div id=\"chart_div\"></div>\n \
  </body>\n \
</html>\n";
}

namespace slowhttptest {

bool StatsDumper::Initialize() {
  file_ = fopen(file_name_.c_str(), "w");
  return file_ != NULL;
}

void StatsDumper::WriteStats(const char* format, ...) {
  CHECK_NOTNULL(file_);
  CHECK_NOTNULL(format);
  // Also must be non-empty.
  check(*format != 0, "Format string cannot be empty");

  PreWrite();
  const string new_format = ModifyFormatString(format);

  va_list va;
  va_start(va, format);
  vfprintf(file_, new_format.c_str(), va);
  va_end(va);
  PostWrite();
  fflush(file_);
}


void StatsDumper::PostWrite() {
  fprintf(file_, "\n");
}

void StatsDumper::WriteString(const char* str) {
  CHECK_NOTNULL(file_);
  CHECK_NOTNULL(str);
  if (*str) {
    fprintf(file_, "%s", str);
  }
}

CSVDumper::CSVDumper(const string& file_name, const string& header)
    : StatsDumper(file_name),
      header_(header){
}

CSVDumper::CSVDumper(const string& file_name)
    : StatsDumper(file_name) {
}

bool CSVDumper::Initialize() {
  if (StatsDumper::Initialize()) {
    WriteString(header_.c_str());
    return true;
  }
  return false;
}

HTMLDumper::HTMLDumper(const std::string& file_name, const char* test_info)
    : StatsDumper(file_name),
      test_info_(test_info){
}

bool HTMLDumper::Initialize() {
  if (StatsDumper::Initialize()) {
    WriteHeader();
    return true;
  }
  return false;
}

HTMLDumper::~HTMLDumper() {
  if (IsOpen()) {
    WriteFooter();
  }
}

void StatsDumper::WriteFormattedString(const char* fmt, const char* str) {
  CHECK_NOTNULL(file_);
  CHECK_NOTNULL(str);
  CHECK_NOTNULL(fmt);
  if (*str) {
    fprintf(file_, fmt, str);
  }
}

void HTMLDumper::WriteHeader() {
  WriteString(HTML_HEADER);
}

void HTMLDumper::WriteFooter() {
  WriteFormattedString(HTML_FOOTER, test_info_.c_str());
}

void HTMLDumper::PreWrite() {
  WriteString("[");
}

void HTMLDumper::PostWrite() {
  WriteString("],\n");
}

string HTMLDumper::ModifyFormatString(const char* format) {
  string new_format(format);
  string::size_type pos = new_format.find('%');
  if (pos != string::npos) {
    // There must be something after the first %.
    check(new_format.size() > pos + 1, "Incorrect format specification");
    new_format.insert(pos + 2, 1, '\'');
    new_format.insert(pos, 1, '\'');
  }
  return new_format;
}

}  // namespace slowhttptest
