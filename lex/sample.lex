%{
#include <stdio.h>
%}

L [a-zA-Z]
D [0-9]

%%
{L}({L}|{D})* 				printf("	id 	 : %s\n", yytext);
[a-zA-Z_][a-zA-Z0-9_]*			printf("	C id : %s\n", yytext);
[+-]?[0-9]+ 				printf("integer  : %s\n", yytext);
[0-9]+"."[0-9]+(e[+-]?[0-9]+)? 		printf("real 	 : %s\n", yytext);
\"([^\042\134]|"\\"(.|[\n]))*\" 	printf("string	 : %s\n", yytext);
"/*"([^*])*+"*/"	 		printf(" text comment : /* ... */\n");
"//".* 					printf(" line comment : // ... \n");
"\n" |
. ;

%%

int yywrap()
{
	return 1;
}
void main()
{
	yylex();
}
