<?xml version="1.0" encoding="UTF-8"?>
<!--

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
-->
<!--
   \file drv_tapi_io_handler_type.xsl
   Stylesheet to generate list of TAPI ioctl handler types
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" indent="no"/>

<!-- entry -->
<xsl:template match="/ioctls">
   <xsl:apply-templates select="ioctl"/>
</xsl:template>

<!-- generate entry for ioctl handler type -->
<xsl:template match="ioctl">
   <xsl:if test="handler and argument and argument[@kind = 'pointer']">
      <xsl:text>   </xsl:text>
      <xsl:choose>
         <xsl:when test="handler[@kind = 'low-level']">
            <xsl:call-template name="ll-context"/>
         </xsl:when>
         <xsl:when test="handler[@kind = 'high-level']">
            <xsl:call-template name="hl-context"/>
         </xsl:when>
         <xsl:otherwise>
            <xsl:message terminate="yes">ERROR: Incorrect handler kind.</xsl:message>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:text> (</xsl:text>
      <xsl:if test="not(normalize-space(argument))">
         <xsl:message terminate="yes">ERROR: pointer argunent require type definition.</xsl:message>
      </xsl:if>
      <xsl:value-of select="normalize-space(argument)"/>
      <xsl:text>*);&#xA;</xsl:text>
   </xsl:if>
</xsl:template>

<!-- generate macro name by context name for high-level handler -->
<xsl:template name="hl-context">
   <xsl:choose>
      <xsl:when test="not(context)">TAPI_HANDLER_ALLOW</xsl:when>
      <xsl:when test="context = 'driver'">TAPI_DRV_HANDLER_ALLOW</xsl:when>
      <xsl:when test="context = 'device'">TAPI_DEV_HANDLER_ALLOW</xsl:when>
      <xsl:when test="context = 'channel'">TAPI_CH_HANDLER_ALLOW</xsl:when>
      <xsl:otherwise>
         <xsl:message terminate="yes">ERROR: Incorrect context.</xsl:message>
      </xsl:otherwise>
   </xsl:choose>
</xsl:template>

<!-- generate macro name by context name for low-level handler -->
<xsl:template name="ll-context">
   <xsl:choose>
      <xsl:when test="context = 'device'">TAPI_LL_DEV_HANDLER_ALLOW</xsl:when>
      <xsl:when test="context = 'channel'">TAPI_LL_CH_HANDLER_ALLOW</xsl:when>
      <xsl:otherwise>
         <xsl:message terminate="yes">ERROR: Incorrect context.</xsl:message>
      </xsl:otherwise>
   </xsl:choose>
</xsl:template>

</xsl:stylesheet>
