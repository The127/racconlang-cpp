grammar RaccoonLang_old;

start
    : usings modules EOF
    ;
    
modules
    : module*
    ;
    
usings
    : using*
    ;
    
using
    : isPub='pub'? 'use' (useAll | importedTypes | importedModule | extensions | infix) ';'
    ;
    
useAll
    : modulePath '::' '*'
    ;
    
extensions
    : 'extensions' modulePath
    ;
    
infix
    : 'infix' modulePath
    ;
    
modulePath
    : isRooted='::'? Identifier ('::' Identifier)*
    ;
    
importedModule
    : modulePath ('as' alias=Identifier)?
    ;
    
importedTypes
    : modulePath '::' '{' useName (',' useName)* ','? '}'
    ;
    
useName
    : name=Identifier ('as' alias=Identifier)?
    ;
    
module
    : 'mod' modulePath ';' declarations
    ;    
    
declarations
    : (declaration | implBlock)*
    ;

declaration
    : aliasDeclaration
    | structDeclaration
    | interfaceDeclaration
    | functionDeclaration
    | enumDeclaration
    | annotationDeclaration
    | moduleVariable
    ;
    
moduleVariable
    : annotation* isPub='pub'? isConst='const'? name=Identifier ':' signature? ('=' expression)? ';'
    ;
    
annotation
    : '#' '[' name=modulePath ('(' ( (annotationMember (',' annotationMember)*) ','?)? ')')? ']'
    ;
    
annotationMember
    : name=Identifier ':' annotationMemberValue
    ;
    
annotationMemberValue
    : literal | typeName
    ;
    
annotationDeclaration   
    : 'pub' 'annotation' name=Identifier (';' | '{' annotationMemberDeclaration* '}') 
    ;
    
annotationMemberDeclaration
    : name=Identifier ':' type=annotationMemberType ('=' (annotationMemberValue))? ';'
    ;
    
annotationMemberType
    : keyword=('bool'|'string'|'char'|'u8'|'u16'|'u32'|'u64'|'u128'|'i8'|'i16'|'i32'|'i64'|'i128'|'name')
    ;
 
enumDeclaration
    : annotation* isPub='pub'? isValue='value'? 'enum' name=Identifier genericList? genericConstraint* '{' enumMember* '}'
    ;
    
enumMember
    : annotation* name=Identifier ('(' methodParams ')')? ','
    ;
    
aliasDeclaration
    : annotation* isPub='pub'? 'alias' name=Identifier genericList? genericConstraint* '=' signature ';'
    ;
    
structDeclaration
    : annotation* isPub='pub'? isValue='value'? 'struct' name=Identifier genericList? ((shortStructBody genericConstraint* ';' | genericConstraint* '{' property* '}'  (structDestructure ';')?) | ';') 
    ;
    
shortStructBody
    : '(' methodParams? ')'
    ;
    
structDestructure
    : 'destructures' 'into' '(' Identifier (',' Identifier)* ')'
    ;
        
genericList
    : '<' Identifier (',' Identifier)* '>'
    ;
    
genericConstraint
    : 'where' Identifier ':' constraint (',' constraint)*
    ;
    
constraint
    : typeConstraint
    | newConstraint
    | valueConstraint
    ;
    
newConstraint
    : 'new' '(' typeList? ')'
    ;
    
valueConstraint
    : 'value'
    ;
    
typeConstraint
    : typeName
    ;
    
property
    : annotation* visibility=('pub' | 'init' | 'readonly' | 'const' )? isUnsafe='unsafe'? name=Identifier (':' type=signature)? ('=' expression )?  ';'  
    ;
    
interfaceDeclaration
    : annotation* isPub='pub'? 'interface' name=Identifier genericList? ('require' inheritedList)? genericConstraint* (';' | '{' interfaceMethodDeclaration* '}')
    ;
    
inheritedList
    : typeName (',' typeName)* ','?
    ;
    
