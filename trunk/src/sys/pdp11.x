/* Default linker script, for normal executables */
OUTPUT_FORMAT("a.out-pdp11", "a.out-pdp11",
	      "a.out-pdp11")
OUTPUT_ARCH(pdp11)
SECTIONS
{
	start = 0;
	. = 0;
	.text :
	{
		CREATE_OBJECT_SYMBOLS
		*(.text)
		_etext = .;
	}
	. = ALIGN(2);
	.data :
	{
		__data_start = .;
		*(.data)
		CONSTRUCTORS
		_edata = .;
	}
	.bss :
	{
		__bss_start = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(2);
		_end = . ;
	}
}
