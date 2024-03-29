/*
	Version: MPL 1.1
	
	The contents of this file are subject to the Mozilla Public License Version 
	1.1 (the "License"); you may not use this file except in compliance with 
	the License. You may obtain a copy of the License at 
	http://www.mozilla.org/MPL/
	
	Software distributed under the License is distributed on an "AS IS" basis,
	WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
	for the specific language governing rights and limitations under the
	License.
	
	The Original Code is the code-parse.inc code analyser.
	
	The Initial Developer of the Original Code is Alex "Y_Less" Cole.
	Portions created by the Initial Developer are Copyright (c) 2022
	the Initial Developer. All Rights Reserved.
*/

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
 * <!--
 * <transition target(.*?)/>
 * -->
*/

/**
 * <!--
 * :::::::: README ::::::::
 *
 * To read the generated XML output, copy `pawndoc.xsl` to `xml/` in your
 * compiler directory (usually `pawno/xml/`).
 *
 * This information goes very close to the top of the generated XML
 * documentation (because of include dependency orders), but doesn't appear in
 * the visual output.
 * -->
*/

/**
 * <!--
 * There is <a href="https://github.com/pawn-lang/compiler/issues/184">a bug</a>
 * in the old compiler related to outputting documentation on state transitions.
 * So after generation of the XML file, you should clean it up with the
 * following RegEx replacement:
 *
 * Search: <transition target(.*?)/>
 * Replace: (nothing)
 *
 * This works 99% of the time, though you may get one where the corrupted target
 * includes the sequence `/>`, in which case you should manually delete them.
 * Note that YSI now includes manual documentation for transitions, but these
 * all include the parameter `keep="true"`, which exists simply to not match
 * that RegEx.  I also put the search at the top of this block so it is easier
 * to find in the output code.  This is fixed in the community compiler.
 * -->
 */

/// <p/>

// So YSI doesn't double-declare things.
#define FIXES_PAWNDOC

// Strip tags from states.
#define __PAWNDOC:%0:%1> __PAWNDOC:%1>

// Defer macro expansion.
#define _PAWNDOC_BRACKETS ()

/**
 * <library
 *   name="pawndoc"
 *   summary="Macros and documentation to avoid pawndoc issues."
 *   license="Copyright (c) 2022 Alex &quot;Y_Less&quot; Cole.  Licensed under MPL 1.1"
 * >
 *   <summary pawndoc="true">
 *     This library uses the enhanced <em>pawndoc.xsl</em> from
 *     <a href="https://github.com/pawn-lang/pawndoc">pawn-lang/pawndoc</a>.
 *     This XSL has features such as library and markdown support, and will not
 *     render this message when used.
 *   </summary>
 *   <remarks>
 *     This code is identical to the same code in <em>fixes.inc</em> and
 *     <em>YSI</em>, even down to the macro names just to avoid conflicts if
 *     they are all in use.  In which case include <em>fixes.inc</em> first,
 *     then this, then <em>YSI</em>.
 *   </remarks>
 * </library>
 */

/// <p/>

#if defined PAWNDOC
	#define _PAWNDOC_FIRST_PASS 0
#else
	#define _PAWNDOC_FIRST_PASS 1
#endif

/**
 * <library>pawndoc</library>
 * <remarks>
 *   Creates fake functions that Pawndoc comments can be attached to in the
 *   first compilation phase, before the real macros are created.  Macros can't
 *   have documentation comments, but why not?  They need documenting...
 *   <p/>
 *   This creates a native of the same name, then calls it from a special
 *   function (called <c>PAWNDOC</c>) with a state set to the name of this
 *   function, to avoid declaring multiple functions in the XML while ensuring
 *   that this fake function is in the XML.  Natives aren't in the output unless
 *   they are called, but we don't have to call them correctly because the
 *   container function is never called.  You do need a fallback version of
 *   `PAWNDOC` to catch all other state variations declared elsewhere in your
 *   script.
 *   <p/>
 *   <code>
 *       #if defined MACRO_NAME                                                   <br /><indent />
 *           // `#define MACRO_NAME` exists previously, don't document it here.   <br /><indent />
 *           HIDE_PAWNDOC(SIMILAR_NAME);                                          <br />
 *       #else                                                                    <br /><indent />
 *           // Doesn't exist.  Document it and define it.                        <br /><indent />
 *           FUNC_PAWNDOC(MACRO_NAME(parameters));                                <br /><indent />
 *           #define MACRO_NAME(%0) (%0)                                          <br />
 *       #endif
 *   </code>
 * </remarks>
 */

