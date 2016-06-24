<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">
    <!--
    Turn markup like this:

    <code xref="bson:bson_t">bson_t</code>

    ... into a link like this:

    http://api.mongodb.org/libbson/current/bson_t.html
    -->
    <xsl:template name="mal.link.target.custom">
        <xsl:param name="node" select="."/>
        <xsl:param name="xref" select="$node/@xref"/>
        <xsl:if test="starts-with($xref, 'bson:')">
            <xsl:variable name="ref"
                          select="substring-after($xref, 'bson:')"/>
            <xsl:text>http://api.mongodb.org/libbson/current/</xsl:text>
            <xsl:value-of select="$ref"/>
            <xsl:text>.html</xsl:text>
        </xsl:if>
    </xsl:template>
</xsl:stylesheet>
