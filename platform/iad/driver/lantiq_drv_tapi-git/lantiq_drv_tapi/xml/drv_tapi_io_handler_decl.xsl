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
   <xsl:if test="handler">
      <xsl:text>IFX_int32_t </xsl:text>
      <xsl:apply-templates select="handler"/>
      <xsl:text> (</xsl:text>
      <xsl:if test="context">
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
      </xsl:if>
      <xsl:if test="argument">
         <xsl:if test="context">
            <xsl:text>, </xsl:text>
         </xsl:if>
         <xsl:apply-templates select="argument"/>
      </xsl:if>
      <xsl:if test="not(context) and not(argument)">
         <xsl:text>void</xsl:text>
      </xsl:if>
      <xsl:text>)&#xA;</xsl:text>
   </xsl:if>
</xsl:template>

<!-- generate handler call -->
<xsl:template match="handler">
   <xsl:if test="not(normalize-space(.))">
      <xsl:message terminate="yes">ERROR: Empty handler.</xsl:message>
   </xsl:if>
   <xsl:choose>
      <xsl:when test="@kind = 'high-level'">
         <xsl:value-of select="."/>
      </xsl:when>
      <xsl:when test="@kind = 'low-level'">
         <xsl:text>(*</xsl:text>
         <xsl:if test="not(contains(.,'.'))">
            <xsl:value-of select="."/>
         </xsl:if>
         <xsl:if test="contains(.,'.')">
            <xsl:value-of select="substring-after(.,'.')"/>
         </xsl:if>
         <xsl:text>)</xsl:text>
      </xsl:when>
      <xsl:otherwise>
         <xsl:message terminate="yes">ERROR: Incorrect handler kind.</xsl:message>
      </xsl:otherwise>
   </xsl:choose>
</xsl:template>

<!-- generate argument kind name -->
<xsl:template match="argument">
   <xsl:choose>
      <xsl:when test="@kind = 'pointer'">
         <xsl:value-of select="normalize-space(.)"/><xsl:text> *</xsl:text>
      </xsl:when>
      <xsl:when test="@kind = 'integer'">IFX_uint32_t </xsl:when>
      <xsl:otherwise>
         <xsl:message terminate="yes">ERROR: Incorrect argument kind.</xsl:message>
      </xsl:otherwise>
   </xsl:choose>
   <xsl:text>variable</xsl:text>
</xsl:template>

<!-- generate macro name by context name for high-level handler -->
<xsl:template name="hl-context">
   <xsl:choose>
      <xsl:when test="context = 'driver'">IFX_TAPI_DRV_CTX_t</xsl:when>
      <xsl:when test="context = 'device'">TAPI_DEV</xsl:when>
      <xsl:when test="context = 'channel'">TAPI_CHANNEL</xsl:when>
      <xsl:otherwise>
         <xsl:message terminate="yes">ERROR: Incorrect context.</xsl:message>
      </xsl:otherwise>
   </xsl:choose>
   <xsl:text> *variable</xsl:text>
</xsl:template>

<!-- generate macro name by context name for low-level handler -->
<xsl:template name="ll-context">
   <xsl:choose>
      <xsl:when test="context = 'device'">IFX_TAPI_LL_DEV_t</xsl:when>
      <xsl:when test="context = 'channel'">IFX_TAPI_LL_CH_t</xsl:when>
      <xsl:otherwise>
         <xsl:message terminate="yes">ERROR: Incorrect context.</xsl:message>
      </xsl:otherwise>
   </xsl:choose>
   <xsl:text> *variable</xsl:text>
</xsl:template>

</xsl:stylesheet>
