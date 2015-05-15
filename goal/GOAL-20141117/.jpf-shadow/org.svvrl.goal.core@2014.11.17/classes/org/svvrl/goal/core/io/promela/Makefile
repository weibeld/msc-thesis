PACKAGE=org.svvrl.goal.core.io.promela
JAVACC=/usr/bin/javacc

default: javacc

javacc: PromelaParser.ii
	${JAVACC} $<

clean:
	rm -f PromelaParser.java PromelaParserVal.java ParseException.java PromelaParserConstants.java PromelaParserTokenManager.java SimpleCharStream.java Token.java TokenMgrError.java
