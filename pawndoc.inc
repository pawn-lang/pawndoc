#if defined FIXES_PAWNDOC
	// fixes.inc already does all of this.
	#if !FIX_Pawndoc
		#define HIDE_PAWNDOC _FIXES_HIDE_PAWNDOC
		#define FUNC_PAWNDOC _FIXES_FUNC_PAWNDOC
		#define FUNCTION_PAWNDOC _FIXES_FUNC_PAWNDOC
		#define CONST_PAWNDOC _FIXES_CONST_PAWNDOC
		#define ENUM_PAWNDOC _FIXES_ENUM_PAWNDOC
	#endif
	#endinput
#endif

/**
 * <library name="pawndoc"
 *   summary="Macros and documentation to avoid pawndoc issues.">
 * </section>
 */

///

#if defined PAWNDOC
	#define _PAWNDOC_FIRST_PASS 0
#else
	#define _PAWNDOC_FIRST_PASS 1
#endif

/**
 * <library>pawndoc</library>
 * <remarks>
 * Creates fake functions that Pawndoc comments can be attached to in the first
 * compilation phase, before the real macros are created.  Macros can't have
 * documentation comments, but why not?  They need documenting...
 * <p/>
 * This creates a native of the same name, then calls it from a special function
 * (called <c>PAWNDOC</c>) with a state set to the name of this function, to
 * avoid declaring multiple functions in the XML while ensuring that this fake
 * function is in the XML.  Natives aren't in the output unless they are called,
 * but we don't have to call them correctly because the container function is
 * never called.  You do need a fallback version of `PAWNDOC` to catch all other
 * state variations declared elsewhere in your script.
 * <p/>
 * <code>
 *     #if defined MACRO_NAME                                                   <br /><indent />
 *         // `#define MACRO_NAME` exists previously, don't document it here.   <br /><indent />
 *         HIDE_PAWNDOC(SIMILAR_NAME);                                          <br />
 *     #else                                                                    <br /><indent />
 *         // Doesn't exist.  Document it and define it.                        <br /><indent />
 *         FUNC_PAWNDOC(MACRO_NAME(parameters));                                <br /><indent />
 *         #define MACRO_NAME(%0) (%0)                                          <br />
 *     #endif
 * </code>
 * <br />
 * </remarks>
 */

#if _PAWNDOC_FIRST_PASS
	// First compiler pass only.
	#define _PAWNDOC_FUNC_PAWNDOC(%0(%1)); native %0(%1) = __PAWNDOC; stock PAWNDOC() <__PAWNDOC:%0> { (%0()); }
#else
	#define _PAWNDOC_FUNC_PAWNDOC(%0(%1));
#endif
// Strip tags from states.
#define __PAWNDOC:%0:%1> __PAWNDOC:%1>

_PAWNDOC_FUNC_PAWNDOC(FUNC_PAWNDOC(const symbol[]));

/**
 * <library>pawndoc</library>
 * <remarks>
 * Creates fake constants that Pawndoc comments can be attached to in the first
 * compilation phase, before the real macros are created.  Macros can't have
 * documentation comments, but why not?  They need documenting...
 * <p/>
 * This creates a static stock of the same name to attach the documentation to,
 * before actually defining the macro.
 * <p/>
 * <code>
 *     #if defined MACRO_NAME                                                   <br /><indent />
 *         // `#define MACRO_NAME` exists previously, don't document it here.   <br /><indent />
 *         _FIXES_HIDE_PAWNDOC(SIMILAR_NAME);                                   <br />
 *     #else                                                                    <br /><indent />
 *         // Doesn't exist.  Document it and define it.                        <br /><indent />
 *         _FIXES_CONST_PAWNDOC(MACRO_NAME = 42);                               <br /><indent />
 *         #define MACRO_NAME (42)                                              <br />
 *     #endif
 * </code>
 * <br />
 * </remarks>
 */

#if _PAWNDOC_FIRST_PASS
	// First compiler pass only.
	#define _PAWNDOC_CONST_PAWNDOC(%0=%1); static stock %0 = %1;
#else
	#define _PAWNDOC_CONST_PAWNDOC(%0);
#endif

_PAWNDOC_FUNC_PAWNDOC(CONST_PAWNDOC(const symbol[] = 0));

/**
 * <library>pawndoc</library>
 * <remarks>
 * Create a <c>const</c> that is never used, which has pawndoc documentation
 * attached to it.  This seems to be the best way to prevent the documentation
 * from appearing in XML (<c>-r</c>) output - even conditional compilation
 * doesn't work on comments.
 * <p />
 * The input should have some unique name, so when using it within an ifdef
 * block, don't use the same name as the tested symbol, because the the
 * replacement in question will be applied to the hidden variable as well.
 * <p />
 * This should also be a globally unique symbol name, because this only works
 * with `<c>const</c>`, not `<c>static const</c>`.
 * </remarks>
 */

#define _PAWNDOC_HIDE_PAWNDOC(%0); const %0 = 0;

_PAWNDOC_FUNC_PAWNDOC(HIDE_PAWNDOC(const symbol[]));

/**
 * <library>pawndoc</library>
 * <remarks>
 * There's a bug with the pawndoc for enums.  They loose the documentation, or
 * get some other documentation unless they're immediately followed by another
 * declaration, most notably a function.  This declares a <c>native</c> with a
 * similar name (prefixed with <c>_@</c>, so watch symbol lengths) so the docs
 * are attached to the correct symbol and the additional function isn't output.
 * </remarks>
 */

#define _PAWNDOC_ENUM_PAWNDOC(%0); static stock %0:_@%0() { return %0; }

_PAWNDOC_FUNC_PAWNDOC(ENUM_PAWNDOC(const symbol[]));

#define HIDE_PAWNDOC _PAWNDOC_HIDE_PAWNDOC
#define FUNC_PAWNDOC _PAWNDOC_FUNC_PAWNDOC
#define FUNCTION_PAWNDOC _PAWNDOC_FUNC_PAWNDOC
#define CONST_PAWNDOC _PAWNDOC_CONST_PAWNDOC
#define ENUM_PAWNDOC _PAWNDOC_ENUM_PAWNDOC

/**
	<library>fixes.inc</library>
	<hidden />
	<remarks>
		<c>PAWNDOC</c> is a fake function used throughout the code to invoke
		natives that don't exist so we can attach pawndoc output to them and
		have it show up in the XML output.  This is one function spread all over
		the place using automata - one state per fake function.  This is so we
		don't have dozens of auxiliary functions in the output, and so those
		functions don't need unique names.
	</remarks>
*/

#if _PAWNDOC_FIRST_PASS
	// First compiler pass only.  Just define the states.
	stock PAWNDOC() <__PAWNDOC:PAWNDOC>
	{
	}

	stock PAWNDOC() <>
	{
	}
#endif