#if _PAWNDOC_FIRST_PASS
	// First compiler pass only.
	#define _FIXES_FUNC_PAWNDOC(%0(%1)); native %0(%1) = __PAWNDOC; stock PAWNDOC _PAWNDOC_BRACKETS <__PAWNDOC:%0> { (%0()); }
#else
	#define _FIXES_FUNC_PAWNDOC(%0(%1));
#endif

_FIXES_FUNC_PAWNDOC(FUNC_PAWNDOC(const symbol[]));

/**
 * <library>pawndoc</library>
 * <remarks>
 *   Creates fake constants that Pawndoc comments can be attached to in the
 *   first compilation phase, before the real macros are created.  Macros can't
 *   have documentation comments, but why not?  They need documenting...
 *   <p/>
 *   This creates a static stock of the same name to attach the documentation
 *   to, before actually defining the macro.
 *   <p/>
 *   <code>
 *       #if defined MACRO_NAME                                                   <br /><indent />
 *           // `#define MACRO_NAME` exists previously, don't document it here.   <br /><indent />
 *           _FIXES_HIDE_PAWNDOC(SIMILAR_NAME);                                   <br />
 *       #else                                                                    <br /><indent />
 *           // Doesn't exist.  Document it and define it.                        <br /><indent />
 *           _FIXES_CONST_PAWNDOC(MACRO_NAME = 42);                               <br /><indent />
 *           #define MACRO_NAME (42)                                              <br />
 *       #endif
 *   </code>
 * </remarks>
 */

#if _PAWNDOC_FIRST_PASS
	// First compiler pass only.
	#define _FIXES_CONST_PAWNDOC(%0=%1); static stock %0 = %1;
#else
	#define _FIXES_CONST_PAWNDOC(%0);
#endif

_FIXES_FUNC_PAWNDOC(CONST_PAWNDOC(const symbol[] = 0));

/**
 * <library>pawndoc</library>
 * <remarks>
 *   Create a <c>const</c> that is never used, which has pawndoc documentation
 *   attached to it.  This seems to be the best way to prevent the documentation
 *   from appearing in XML (<c>-r</c>) output - even conditional compilation
 *   doesn't work on comments.
 *   <p/>
 *   The input should have some unique name, so when using it within an ifdef
 *   block, don't use the same name as the tested symbol, because the the
 *   replacement in question will be applied to the hidden variable as well.
 *   <p/>
 *   This should also be a globally unique symbol name, because this only works
 *   with `<c>const</c>`, not `<c>static const</c>`.
 * </remarks>
 */

#define _FIXES_HIDE_PAWNDOC(%0); const %0 = 0;

_FIXES_FUNC_PAWNDOC(HIDE_PAWNDOC(const symbol[]));

/**
 * <library>pawndoc</library>
 * <remarks>
 *   There's a bug with the pawndoc for enums.  They loose the documentation, or
 *   get some other documentation unless they're immediately followed by another
 *   declaration, most notably a function.  This uses the same
 *   <c>PAWNDOC&lt;&gt;</c> trick as <c>FUNC_PAWNDOC</c> to use the <c>enum</c>
 *   name as a symbol, this time passing it to a function that should always
 *   exist.  This ensures the docs are attached to the correct symbol and the
 *   additional function is well documented.
 * </remarks>
 */

#define _FIXES_ENUM_PAWNDOC(%0); stock PAWNDOC _PAWNDOC_BRACKETS <__PAWNDOC:%0> { random(_:%0); }

_FIXES_FUNC_PAWNDOC(ENUM_PAWNDOC(const symbol[]));

#define HIDE_PAWNDOC _FIXES_HIDE_PAWNDOC
#define FUNC_PAWNDOC _FIXES_FUNC_PAWNDOC
#define FUNCTION_PAWNDOC _FIXES_FUNC_PAWNDOC
#define CONST_PAWNDOC _FIXES_CONST_PAWNDOC
#define ENUM_PAWNDOC _FIXES_ENUM_PAWNDOC

