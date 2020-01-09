gcc -c qf.c
ar rcs libqf.a qf.o

gcc -c bloom.c
ar rcs libbloom.a bloom.o

gcc -c tst.c
ar rcs libtst.a tst.o

gcc test_cpy.c -o test_cpy -L. -lqf -lbloom -ltst -lm

gcc test_common.c -o test_common -L. -lqf -lbloom -ltst -lm


