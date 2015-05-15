PACKAGE=logic.temporal.qptl
FLEX=java -classpath /opt/jflex/lib/JFlex.jar JFlex.Main -v
YACC=/usr/local/bin/yaccj
JAVACC=/usr/local/bin/javacc

default: javacc

javacc: QPTLParser.ii
	${JAVACC} $<

byaccj: Lexer Parser

Parser: QPTLParser.y
	${YACC} -Jclass=QPTLParser -Jpackage=${PACKAGE} -Jthrows=ParseException -Jnorun $<

Lexer: QPTLLexer.l
	${FLEX} QPTLLexer.l

clean:
	rm -rf QPTLLexer.java* QPTLParser.java QPTLParserVal.java ParseException.java QPTLParserConstants.java QPTLparserTokenManager.java SimpleCharStream.java Token.java TokenMgrError.java
