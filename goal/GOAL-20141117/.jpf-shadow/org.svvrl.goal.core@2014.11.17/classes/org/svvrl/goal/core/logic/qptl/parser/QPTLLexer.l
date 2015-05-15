/*
 * Description: The lexer for QPTL formula
 * Author: M.H. Tsai <minghsientsai@ntu.edu.tw>
 */

package goal.logic.temporal.qptl;

%%

%class QPTLLexer
%unicode
%byaccj
%line
%column

%{
	/* store a reference to the parser object */
	private QPTLParser yyparser;

	/* constructor taking an additional parser object */
	public QPTLLexer(java.io.Reader r, QPTLParser yyparser) {
		this(r);
		this.yyparser = yyparser;
	}
%}


%%

[ \t\n\r]		{}

/* */
"tt"			{ return QPTLParser.TT; }
"ff"			{ return QPTLParser.FF; }

/* boolean connectives */
"~"			{ return QPTLParser.OP_NEG; }
"!"			{ return QPTLParser.OP_NEG; }
"/\\"		{ return QPTLParser.OP_AND; }
"&"			{ return QPTLParser.OP_AND; }
"\\/"		{ return QPTLParser.OP_OR; }
"|"			{ return QPTLParser.OP_OR; }
"-->"		{ return QPTLParser.OP_IMP; }
"<-->"		{ return QPTLParser.OP_IFF; }

/* future temporal operators */
"()"			{ return QPTLParser.OP_X; }
"X"			{ return QPTLParser.OP_X; }
"<>"			{ return QPTLParser.OP_F; }
"F"			{ return QPTLParser.OP_F; }
"[]"			{ return QPTLParser.OP_G; }
"G"			{ return QPTLParser.OP_G; }
"U"			{ return QPTLParser.OP_U; }
"W"			{ return QPTLParser.OP_W; }
"R"			{ return QPTLParser.OP_R; }

/* past temporal operators */
"(-)"		{ return QPTLParser.OP_Y; }
"Y"			{ return QPTLParser.OP_Y; }
"(~)"		{ return QPTLParser.OP_Z; }
"Z"			{ return QPTLParser.OP_Z; }
"<->"		{ return QPTLParser.OP_O; }
"O"			{ return QPTLParser.OP_O; }
"[-]"		{ return QPTLParser.OP_H; }
"H"			{ return QPTLParser.OP_H; }
"S"			{ return QPTLParser.OP_S; }
"B"			{ return QPTLParser.OP_B; }

/* quantifiers */
"A"			{ return QPTLParser.OP_A; }
"E"			{ return QPTLParser.OP_E; }

/* parenthesis */
"("			{ return QPTLParser.LPAR; }
")"			{ return QPTLParser.RPAR; }
":"			{ return QPTLParser.COLON; }

[a-z][a-z0-9_-]*
			{ yyparser.yylval = new QPTLParserVal(yytext());
			  return QPTLParser.PROP; }