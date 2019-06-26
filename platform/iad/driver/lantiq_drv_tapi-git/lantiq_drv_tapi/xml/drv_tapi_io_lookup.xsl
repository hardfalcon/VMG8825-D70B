<?xml version="1.0" encoding="UTF-8"?>
<!--

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
-->
<!--
   \file drv_tapi_io_lookup.xsl
   Stylesheet to generate lookup table of TAPI ioctls
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<!-- entry -->
<xsl:template match="/ioctls">
   <xsl:apply-templates select="ioctl"/>
</xsl:template>

<!-- generate feature condition
   #if <feature>
   <...>
   #endif /* <feature> */
-->
<xsl:template name="features">
   <xsl:param name="position">start</xsl:param>
   <xsl:if test="features and features/feature">
      <xsl:if test="$position = 'start'">
         <xsl:text>#if </xsl:text>
         <xsl:apply-templates select="features/feature"/>
         <xsl:text>&#xA;</xsl:text>
      </xsl:if>
      <xsl:if test="$position = 'end'">
         <xsl:text>#endif /* </xsl:text>
         <xsl:apply-templates select="features/feature"/>
         <xsl:text> */&#xA;</xsl:text>
      </xsl:if>
   </xsl:if>
</xsl:template>

<!-- generate feature detection
   defined(feature) &&
-->
<xsl:template match="feature">
   <xsl:if test="not(normalize-space(.))">
      <xsl:message terminate="yes">ERROR: Empty feature.</xsl:message>
   </xsl:if>
   <xsl:text disable-output-escaping="yes">defined(</xsl:text>
   <xsl:value-of select="."/>
   <xsl:if test="not(position()=last())">
      <xsl:text disable-output-escaping="yes">) &amp;&amp; </xsl:text></xsl:if>
   <xsl:if test="position()=last()">)</xsl:if>
</xsl:template>

<!-- generate IOCTL lookup entry:
   IOCTL_LKUP_TBL_ADD<IOCTL name>
-->
<xsl:template match="ioctl">
   <xsl:if test="handler">
      <xsl:call-template name="features">
         <xsl:with-param name="position">start</xsl:with-param>
      </xsl:call-template>
      <xsl:text>   IOCTL_LKUP_TBL_ADD (</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>),&#xA;</xsl:text>
      <xsl:call-template name="features">
         <xsl:with-param name="position">end</xsl:with-param>
      </xsl:call-template>
   </xsl:if>
</xsl:template>

</xsl:stylesheet>
