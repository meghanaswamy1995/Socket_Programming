all: serverM.c serverC.c serverCS.c serverEE.c client.c
		gcc -o sv serverM.c
		gcc -o cred serverC.c
		gcc -o cs serverCS.c
		gcc -o ee serverEE.c
		gcc -o cl client.c

sv:
		./sv
cred:
		./cred
cs:
		./cs
ee:
		./ee
cl:
		./cl

.PHONY: sv cred cs ee cl
 
