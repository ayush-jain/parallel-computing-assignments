default: prod_cons_a.c prod_cons_b.c prod_cons_c.c binary_tree.c
	gcc prod_cons_a.c -pthread -o out1
	gcc prod_cons_b.c -pthread -o out2
	gcc prod_cons_c.c -pthread -o out3
	gcc binary_tree.c -pthread -o out4

clean:
	-rm -f out1
	-rm -f out2
	-rm -f out3
	-rm -f out4