interfaceMethodDeclaration
    : annotation* isPub='pub'? 'fn' name=Identifier genericList? '(' methodParams? ')' ('->' returnType=signature)? genericConstraint*  (methodBody | lambdaBody | ';')
    ;
    
methodParams
    : methodParam (',' methodParam)* ','?
    ;
    
methodParam
    : name=Identifier ':' type=signature
    ;
    
implBlock
    : isExtension='extension'? 'impl' genericList? (interfaceType=typeName 'on')? name=typeName ('for' forInterface=typeName)? genericConstraint* implBody 
    ;
    
implBody
    : '{' (constructor | destructor | functionDeclaration)* '}'
    ;
    
constructor
    : annotation* isPub='pub'? ctor=Identifier '(' methodParams? ')' (':' calledCtor=Identifier '(' expressionList ')')? (';' | blockStatement)
    ;
    
destructor
    : annotation* '~' dtor=Identifier '(' ')' blockStatement
    ;
    
functionDeclaration
    : annotation* isPub='pub'? isUnsafe='unsafe'? isInfix='infix'? isStatic='static'? 'fn' name=Identifier genericList? '(' methodParams? ')' ('->' returnType=signature)? genericConstraint* (methodBody | lambdaBody)
    ;
    
methodBody
    : blockExpression
    ;
    
lambdaBody
    : '=>' expression ';'
    ;
    
statement
    : doHandleStatement
    | expressionStatement
    | variableDeclarationStatement
    | assignmentStatement
    | assertStatement
    | blockStatement
    | unsafeBlockStatement
    ;
    
assertStatement
    : 'assert' value=expression ':' print=String ';'
    ; 
    
variableDeclarationStatement
    : isUsing='using'? mut=('let' | 'const') variableTarget (':' signature)? '=' expression ';'
    ;
    
variableTarget
    : binding | destructure
    ;
    
assignmentStatement
    : assignmentTarget op=('=' | '+=' | '-=' | '/=' | '*=' | '&=' | '^⁼' | '|=' ) expression ';'
    ;
    
assignmentTarget
    : isDiscard=Discard 
    | destructure 
    | expression
    ;
    
destructure
    : '(' bindingList ')'
    ;
    
expressionStatement
    : expressionWithoutBlock isStatement=';'
    | expressionWithBlock isStatement=';'?
    ;
    
doHandleStatement
    : 'do' blockStatement 'handle' '{' handleMatchPart* '}'
    ;
    
blockStatement
    : '{' statement* '}'
    ;
    
unsafeBlockStatement
    : 'unsafe' blockStatement
    ;
    
handleMatchPart
    : handleMatch whenCondition? '=>' expression ','
    ;
    
asBinding
    : 'as' binding
    ;
    
handleMatch
    : typeDestructureMatch
    | structMatch 
    ;
    
match
    : literalMatch
    | bindMatch
    | destructureMatch
    | structMatch
    | typeDestructureMatch
    ;
    
whenCondition
    : 'when' expression
    ;
    
structMatch
    :  typeName ('{' structMatchPart (',' structMatchPart)* '}')? asBinding?
    ;
    
structMatchPart
    : propertyName=Identifier ':'  match
    ;
    
destructureMatch
    : '(' match (',' match)* ')'
    ;
    
typeDestructureMatch
    : typeName destructureMatch asBinding?
    ;
    
literalMatch
    : literal
    ;
    
bindMatch
    : binding
    ;
    
loopExpression
    : label=Label? 'loop' blockExpression elsePart?
    ;
    
whileExpression
    : label=Label? 'while' expression blockExpression elsePart?
    ;
    
forInExpression
    : label=Label? 'for' bindingList 'in' in=expression blockExpression elsePart?
    ;

bindingList
    : binding (',' binding)* ','?
    ;

binding
    : valueBinding 
    | discardBinding
    ;
    
valueBinding
    : Identifier
    ;
    
discardBinding
    : Discard
    ;
    
blockExpression
    : '{' statement* value=expression? '}'
    ;
    
