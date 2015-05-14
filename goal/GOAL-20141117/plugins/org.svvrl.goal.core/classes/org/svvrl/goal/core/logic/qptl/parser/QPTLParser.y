/*
 * Description: The parser for QPTL formula
 * Author: M.H. Tsai <minghsientsai@ntu.edu.tw>
 */

%{
	import java.text.ParseException;
	import java.io.IOException;
	import java.io.Reader;
	import java.io.StringReader;
	import logic.Proposition;
	import logic.temporal.qptl.QPTL;
	import logic.temporal.qptl.QPTLLexer;

%}

%token TT FF
%token LPAR RPAR COLON
%token PROP
%token OP_NEG OP_AND OP_OR OP_IMP OP_IFF
%token OP_X OP_F OP_G OP_U OP_W OP_R
%token OP_Y OP_Z OP_O OP_H OP_S OP_B
%token OP_A OP_E

%right OP_IMP
%left OP_IFF
%left OP_OR
%left OP_AND
%nonassoc OP_U OP_W OP_R OP_S OP_B
%nonassoc OP_X OP_F OP_G OP_Y OP_Z OP_O OP_H
%nonassoc OP_NEG
%nonassoc OP_A OP_E

%%

qptl:
OP_A PROP COLON qptl
	{
		Proposition q = new Proposition($2.sval);
		QPTL f = check((QPTL) $4.obj);
		$$.obj = QPTL.A(q, f);
	}
| OP_E PROP COLON qptl
	{
		Proposition q = new Proposition($2.sval);
		QPTL f = check((QPTL) $4.obj);
		$$.obj = QPTL.E(q, f);
	}
| qptl_binary
	{
		$$ = $1;
	}
;

qptl_binary:
qptl_binary OP_IMP qptl_unary
	{
		QPTL f1 = check((QPTL) $1.obj);
		QPTL f2 = check((QPTL) $3.obj);
		$$.obj = QPTL.imp(f1, f2);
	}
| qptl_binary OP_IFF qptl_unary
	{
		QPTL f1 = check((QPTL) $1.obj);
		QPTL f2 = check((QPTL) $3.obj);
		$$.obj = QPTL.iff(f1, f2);
	}
| qptl_binary OP_OR qptl_unary
	{
		QPTL f1 = check((QPTL) $1.obj);
		QPTL f2 = check((QPTL) $3.obj);
		$$.obj = QPTL.or(f1, f2);
	}
| qptl_binary OP_AND qptl_unary
	{
		QPTL f1 = check((QPTL) $1.obj);
		QPTL f2 = check((QPTL) $3.obj);
		$$.obj = QPTL.and(f1, f2);
	}
| qptl_binary OP_U qptl_unary
	{
		QPTL f1 = check((QPTL) $1.obj);
		QPTL f2 = check((QPTL) $3.obj);
		$$.obj = QPTL.U(f1, f2);
	}
| qptl_binary OP_W qptl_unary
	{
		QPTL f1 = check((QPTL) $1.obj);
		QPTL f2 = check((QPTL) $3.obj);
		$$.obj = QPTL.W(f1, f2);
	}
| qptl_binary OP_R qptl
	{
		QPTL f1 = check((QPTL) $1.obj);
		QPTL f2 = check((QPTL) $3.obj);
		$$.obj = QPTL.R(f1, f2);
	}
| qptl_binary OP_S qptl_unary
	{
		QPTL f1 = check((QPTL) $1.obj);
		QPTL f2 = check((QPTL) $3.obj);
		$$.obj = QPTL.S(f1, f2);
	}
| qptl_binary OP_B qptl_unary
	{
		QPTL f1 = check((QPTL) $1.obj);
		QPTL f2 = check((QPTL) $3.obj);
		$$.obj = QPTL.B(f1, f2);
	}
| qptl_unary
	{
		$$ = $1;
	}
;

qptl_unary:
OP_NEG qptl_unary
	{
		QPTL f = check((QPTL) $2.obj);
		$$.obj = QPTL.neg(f);
	}
| OP_X qptl_unary
	{
		QPTL f = check((QPTL) $2.obj);
		$$.obj = QPTL.X(f);
	}
| OP_F qptl_unary
	{
		QPTL f = check((QPTL) $2.obj);
		$$.obj = QPTL.F(f);
	}
| OP_G qptl_unary
	{
		QPTL f = check((QPTL) $2.obj);
		$$.obj = QPTL.G(f);
	}
| OP_Y qptl_unary
	{
		QPTL f = check((QPTL) $2.obj);
		$$.obj = QPTL.Y(f);
	}
| OP_Z qptl_unary
	{
		QPTL f = check((QPTL) $2.obj);
		$$.obj = QPTL.Z(f);
	}
| OP_O qptl_unary
	{
		QPTL f = check((QPTL) $2.obj);
		$$.obj = QPTL.O(f);
	}
| OP_H qptl_unary
	{
		QPTL f = check((QPTL) $2.obj);
		$$.obj = QPTL.H(f);
	}
| qptl_atomic
	{
		$$ = $1;
	}
;

qptl_atomic:
PROP
	{
		String str = new String($1.sval);
		QPTL f = check(QPTL.atomic(new Proposition(str)));
		$$.obj = f;
	}
| LPAR qptl RPAR
	{
		$$ = $2;
	}
;

%%

/* a reference to the lexer object */
private QPTLLexer lexer;

private QPTL check (QPTL f) throws ParseException {
	if (f == null || f.toString().equals("")) {
		yyerror("Parse error inside");
	}
	return f;
}

/* interface to the lexer */
private int yylex () {
	int yyl_return = -1;
	try {
		yyl_return = lexer.yylex();
	}
	catch (IOException e) {
		System.err.println("IO error :" + e);
	}
	return yyl_return;
}

/* error reporting */
public void yyerror (String error) throws ParseException {
//	System.err.println ("Error: " + error);
	throw new ParseException("", -1);
}

/* lexer is created in the constructor */
public QPTLParser(Reader r) {
	lexer = new QPTLLexer(r, this);
}

public QPTLParser(String str) {
	lexer = new QPTLLexer(new StringReader(str), this);
}
