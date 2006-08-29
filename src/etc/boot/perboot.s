0:
	tst	*$177602
	bpl	0b
	clr	*$177604
	clr	*$177600
0:
	tst	*$177602
	bpl	0b
	clr	pc