ifExpression
    : ifPart elseIfPart* elsePart? 
    ;
    
ifPart
    : 'if' expression blockExpression
    ;
    
elseIfPart
    : 'else' 'if' expression blockExpression
    ;
    
elsePart   
    : 'else' blockExpression
    ;
    
returnExpression
    : 'return' expression?
    ;
    
expressionList
    : expression (',' expression)*
    ;
    
expressionWithBlock
    : ifExpression
    | loopExpression
    | whileExpression
    | forInExpression
    | blockExpression
    | matchExpression
    ;
    
expressionWithoutBlock
    : precedence1Expression
    | newExpr
    | returnExpression
    | runExpression
    | breakExpression
    | continueExpression
    ;
    
sizeofExpr
    : 'sizeof' '(' typeName ')'
    ;
    
nameofExpr
    : 'nameof' '(' expression ')'
    ;
    
matchExpression
    : 'match' value=expression '{'  (matchPart ',')* '}'
    ;
       
matchPart
    : match whenCondition? '=>' expression
    ;
    
expression
    : expressionWithBlock
    | expressionWithoutBlock
    ;
    
runExpression
    : 'run' expression
    ;
    
breakExpression
    : 'break' label=Label? expression?
    ;
    
continueExpression
    : 'continue' label=Label?
    ;
    
precedence1Expression
    : precedence2Expression precedence1Follow*
    ;
    
precedence1Follow
    : op='|>' precedence2Expression
    ;
    
precedence2Expression   
    : raiseExpression | resumeExpression | infixExpression
    ;
    
raiseExpression
    : 'raise' infixExpression
    ;
    
resumeExpression
    : 'resume' 'with' infixExpression
    ;
    
infixExpression
    : precedence3Expression infixFollow*
    ;
    
infixFollow
    : op=Identifier precedence3Expression
    ;
    
precedence3Expression
    : precedence4Expression precedence3Follow*
    ;
    
precedence3Follow
    : op=('||' | '|' | '&&' | '&' | '^') precedence4Expression
    ;
    
precedence4Expression
    : precedence5Expression precedence4Follow*
    ;
    
precedence4Follow 
    : op=('==' | '!=' | '<' | '<=' | '>' | '>=') precedence5Expression
    ;
    
precedence5Expression
    : precedence6Expression precedence5Follow*
    ;
    
precedence5Follow
    : op=('+' | '-') precedence6Expression
    ;
    
precedence6Expression
    : l=precedence6bExpression (op='**' r=precedence6bExpression)?
    ;
    
precedence6bExpression
    : precedence7Expression precedence6bFollow*
    ;
    
precedence6bFollow
    : op=('*' | '/' | '%') precedence7Expression
    ;
    
precedence7Expression
    : op=('+' | '-' | '~' | '!')? precedence8Expression
    ;
    
precedence8Expression
    : precedence9Expression subsequentExpression*
    ;
    
subsequentExpression
    : functionCallExpression
    | withExpression
    | partialFunctionApplicationExpression
    | arrayAccessExpression
    | sliceExpression
    | accessExpression
    ;
    
withExpression
    : 'with' '{' assignmentList '}'
    ;
    
assignmentList
    : assignmentListItem (',' assignmentListItem)* ','?
    ;
    
assignmentListItem
    : name=Identifier '=' expression
    ;
    
lambdaExpression
    : shortLambda
    | longLambda
    ;
    
shortLambda
    : '(' lambdaParams? ')' '=>' expression
    ;
    
longLambda
    : 'fn' '(' lambdaParams? ')' ('->' signature)? blockExpression
    ;
    
lambdaParams
    : lambdaParam (',' lambdaParam)*
    ;
    
lambdaParam
    : name=Identifier (':' signature)?
    ;
    
newExpr
    : 'new' '(' functionArguments? ')'
    ;
    
functionCallExpression
    : '('  functionArguments? ')'
    ;
    
partialFunctionApplicationExpression
    : '(' partialFunctionArguments ')'
    ;
    
