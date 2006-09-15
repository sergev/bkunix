SUBDIR		= cc pcc c2 crt

all:
		for i in ${SUBDIR}; do $(MAKE) -C $$i; done

install:
		for i in ${SUBDIR}; do $(MAKE) -C $$i install; done

clean:
		rm -f *~
		for i in ${SUBDIR}; do $(MAKE) -C $$i clean; done
