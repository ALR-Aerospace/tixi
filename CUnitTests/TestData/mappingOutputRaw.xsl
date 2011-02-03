<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  exclude-result-prefixes="xsi">
  
  <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>
  
  <!--Define Variable for toolOutput.xml-->
  <xsl:variable name="toolOutputFile" select="'./ToolOutput/toolOutput.xml'"/>
  
  <!--Copy complete Source (CPACSInitial.xml) to Result (CPACSResult.xml)-->
  <xsl:template match="@* | node()">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

  
  <!--Modify a value of an existing node-->
    <xsl:template match="/configuration/global/pax">
    <pax>  
    <xsl:value-of select="document($toolOutputFile)/toolOutput/data/result1"/>
    </pax>
    </xsl:template>

  
  
</xsl:stylesheet>