functionArguments
    : functionArgument (',' functionArgument)*
    ;
    
functionArgument
    : expression splat='...'?
    ;
    
partialFunctionArguments
    : partialFunctionArgument (',' partialFunctionArgument)*
    ;
    
partialFunctionArgument
    : (expression | '$') splat='...'?
    ;
    
arrayAccessExpression
    : '[' expression ']'
    ;
    
sliceExpression
    : '[' from=expression? ':' to=expression? ']'
    ;
    
accessExpression
    :  '.' name=Identifier turbofish?
    ;
  
precedence9Expression
    : parExpression
    | lambdaExpression
    | tupleExpression
    | intervalExpression
    | literal
    | modulePathExpression
    | sequenceExpression
    | initStructExpression
    | keyWordExpression
    | sizeofExpr
    | nameofExpr
    ;
    
modulePathExpression
    : modulePath genericPart?
    ;
    
keyWordExpression
    : keyword=('self' | 'default')
    ;
    
initStructExpression
    : typeName '{' structMemberInitializer* '}'
    ;
    
structMemberInitializer
    : name=Identifier ':' expression ','
    ;
    
turbofish
    : '::' genericPart
    ;
    
sequenceExpression  
    : '[' functionArguments ','? ']'
    ;
    
parExpression
    : '(' expression ')'
    ;
    
tupleExpression
    : '(' functionArguments ')'
    ;
    
intervalExpression
    : lPar=('(' | '[') from=expression? ';' to=expression? rPar=(')' | ']')
    ;
    
signature
    : typeName
    | tupleType
    | functionType
    ;
    
functionType
    : 'fn' genericList? '(' typeList? ')' ('->' returnType=signature)? genericConstraint* 
    ;
    
typeList
    : typeListItem (',' typeListItem)* ','?
    ;
    
typeListItem
    : (name=Identifier ':')? signature
    ;
    
tupleType
    : '(' tupleMember (',' tupleMember)* ')'
    ;
    
tupleMember
    : (name=Identifier ':')? signature
    ;

typeName
    : modulePath genericPart?
    | keyword=('unit'|'__ptr'|'Self'|'bool'|'callable'|'string'|'char'|'u8'|'u16'|'u32'|'u64'|'u128'|'i8'|'i16'|'i32'|'i64'|'i128')
    ;
    
genericPart
    : '<' signature (',' signature)* '>'
    ;
    
literal
    : Number
    | Float
    | String
    | Boolean
    | Char
    ;
    
Label
    : '\'' Identifier
    ;
    
Char
    : '\'' . '\''
    ;
    
Boolean 
    : 'true' | 'false'
    ;
    
Number
	: Num
	| NumberType '(' Num ')' 
    ;
        
fragment
Num
    : Sign? (HexNumber
             | OctNumber
             | BinaryNumber
             | DecNumer)
    ;
	
fragment
Sign    
    : [+-]
    ;
	
fragment
DecNumer
    : [0-9]+ 
    ;
    
fragment
NumberType
    : 'u8'|'u16'|'u32'|'u64'|'u128'|'i8'|'i16'|'i32'|'i64'|'i128'
    ;
    
Float
    : FloatNum
    | FloatType '(' FloatNum ')'
    ;
    
fragment
FloatNum
    : [0-9]+ ('.' [0-9]*)?
    ;
    
fragment
FloatType
    : 'f32'|'f64'
    ;
	
fragment
HexNumber
    : '0x' [0-9a-fA-F_]+
    ;
    
fragment
OctNumber
    : '0o' [0-7_]+
	;
	
fragment
BinaryNumber
    : '0b' [0-1_]+
    ;
	
String
    : QuotedString
    ;

fragment    
QuotedString
    : '"' .*? '"'
    ;
    
Identifier
    : '@'? (([a-zA-Z][_a-zA-Z0-9]*) | '_' [_a-zA-Z0-9]+)
    ;
  
Discard
    : '_'
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