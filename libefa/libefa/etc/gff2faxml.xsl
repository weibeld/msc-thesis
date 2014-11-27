<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		version="1.0">

  <xsl:output method="xml" indent="yes" encoding="UTF-8" />
  <xsl:strip-space elements="from read stateID symbol to" />
  <xsl:preserve-space elements="acc alphabet initialStateSet state stateSet transitionSet transition" />

  <xsl:template match="/structure">
    <faxml version="0.3">
      <automaton class="non-deterministic" minimal="false">
	<xsl:if test="acc/@type = 'buchi'">
	  <xsl:attribute name="type">buechi</xsl:attribute>
	</xsl:if>
	<xsl:attribute name="name">
	  <xsl:value-of select="$filename" />
	</xsl:attribute>
	<xsl:apply-templates select="alphabet" />
	<xsl:apply-templates select="stateSet" />
      </automaton>
    </faxml>
  </xsl:template>

  <xsl:template match="alphabet">
    <alphabet>
      <xsl:for-each select="symbol">
	<xsl:value-of select="." />
      </xsl:for-each>
    </alphabet>
  </xsl:template>

  <xsl:template match="stateSet">
    <states>
      <xsl:for-each select="state">
	<state>
	  <xsl:variable name="stid" select="@sid" />
	  <xsl:attribute name="name">
	    <xsl:value-of select="$stid" />
	  </xsl:attribute>
	  <xsl:for-each select="../../initialStateSet/stateID">
	    <xsl:if test=". = $stid">
	      <xsl:attribute name="initial">true</xsl:attribute>
	    </xsl:if>
	  </xsl:for-each>
	  <xsl:for-each select="../../acc/stateID">
	    <xsl:if test=". = $stid">
	      <xsl:attribute name="accepting">true</xsl:attribute>
	    </xsl:if>
	  </xsl:for-each>
	  <transitions>
	    <xsl:for-each select="../../transitionSet/transition">
	      <xsl:if test="from = $stid">
		<transition>
		  <xsl:attribute name="label">
		    <xsl:value-of select="read" />
		  </xsl:attribute>
		  <xsl:attribute name="to">
		    <xsl:value-of select="to" />
		  </xsl:attribute>
		</transition>
	      </xsl:if>
	    </xsl:for-each>
	  </transitions>
	</state>
      </xsl:for-each>
    </states>
  </xsl:template>

</xsl:stylesheet>
