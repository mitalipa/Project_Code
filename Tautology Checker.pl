/*Parser will parse the below laguage

 S0 -> Var | 'True' | 'False' | '(' S6 ')'

 S2 -> S0 | '~' S2

 S3 -> S2 | S3 '&' S2

 S4 -> S3 | S4 'or' S3

 S5 -> S4 | S4 '=>' S5

 S6 -> S5 | S5 '<=>' S6

*/

s0(L,T):-
	L=[H],(H='True',T='True';H='False',T='False')
	;
	L=[H],not(H='True';H='False'),T=H
	;
	append(['('|A],[')'],L),s6(A,T).

s2(L,T):-
	s0(L,T)
	;
	L=['~'|A],s2(A,T1),T=['~',T1].

s3(L,T):-
	s2(L,T)
	;
	append(A,['&'|B],L),s3(A,T1),s2(B,T2),T=['&',T1,T2].

s4(L,T):-
	s3(L,T)
	;
	append(A,['or'|B],L),s4(A,T1),s3(B,T2),T=['or',T1,T2].

s5(L,T):-
	s4(L,T)
	;
	append(A,['=>'|B],L),s4(A,T1),s5(B,T2),T=['=>',T1,T2].

s6(L,T):-
	s5(L,T)
	;
	append(A,['<=>'|B],L),s5(A,T1),s6(B,T2),T=['<=>',T1,T2].

parse(L,T):-
	s6(L,T).

/*Evaluator*/

evaluator(S,T):-
	S='True',T=true
	;
	S='False',T=false
	;
	S=['~',A],evaluator(A,A1),(A1=true,T=false;A1=false,T=true)
	;
	S=['&',A,B],evaluator(A,A1),evaluator(B,B1),(A1=true,B1=true,T=true;
						    A1=true,B1=false,T=false;
						    A1=false,B1=true,T=false;
						    A1=false,B1=false,T=false)
	;
	S=['or',A,B],evaluator(A,A1),evaluator(B,B1),(A1=false,B1=false,T=false;
						     A1=true,B1=false,T=true;
						     A1=false,B1=true,T=true;
						     A1=true,B1=true,T=true)
	;
	S=['=>',A,B],evaluator(A,A1),evaluator(B,B1),(A1=true,B1=false,T=false;
						     A1=true,B1=true,T=true;
						     A1=false,B1=false,T=true;
						     A1=false,B1=true,T=true)
	;
	S=['<=>',A,B],evaluator(A,A1),evaluator(B,B1),(A1=true,B1=true,T=true;
						      A1=false,B1=false,T=true;
						      A1=false,B1=true,T=false;
						      A1=true,B1=false,T=false).


/* Get variables from the S-Expression which can have repetition.
*/

getVariables(S,T):-
	S=[],T=[]
	;
	S=[H],not(H=[_|_]),(not(H='~'),not(H='&'),not(H='or'),not(H='=>'),
	       not(H='<=>'),not(H='True'),not(H='False'),T=[H];T=[]),!
	;
	S=[H],H=[_|_],getVariables(H,T)
	;
	S=[H|Tail],not(Tail=[]),getVariables(Tail,T1),
	(not(H=[_|_]),getVariables([H],T2);H=[_|_],getVariables(H,T2)),append(T1,T2,T).

/*
supporting function of the cross product. If Y is a list,
singletonCP(X,Y,R) means R is a list of all pairs [X,B] where B in Y.
*/
singletonCP(X,Y,R):-
	Y=[],R=[] ;
	Y=[H|T], singletonCP(X,T,R1), R=[[X,H]|R1].

/*
cross product(X,Y,R) where R is the list of all pairs [A,B] where A is
member of X and B is a member of Y
*/
crossProduct(X,Y,R):-
	X=[], R=[]
	;
	X=[H|XT],
	crossProduct(XT,Y,R1),
	singletonCP(H,Y,R2),
	append(R1,R2,R).


/*
getTruthValues(V,L) where L is the list of truth values combination as
per the number of variables contained in the list V.
*/

getTruthValues(V,L):-
	V=[_],L=['True','False']
	;
	V=[_|T],getTruthValues(T,L1),crossProduct(['True','False'],L1,L).

/*
flatten(X,Y) will flatten the list X into a single list for example
X=[1,[2,3],[4,5]] and Y =[1,2,3,4,5]
*/
flatten(X,Y):-
	X=[],Y=[]
        ;
        X=[H|T],
        flatten(H,Y2),
        flatten(T,Y3),
        append(Y2,Y3,Y),!
        ;
        not(X=[];X=[_|_]),Y=[X].

/*
FlattenElement(L,NL) which takes a list as input which may have list inside list
ex
*/
flattenElement(L,NL):-
	L=[],NL=[]
	;
	L=[H|T],flatten(H,NL1),flattenElement(T,NL2),NL=[NL1|NL2].

/*
	T=list which contains truth values
	S=List containing S-Expression
	V=List containing List of variables
	L=Output List

This method replaces the variable which is in S and equal to V
with the value in T and returns the updated list L.
V=variable to be replaced
S=S-Expression
T=T/F
L=output

*/
replaceVariable(V,S,T,L):-
	S=[],L=[]
	;
	S=[Head|Tail],replaceVariable(V,Tail,T,L1),(Head=V,L=[T|L1];not(Head=V),not(Head=[_|_]),L=[Head|L1];Head=[_|_],
						    replaceVariable(V,Head,T,L2),L=[L2|L1]).

/*
   T=Truth value
   S=S-Expression
   V=variable list
   L=output
*/
substituteElement(T,S,V,L):-
	V=[],T=[],L=S
	;
	V=[H|Tail],T=[Th|Tt],replaceVariable(H,S,Th,L1),substituteElement(Tt,L1,Tail,L).

/*
   T=truth value list
   S=S-Expression
   V=variable list
   L=output
*/
substitute(T,S,V,L):-
	T=[],L=[]
	;
	T=[Head|Tail],substituteElement(Head,S,V,L1),substitute(Tail,S,V,L2),L=[L1|L2].

/*check if tautology*/

checkTautology(T):-
	T=[]
	;
	T=[Head|Tail],
	evaluator(Head,T1),checkTautology(Tail),T1=true.


/*
   taut ->use sort
*/
taut(E):-
	parse(E,P),getVariables(P,V1),
	sort(V1,V),
	getTruthValues(V,TV),
	flattenElement(TV,FE),
	substitute(FE,P,V,T),
	checkTautology(T).













