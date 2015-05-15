JAVACC=/usr/bin/javacc

default: javacc

javacc: CMDParser.ii
	${JAVACC} $<

clean:
	rm -rf *.java
