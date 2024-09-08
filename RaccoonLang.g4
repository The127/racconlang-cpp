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
    ;

enumDeclaration
    : 'pub'? 'enum' Identifier genericParams constraintDeclaration* '{' enumMemberDeclaration* '}'
    ;

enumMemberDeclaration
    : Identifier '(' typeList ')' ','
    ;

interfaceDeclaration
    : 'pub'? 'interface' Identifier genericParams ('require' typeNameList)? constraintDeclaration* (';' | '{' interfaceMethodDeclaration '}')
    ;

interfaceMethodDeclaration
    : 'pub'? 'fn' Identifier genericParams '(' namedTypeList ')' ('->' type)? constraintDeclaration* ';'
    ;

structDeclaration
    : 'pub'? 'struct' Identifier genericParams (';' | shortStructBody | longStructBody)
    ;

shortStructBody
    : '(' namedTypeList ')' constraintDeclaration*
    ;

longStructBody
    : '{' propertyDeclaration* '}' constraintDeclaration* (structDestructure ';')?
    ;

propertyDeclaration
    : 'pub'? Identifier ':' type ';'
    ;

structDestructure
    : 'destructures' 'into' '(' identifierList ')'
    ;

functionDeclaration
    : 'pub'? 'fn' Identifier '(' namedTypeList ')' ('->' type)? constraintDeclaration* (methodBody | lambdaBody)
    ;

methodBody
    : '{' '}'
    ;

lambdaBody
    : '=>' ';'
    ;

aliasDeclaration
    : 'pub'? 'alias' Identifier genericParams? constraintDeclaration* '=' type ';'
    ;

moduleVariableDeclaration
    : 'pub'? 'let' Identifier ':' type ';'
    ;

constraintDeclaration
    : 'require' Identifier ':' genericConstraint (',' genericConstraint)* ','?
    ;

genericConstraint
    : interfaceConstraint
    ;

interfaceConstraint
    : typeName
    ;

genericParams
    : 'of' '(' identifierList ')'
    ;

identifierList
    : Identifier (',' Identifier) ','?
    ;

typeList
    : type (',' type)* ','?
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
    : path genericParams?
    ;

tupleType
    : '(' typeList ')'
    ;

fnType
    : 'fn' genericParams? '(' typeList ')' ('->' type)? constraintDeclaration*
    ;

namedTypeList
    : Identifier ':' type (Identifier ':' type)* ','?
    ;

Identifier
    : 'Identifier'
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