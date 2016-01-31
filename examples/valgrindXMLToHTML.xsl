<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- output method
       we must use indent="no" here since we are creating javascript string which can't be multiline string -->
  <xsl:output method="html" encoding="UTF-8" indent="no"/>

  <!-- The html header -->
  <xsl:template match="/valgrindoutput">
    <xsl:text disable-output-escaping='yes'>&lt;!DOCTYPE html>
</xsl:text>
    <html lang="en">
      <head>
        <title>Valgrind Report: <xsl:value-of select="tool/text()"/></title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <link rel="stylesheet" type="text/css" href="https://cdn.datatables.net/s/bs-3.3.5/jq-2.1.4,dt-1.10.10/datatables.min.css"/>
        <link rel="shortcut icon" href="data:image/x-icon;," type="image/x-icon"/>
        <script type="text/javascript" src="https://cdn.datatables.net/s/bs-3.3.5/jq-2.1.4,dt-1.10.10/datatables.min.js"> </script>
        <script type="text/javascript">
          $(document).ready(function() {
            // initialize the error table
            $("#ErrorTable").dataTable({
              'lengthMenu': [ [10, 20, 50, 100, 200, 500, -1], [10, 20, 50, 100, 200, 500, 'All'] ],
              'pageLength': 50,
              'aaSorting': [],
              'stateSave': true,
              // we use javascript source for this table due to performance reasons
              'data': ErrorTable_data,
              'columns': [
                { data: 'd0' },
                { data: 'd1' }
              ],
              "rowCallback": function(row, data) {
                $(row).children("td").eq(0).addClass(data["c0"]);
                $(row).children("td").eq(1).addClass(data["c1"]);
              }
            });
            // initialize the stack location table
            $(".ErrorStackTable").dataTable({
              "language": {
                "lengthMenu": "Show _MENU_ stack locations in order",
              },
              'lengthMenu': [ [1, 2, 3, 5, 10, 20, -1], [1, 2, 3, 5, 10, 20, 'All'] ],
              'pageLength': 3,
              'bSort' : false,
              'stateSave': true
            });
            $('#ErrorTable').DataTable().columns.adjust().draw();
            $('.ErrorStackTable').DataTable().columns.adjust().draw();
          });
        </script>
      </head>
      <body style="margin:1em">
        <h1>Valgrind Report: <xsl:value-of select="tool/text()"/></h1>
        <h2>Call Information</h2>
        <dl class="dl-horizontal">
          <dt>Analysed Program</dt>
          <dd><code>
            <!-- the program being run including its arguments -->
            <xsl:call-template name="replace-string"><!-- add possible word breaks after each / character -->
              <xsl:with-param name="text" select="args/argv/exe/text()"/>
              <xsl:with-param name="replace" select="'/'" />
              <xsl:with-param name="with" select="'/&#8203;'"/>
            </xsl:call-template>
            <xsl:apply-templates select="args/argv/arg"/>
          </code></dd>
          <dt>Valgrind</dt>
          <dd><code>
            <!-- the program being run including its arguments -->
            <xsl:call-template name="replace-string"><!-- add possible word breaks after each / character -->
              <xsl:with-param name="text" select="args/vargv/exe/text()"/>
              <xsl:with-param name="replace" select="'/'" />
              <xsl:with-param name="with" select="'/&#8203;'"/>
            </xsl:call-template>
            <xsl:apply-templates select="args/vargv/arg"/>
          </code></dd>
        </dl>
        <h2>Errors</h2>
        <!-- a table of all errors: 1 column: error type; 2 column error details which is itself a table -->
        <table id="ErrorTable" class="table table-bordered">
          <thead><tr><th>Error Type</th><th>Error Details</th></tr></thead>
          <tbody>
            <script type="text/javascript">
              var ErrorTable_data=[
                <xsl:apply-templates select="error"/>
              ];
            </script>
          </tbody>
        </table>
        <hr/>
        <span class="pull-left small">
          <a href="www.mbsim-env.de/mbsim/html/impressum_disclaimer_datenschutz.html#impressum">Impressum</a> /
          <a href="www.mbsim-env.de/mbsim/html/impressum_disclaimer_datenschutz.html#disclaimer">Disclaimer</a> /
          <a href="www.mbsim-env.de/mbsim/html/impressum_disclaimer_datenschutz.html#datenschutz">Datenschutz</a>
        </span>
        <span class="pull-right small">
          Generated by valgrindXMLToHTML.xsl
          <a href="http://validator.w3.org/check?uri=referer">
            <img src="https://www.w3.org/Icons/valid-html401-blue.png" alt="Valid HTML"/>
          </a>
        </span>
      </body>
    </html>
  </xsl:template>

  <!-- argument of the program being run, seperated by spaces -->
  <xsl:template match="arg">
    <xsl:text> </xsl:text>
    <xsl:call-template name="replace-string"><!-- add possible word breaks after each / character -->
      <xsl:with-param name="text" select="text()"/>
      <xsl:with-param name="replace" select="'/'" />
      <xsl:with-param name="with" select="'/&#8203;'"/>
    </xsl:call-template>
  </xsl:template>

  <!-- one row of the error table -->
  <xsl:template match="error">
    {
      <!-- d<colIndex> == data for column <colIndex>
           c<colIndex> == class attribute for column <colIndex> -->
      <!-- error type -->
      "d0":'<h3><xsl:value-of select="kind/text()"/></h3>
        <!-- suppression raw text, if existing -->
        <xsl:if test="suppression/rawtext">
          <div class="btn-group btn-group-sm">
            <button type="button" class="btn btn-info dropdown-toggle" data-toggle="dropdown">Show suppression <span class="caret"></span></button>
            <pre class="dropdown-menu" role="menu">
              <xsl:call-template name="replace-string"><!-- convert all newline char to \n (this must be a single line javascript string) -->
                <xsl:with-param name="text" select="suppression/rawtext/text()"/>
                <xsl:with-param name="replace" select="'&#10;'" />
                <xsl:with-param name="with" select="'\n'"/>
              </xsl:call-template>
            </pre>
          </div>
        </xsl:if>',
      "c0":'<xsl:choose>
              <xsl:when test="kind/text()='Leak_DefinitelyLost' or kind/text()='Leak_IndirectlyLost' or kind/text()='Leak_PossiblyLost' or kind/text()='Leak_StillReachable'">warning</xsl:when>
              <xsl:otherwise>danger</xsl:otherwise>
            </xsl:choose>',
      <!-- error details -->
      "d1":'<!-- location stack of the valgrind "what" message --><h4 class="text-danger">
          <xsl:call-template name="replace-string"><!-- quote ' to \' (this is a javascript string) -->
            <xsl:with-param name="text" select="what/text()"/>
            <xsl:with-param name="replace" select="&quot;'&quot;" />
            <xsl:with-param name="with" select="&quot;\'&quot;"/>
          </xsl:call-template>
        </h4>
        <xsl:apply-templates select="what/following-sibling::stack[position()=1]"/>
        <!-- location stack of the valgrind "auxwhat" message, if existing -->
        <xsl:if test="auxwhat">
          <h4 class="text-danger">
            <xsl:call-template name="replace-string"><!-- quote ' to \' (this is a javascript string) -->
              <xsl:with-param name="text" select="auxwhat/text()"/>
              <xsl:with-param name="replace" select="&quot;'&quot;" />
              <xsl:with-param name="with" select="&quot;\'&quot;"/>
            </xsl:call-template>
          </h4>
          <xsl:apply-templates select="auxwhat/following-sibling::stack[position()=1]"/>
        </xsl:if>
        <!-- location stack of the valgrind "xwhat" message, if existing -->
        <xsl:if test="xwhat">
          <h4 class="text-danger">
            <xsl:call-template name="replace-string"><!-- quote ' to \' (this is a javascript string) -->
              <xsl:with-param name="text" select="xwhat/text/text()"/>
              <xsl:with-param name="replace" select="&quot;'&quot;" />
              <xsl:with-param name="with" select="&quot;\'&quot;"/>
            </xsl:call-template>
          </h4>
          <xsl:apply-templates select="xwhat/following-sibling::stack[position()=1]"/>
        </xsl:if>',
      "c1":''
    },
  </xsl:template>

  <!-- a table for the location stack -->
  <xsl:template match="stack">
    <table class="ErrorStackTable table table-striped table-hover table-bordered table-condensed">
      <thead><tr><th>File:Line</th><th>Function Name</th><th>Library</th></tr></thead>
      <tbody>
        <!-- each stack entry is a table row -->
        <xsl:apply-templates select="frame"/>
      </tbody>
    </table>
  </xsl:template>

  <!-- one row of the location stack table -->
  <xsl:template match="frame">
    <!-- skip rows without a function name: these are very OS internal -->
    <xsl:if test="fn">
      <tr>
        <td>
          <a>
            <xsl:attribute name="href">
              <xsl:value-of select="dir/text()"/>/<xsl:value-of select="file/text()"/>
            </xsl:attribute>
            <!-- directory of the file of the stack location -->
            <xsl:call-template name="replace-string"><!-- add possible word breaks after each / character -->
              <xsl:with-param name="text" select="dir/text()"/>
              <xsl:with-param name="replace" select="'/'" />
              <xsl:with-param name="with" select="'/&#8203;'"/>
            </xsl:call-template>
            <!-- file of the stack location -->
            <xsl:if test="dir"><xsl:text>/&#8203;</xsl:text></xsl:if>
            <xsl:value-of select="file/text()"/>
          </a>
          <xsl:if test="line">:<xsl:value-of select="line/text()"/></xsl:if>
        </td>
        <!-- function name of the stack location -->
        <td>
          <xsl:call-template name="replace-string"><!-- add possible word breaks after each :: character -->
            <xsl:with-param name="text" select="fn/text()"/>
            <xsl:with-param name="replace" select="'::'" />
            <xsl:with-param name="with" select="'::&#8203;'"/>
          </xsl:call-template>
        </td>
        <!-- library of the stack location -->
        <td>
          <xsl:call-template name="replace-string"><!-- add possible word breaks after each / character -->
            <xsl:with-param name="text" select="obj/text()"/>
            <xsl:with-param name="replace" select="'/'" />
            <xsl:with-param name="with" select="'/&#8203;'"/>
          </xsl:call-template>
        </td>
      </tr>
    </xsl:if>
  </xsl:template>

  <!-- Helper function: replace text in string
  this must be called e.g. with
  <xsl:call-template name="replace-string">
    <xsl:with-param name="text" select="'aa::bb::cc'"/>
    <xsl:with-param name="replace" select="'::'" />
    <xsl:with-param name="with" select="','"/>
  </xsl:call-template>
  -->
  <xsl:template name="replace-string">
    <xsl:param name="text"/>
    <xsl:param name="replace"/>
    <xsl:param name="with"/>
    <xsl:choose>
      <xsl:when test="contains($text, $replace)">
        <xsl:value-of select="substring-before($text, $replace)"/>
        <xsl:value-of select="$with"/>
        <xsl:call-template name="replace-string">
          <xsl:with-param name="text" select="substring-after($text, $replace)"/>
          <xsl:with-param name="replace" select="$replace"/>
          <xsl:with-param name="with" select="$with"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$text"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
