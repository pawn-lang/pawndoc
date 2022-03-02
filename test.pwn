// Sadly this option doesn't work with `#pragma`.  Invoke the compiler with `-r`
// on the command line or in `pawn.cfg` to see the XML documentation output.
#pragma option -r

#include "pawndoc"

///
/**
 *  My enumeration.
 */
enum E_NUM
{
}
// After the `enum`.
ENUM_PAWNDOC(E_NUM);

/**
 *  My macro.
 */
FUNC_PAWNDOC(MyMacro(const string[]));
// After the `FUNC_PAWNDOC`.
#define MyMacro(%0) (%0[0])

/**
 *  My define.
 */
CONST_PAWNDOC(MY_DEFINE = 42);
// After the `CONST_PAWNDOC`.
#define MY_DEFINE (42)

native print(const string[]);

main()
{
	print("Testing pawndoc output");
}

