<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- Output as HTML -->
  <xsl:output method="html" encoding="UTF-8" indent="yes"/>

  <!-- Template for the root element -->
  <xsl:template match="/">
    <html>
      <head>
        <title>Google Test Report</title>
        <style type="text/css">
          body { font-family: Arial, sans-serif; margin: 20px; }
          table { border-collapse: collapse; width: 100%; margin-bottom: 20px; }
          th, td { border: 1px solid #ddd; padding: 8px; }
          th { background-color: #f2f2f2; text-align: left; }
          h1, h2 { color: #333; }
          .summary { margin-bottom: 20px; }
          .testsuite { margin-bottom: 40px; }
        </style>
      </head>
      <body>
        <h1>Google Test Report</h1>

        <!-- Overall summary from the root testsuites element -->
        <div class="summary">
          <h2>Overall Summary</h2>
          <p>
            Total tests: <strong><xsl:value-of select="/testsuites/@tests"/></strong><br/>
            Failures: <strong><xsl:value-of select="/testsuites/@failures"/></strong><br/>
            Disabled: <strong><xsl:value-of select="/testsuites/@disabled"/></strong><br/>
            Errors: <strong><xsl:value-of select="/testsuites/@errors"/></strong><br/>
            Total time: <strong><xsl:value-of select="/testsuites/@time"/></strong> seconds<br/>
            Timestamp: <strong><xsl:value-of select="/testsuites/@timestamp"/></strong>
          </p>
        </div>

        <!-- Process each testsuite element -->
        <xsl:for-each select="/testsuites/testsuite">
          <div class="testsuite">
            <h2>
              <xsl:value-of select="@name"/>
            </h2>
            <p>
              Tests: <strong><xsl:value-of select="@tests"/></strong> |
              Failures: <strong><xsl:value-of select="@failures"/></strong> |
              Disabled: <strong><xsl:value-of select="@disabled"/></strong> |
              Skipped: <strong><xsl:value-of select="@skipped"/></strong> |
              Errors: <strong><xsl:value-of select="@errors"/></strong> |
              Time: <strong><xsl:value-of select="@time"/></strong> seconds |
              Timestamp: <strong><xsl:value-of select="@timestamp"/></strong>
            </p>

            <!-- Table of test cases within this testsuite -->
            <table>
              <tr>
                <th>Test Case Name</th>
                <th>Value Param</th>
                <th>File</th>
                <th>Line</th>
                <th>Status</th>
                <th>Result</th>
                <th>Time (s)</th>
                <th>Timestamp</th>
                <th>Class Name</th>
              </tr>
              <xsl:for-each select="testcase">
                <tr>
                  <td><xsl:value-of select="@name"/></td>
                  <td><xsl:value-of select="@value_param"/></td>
                  <td><xsl:value-of select="@file"/></td>
                  <td><xsl:value-of select="@line"/></td>
                  <td><xsl:value-of select="@status"/></td>
                  <td><xsl:value-of select="@result"/></td>
                  <td><xsl:value-of select="@time"/></td>
                  <td><xsl:value-of select="@timestamp"/></td>
                  <td><xsl:value-of select="@classname"/></td>
                </tr>
              </xsl:for-each>
            </table>
          </div>
        </xsl:for-each>
      </body>
    </html>
  </xsl:template>

</xsl:stylesheet>