/**
 * <library>pawndoc</library>
 * <hidden />
 * <remarks>
 *   <c>PAWNDOC</c> is a fake function used throughout the code to invoke
 *   natives that don't exist so we can attach pawndoc output to them and have
 *   it show up in the XML output.  This is one function spread all over the
 *   place using automata - one state per fake function.  This is so we don't
 *   have dozens of auxiliary functions in the output, and so those functions
 *   don't need unique names.
 * </remarks>
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

// Some compile-time safety.
#define PAWNDOC() Dont_Call_PAWNDOC()

/**
 * <library>pawndoc</library>
 * <hidden />
 * <remarks>
 *   This function only exists to hide other functions.  It starts a massive
 *   comment block in the pawndoc XML output that hides all internal functions -
 *   that is, those that start with <c>_</c>.  This is a very common pattern
 *   used throughout pawndoc (and most of programming).  Unfortunately these
 *   weirdly named internal functions appear in the generated documentation
 *   always.  This is the best way to hide them.  You still see one function
 *   (this one), but at least the documentation for this one function can
 *   explain why it exists.
 *   <p/>
 *   This is <c>Zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz</c> and starts the internal
 *   functions block.  This is the last possible name before the first character
 *   is <c>_</c> when the functions are sorted lexicographically
 *   (alphabetically).  There's also <c>Zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzy</c> that
 *   starts the variables block; they could have the same name if they were not
 *   declared in the same scope, but they might not always be separately
 *   defined.
 *   <p/>
 *   Note that the actual hiding itself is done by an XML comment started here
 *   and ended by a later variable.
 * </remarks>
 * 
 * <!--
 */

static stock Zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz() {}

/**
 * <library>pawndoc</library>
 * <hidden />
 * <remarks>
 *   This function only exists to hide other functions.  It end a massive
 *   comment block in the pawndoc XML output that hides all internal functions -
 *   that is, those that start with <c>_</c>.  This is a very common pattern
 *   used throughout pawndoc (and most of programming).  Unfortunately these
 *   weirdly named internal functions appear in the generated documentation
 *   always.  This is the best way to hide them.  You still see one function
 *   (<c>Zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz</c>), but at least the documentation
 *   for that can explain why it exists.
 *   <p/>
 *   This is <c>_zzzzzzzzzzzzzzzzzzzzzzzzzzzzzz</c> and ends the internal
 *   functions block.  This is the last possible name before the first character
 *   is no longer <c>_</c> when the functions are sorted lexicographically
 *   (alphabetically).  There's also <c>_zzzzzzzzzzzzzzzzzzzzzzzzzzzzzy</c> that
 *   ends the variables block; they could have the same name if they were not
 *   declared in the same scope, but they might not always be separately
 *   defined.
 *   <p/>
 *   Note that the actual hiding itself is done by an XML comment started
 *   earlier and ended here.
 * </remarks>
 * 
 * -->
 */

static stock _zzzzzzzzzzzzzzzzzzzzzzzzzzzzzz() {}

/**
 * <library>pawndoc</library>
 * <hidden />
 * <remarks>
 *   This variable only exists to hide other variables.  It starts a massive
 *   comment block in the pawndoc XML output that hides all internal variables -
 *   that is, those that start with <c>_</c>.  This is a very common pattern
 *   used throughout pawndoc (and most of programming).  Unfortunately these
 *   weirdly named internal variables appear in the generated documentation
 *   always.  This is the best way to hide them.  You still see one variable
 *   (this one), but at least the documentation for this one variable can
 *   explain why it exists.
 *   <p/>
 *   This is <c>Zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzy</c> and starts the internal
 *   variables block.  This is the last possible name, except one, before the
 *   first character is <c>_</c> when the functions are sorted lexicographically
 *   (alphabetically).  There's also <c>Zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz</c> that
 *   starts the functions block; they could have the same name if they were not
 *   declared in the same scope, but they might not always be separately
 *   defined.
 *   <p/>
 *   Note that the actual hiding itself is done by an XML comment started here
 *   and ended by a later variable.
 * </remarks>
 * 
 * <!--
 */

static stock Zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzy;

/**
 * <library>pawndoc</library>
 * <hidden />
 * <remarks>
 *   This variable only exists to hide other variables.  It end a massive
 *   comment block in the pawndoc XML output that hides all internal variables -
 *   that is, those that start with <c>_</c>.  This is a very common pattern
 *   used throughout pawndoc (and most of programming).  Unfortunately these
 *   weirdly named internal variables appear in the generated documentation
 *   always.  This is the best way to hide them.  You still see one variable
 *   (<c>Zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzy</c>), but at least the documentation
 *   for that can explain why it exists.
 *   <p/>
 *   This is <c>_zzzzzzzzzzzzzzzzzzzzzzzzzzzzzy</c> and ends the internal
 *   variables block.  This is the last possible name before the first character
 *   is no longer <c>_</c>, except for one, when the functions are sorted
 *   lexicographically (alphabetically).  There's also
 *   <c>_zzzzzzzzzzzzzzzzzzzzzzzzzzzzzz</c> that ends the functions block; they
 *   could have the same name if they were not declared in the same scope, but
 *   they might not always be separately defined.
 *   <p/>
 *   Note that the actual hiding itself is done by an XML comment started
 *   earlier and ended here.
 * </remarks>
 * 
 * -->
 */

static stock _zzzzzzzzzzzzzzzzzzzzzzzzzzzzzy;

