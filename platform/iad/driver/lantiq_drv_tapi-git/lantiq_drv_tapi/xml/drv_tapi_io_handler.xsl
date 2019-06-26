<?xml version="1.0" encoding="UTF-8"?>
<!--

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
-->
<!--
   \file drv_tapi_io_handler.xsl
   Stylesheet to generate list of TAPI ioctl handlers
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" indent="no"/>

<!-- entry -->
<xsl:template match="/ioctls">
   <xsl:apply-templates select="ioctl"/>
</xsl:template>

<!-- generate feature condition
   #if <feature>
   <...>
   #else
      TAPI_HANDLER_UNSUPPORTED,
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
         <xsl:text>#else&#xA;</xsl:text>
         <xsl:text>   TAPI_HANDLER_UNSUPPORTED</xsl:text>
         <xsl:if test="not(position()=last())">,</xsl:if>
         <xsl:text>&#xA;#endif /* </xsl:text>
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

<!-- generate handler definition
   /* IOCTL name */ TAPI_HANDLER_ADD (...),
-->
<xsl:template match="ioctl">
   <xsl:if test="handler">
      <xsl:call-template name="features">
         <xsl:with-param name="position">start</xsl:with-param>
      </xsl:call-template>
   </xsl:if>
   <xsl:text>   /* </xsl:text>
   <xsl:value-of select="@name"/>
   <xsl:text> */ </xsl:text>
   <xsl:if test="not(handler)">
      <xsl:text>TAPI_HANDLER_UNSUPPORTED</xsl:text>
      <xsl:if test="not(position()=last())">,&#xA;</xsl:if>
   </xsl:if>
   <xsl:if test="handler">
      <xsl:text>TAPI_HANDLER_ADD (</xsl:text>
      <xsl:call-template name="context"/>
      <xsl:text>, </xsl:text>
      <xsl:call-template name="argument"/>
      <xsl:text>, </xsl:text>
      <xsl:call-template name="states"/>
      <xsl:text>, </xsl:text>
      <xsl:apply-templates select="handler"/>
      <xsl:if test="not(position()=last())">),&#xA;</xsl:if>
      <xsl:if test="position()=last()">)&#xA;</xsl:if>
      <xsl:call-template name="features">
         <xsl:with-param name="position">end</xsl:with-param>
      </xsl:call-template>
   </xsl:if>
</xsl:template>

<!-- generate context name -->
<xsl:template name="context">
   <xsl:choose>
      <xsl:when test="not(context)">TAPI_IOC_CTX_NONE</xsl:when>
      <xsl:when test="context = 'driver'">TAPI_IOC_CTX_DRV</xsl:when>
      <xsl:when test="context = 'device'">TAPI_IOC_CTX_DEV</xsl:when>
      <xsl:when test="context = 'channel'">TAPI_IOC_CTX_CH</xsl:when>
      <xsl:otherwise>
         <xsl:message terminate="yes">ERROR: Incorrect context.</xsl:message>
      </xsl:otherwise>
   </xsl:choose>
</xsl:template>

<!-- generate argument kind name -->
<xsl:template name="argument">
   <xsl:choose>
      <xsl:when test="not(argument)">TAPI_ARG_NONE</xsl:when>
      <xsl:when test="argument[@kind = 'pointer']">TAPI_ARG_POINTER</xsl:when>
      <xsl:when test="argument[@kind = 'integer']">TAPI_ARG_INTEGER</xsl:when>
      <xsl:otherwise>
         <xsl:message terminate="yes">ERROR: Incorrect argument kind.</xsl:message>
      </xsl:otherwise>
   </xsl:choose>
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
      <xsl:when test="@kind = 'low-level'">TAPI_LL_HANDLER (<xsl:value-of select="."/>)</xsl:when>
      <xsl:otherwise>
         <xsl:message terminate="yes">ERROR: Incorrect handler kind.</xsl:message>
      </xsl:otherwise>
   </xsl:choose>
</xsl:template>

<!-- generate device state requirement -->
<xsl:template name="states">
   <xsl:if test="states and states/state">
      <xsl:apply-templates select="states/state"/>
   </xsl:if>
   <xsl:if test="not(states and states/state)">
      <!-- default value -->
      <xsl:choose>
         <xsl:when test="context = 'channel'">
            <xsl:text>TAPI_DS_INIT</xsl:text>
         </xsl:when>
         <xsl:otherwise>
            <xsl:text>TAPI_DS_NONE</xsl:text>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:if>
</xsl:template>

<!-- generate state detection
   state | ...
-->
<xsl:template match="state">
   <xsl:if test="not(normalize-space(.))">
      <xsl:message terminate="yes">ERROR: Empty state.</xsl:message>
   </xsl:if>
   <xsl:value-of select="."/>
   <xsl:if test="not(position()=last())"> | </xsl:if>
</xsl:template>

</xsl:stylesheet>
