grammar RaccoonLang;

start
    : use* module* EOF
    ;

use
    : 'use' (usePath | useMultiple)
    ;

usePath
    : path ';'
    ;

useMultiple
    : path '::' '{' identifierList '}'
    ;

path
    : '::'? Identifier ('::' Identifier)*
    ;

module
    : 'mod' path ';' topLevelDeclaration*
    ;

topLevelDeclaration
    : enumDeclaration
    | interfaceDeclaration
    | structDeclaration
    | functionDeclaration
    | aliasDeclaration
    | moduleVariableDeclaration
    | implBlock
    ;

enumDeclaration
    : 'pub'? 'enum' Identifier genericParams? constraintDeclaration* '{' enumMemberDeclarationList '}'
    ;

enumMemberDeclarationList
    : enumMemberDeclaration (',' enumMemberDeclaration)* ','?
    ;

enumMemberDeclaration
    : Identifier ('(' typeList ')')?
    ;

interfaceDeclaration
    : 'pub'? 'interface' Identifier genericParams? (':' typeNameList)? constraintDeclaration* '{' interfaceMethodDeclaration '}'
    ;

interfaceMethodDeclaration
    : 'pub'? 'mut'? 'fn' Identifier genericParams? '(' parameterList ')' ('->' returnType)? constraintDeclaration* ';'
    | 'pub'? 'mut'? 'get' Identifier '->' returnType ';'
    | 'pub'? 'mut'? 'set' Identifier '(' parameter ')' ';'
    ;

structDeclaration
    : 'pub'? 'struct' Identifier genericParams?  (shortStructBody | longStructBody)
    ;

shortStructBody
    : '(' parameterList ')' constraintDeclaration*
    ;

longStructBody
    : '{' propertyDeclaration* '}' constraintDeclaration* (structDestructure ';')?
    ;

propertyDeclaration
    : 'pub'? 'mut'? Identifier ':' type ';'
    ;

structDestructure
    : 'destructures' 'into' '(' identifierList ')'
    ;

functionDeclaration
    : 'pub'? 'fn' Identifier genericParams? '(' parameterList ')' ('->' type)? constraintDeclaration* (blockExpression | lambdaBody)
    ;

lambdaBody
    : '=>' ';'
    ;

aliasDeclaration
    : 'pub'? 'alias' Identifier genericParams? '=' type constraintDeclaration* ';'
    ;

moduleVariableDeclaration
    : 'pub'? 'mut'? 'let' Identifier ':' type ';'
    ;

implBlock
    : 'impl' genericParams? (typeName 'on')? typeName genericArguments? ('for' typeName)? constraintDeclaration* '{' (implConstructor | implDestructor | implMethod | implSetter | implGetter)* '}'
    ;

implConstructor
    : 'pub'? Identifier '(' parameterList? ')' blockStatement
    ;

implDestructor
    : '~' Identifier '(' ')' blockStatement
    ;

implMethod
    : 'pub'? 'mut'? 'static'? 'fn' Identifier genericParams '(' ')' ('->' type)  constraintDeclaration* (blockExpression | lambdaBody)
    ;

implSetter
    : 'pub'? 'set' Identifier '(' parameter ')' (blockExpression | lambdaBody)
    ;

implGetter
    : 'pub'? 'mut'? Identifier '(' ')' ('->' type) (blockExpression | lambdaBody)
    ;

constraintDeclaration
    : 'where' Identifier ':' genericConstraint (',' genericConstraint)* ','?
    ;

genericConstraint
    : interfaceConstraint
    ;

interfaceConstraint
    : typeName
    ;

genericParams
    :  '<' identifierList '>'
    ;

genericArguments
    : '<' typeList '>'
    ;

identifierList
    : Identifier (',' Identifier) ','?
    ;

typeList
    : type (',' type)* ','?
    ;

returnType
    : 'mut'? type
    ;

type
    : typeName
    | tupleType
    | fnType
    ;

typeNameList
    : typeName (',' typeName)* ','?
    ;

typeName
    : path genericArguments?
    ;

tupleType
    : '(' typeList ')'
    ;

fnType
    : 'fn' genericArguments? '(' typeList ')' ('->' type)? constraintDeclaration*
    ;

parameterList
    : parameter (',' parameter)* ','?
    ;

parameter
    : ('mut' | 'ref')? Identifier ':' type
    ;

blockStatement
    : '{' '}'
    ;

blockExpression
    : '{' '}'
    ;

Identifier
    : [@] [a-zA-Z_] [a-zA-Z0-9_]*
    ;

// comments and white space -> ignored
BLOCK_COMMENT
	: '/*' .*? '*/' -> channel(HIDDEN)
	;
LINE_COMMENT
	: '//' ~[\n]* -> channel(HIDDEN)
	;
WS
	: [ \t\r\n\f]+ -> channel(HIDDEN)
	;