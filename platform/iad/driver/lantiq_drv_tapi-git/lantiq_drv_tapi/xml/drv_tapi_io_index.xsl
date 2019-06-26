<?xml version="1.0" encoding="UTF-8"?>
<!--

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
-->
<!--
   \file drv_tapi_io_index.xsl
   Stylesheet to generate list of TAPI ioctl indexes
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" indent="no"/>

<!-- entry -->
<xsl:template match="/ioctls">
   <xsl:apply-templates select="ioctl"/>
</xsl:template>

<!-- generate IOCTL index name:
   <IOCTL name>_IDX
-->
<xsl:template match="ioctl">
   <xsl:text>   </xsl:text>
   <xsl:value-of select="@name"/>
   <xsl:text>_IDX</xsl:text>
   <xsl:if test="not(position()=last())">,&#xA;</xsl:if>
   <xsl:if test="position()=last()"><xsl:text>&#xA;</xsl:text></xsl:if>
</xsl:template>

</xsl:stylesheet>
