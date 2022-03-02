# Pawndoc

## Introduction

This include documents how to document your code and functions using *pawndoc*, the XML-based
documentation comments system built in to the compiler.  It also discusses some bugs with the
system, presents some solutions for said bugs, and extends what can and can't be documented.

If you only want to learn about documenting your code, just read the first section.  At the end of
that section some bugs are explained, and solutions provided wholesale.  If you don't care about why
they work, just take the solutions and use them.  If you do care, read the later sections.

## Documentation Comments.

The simplest place to start is with *pawndoc*, the compiler in-built system for generating
documentation from comments.  When you compile a mode with the `-r` flag an XML file is generated
containing all your global variables and functions, to which you can attach extra information.  This
extra information is added through special documentation comments - those using `///` or `/** */`
instead of `//` and `/* */`.  The comments right before a function are the comments for that
function:

```pawn
/**
	<remarks>
		Gets a random integer between 0 and <c>n - 1</c> (inclusive).
	</remarks>
*/
native random(n);
```

That will produce the following in an XML file:

```xml
<member name="M:random" syntax="random(n)">
	<attribute name="native"/>
	<referrer name="main"/>
	<param name="n"></param>
	<remarks>  Gets a random integer between 0 and <c>n - 1</c> (inclusive).  </remarks> 
</member>
```

## Syntax And Tags.

There are a few standard XML tags, like `<summary>`, `<remarks>`, and `<param>` (which can be given
explicitly or auto-generated); but any valid XML can be used and the result parsed for output via a
`pawndoc.xsl` file (which won't be covered here).  Note that the requirement for valid XML does mean
that these comments need standard XML escapes like `&amp;` and `&lt;` etc:

```pawn
/**
 * <param name="playerid">The player you want to do something to.</param>
 * <summary>
 *   A custom function!  Returns <c>true</c> when the playerid is <c>&lt; 4</c>.
 * </summary>
 */
MyFunction(playerid)
{
	return playerid < 4;
}
```

Leading `*`s on comment lines are ignored, to make writing the comments nicer.  Here the parameter
was specified explicitly so we can give more information in the output, which now looks like:

```xml
<member name="M:MyFunction" syntax="MyFunction(player)">
	<stacksize value="1"/>
	<referrer name="main"/>
	<param name="playerid">
		The player you want to do something to.
	</param>
	<summary>
		A custom function!    Returns <c>true</c> when the playerid is <c>&lt; 4</c>.
	</summary>
</member>
```

Stacksize, referrers (callers), dependencies (callees), automata (states), and more are also
auto-generated in the output.  Note that some parts of the comments (like `<param>`) are parsed, but
some are just copied verbatim (with only newlines stripped).  You can even include XML comments in
the output:

```pawn
/**
 * <!--
 *   A comment in a comment!
 * -->
 */
```

## Attached And Unattached Comments.

Documentation comments are attached to the next or current declaration:

```pawn
       /// All
Func() /// on
{
}      /// <c>Func</c>
```

Comments on the same line as a function or variable are easy - they are always
attached to the given symbol.  So regardless of what else happens `on` and
`<c>Func</c>` in the example above will always document `Func()`.  `All`, on the
other hand, is a little different.

Comments before a declaration will document the next symbol, unless there's
another documentation comment between it and the symbol.  `///`s on adjacent
lines count as the same documentation block so this is allowed:

```pawn
/// These lines
/// all document
/// the variable.
new gVariable = 7;
```

Blank lines, normal comments, and pre-processor directives after the last
documentation comment don't matter:

```pawn
/// These lines
/// all document
/// the variable.

#define BIG_GAP
// This normal comment is irrelevant.

new gVariable = 7;
```

But any gap (including directives and normal comments) mid-documentation resets
the comment block:

```pawn
/// These lines
/// are unattached.

/// Only this line documents the variable.
new gVariable = 7;
```

`/** */` comments are always independent of each other, and will also break up
`///` blocks:

```pawn
/// These lines
/// are unattached.
/**
   Only this documents, despite the adjacency.
*/
new gVariable = 7;
```

So what happens to the comments that don't get assigned to a symbol, because
there's another documentation comment that gets in the way?  They are global
documentation comments and appear in the `<general>` tag at the start of the
XML.  These are "unattached", but still useful for documenting whole
libraries or general information.  *fixes.inc* for example has a huge
unattached documentation block at the start of the file for settings, styles,
credits, and more.

## Documented Symbols

Most symbols can be documented - functions (all types), variables, constants, and enums.  Things
that can't be documented include pre-processor macros and enum members.  Some comments are removed
from the XML if their symbol isn't used - mainly constants and natives; variables and functions are
always in the XML, even if they don't end up in the final AMX.  Sadly, while enums can be
documented, they are quite buggy - they are only included in the output if you use them as an array
size or constant, not a tag, nor if you only use a member:

```pawn
/**
 * <remarks>
 *   <c>e num</c> - get it?
 * </remarks>
 */

enum E_NUM
{
	A = 5
}

new gArray[E_NUM]; // This will generate output.
new E_NUM:gTagged; // This will not.
new gValue = E_NUM; // This will generate output.
new gMember = A; // This will not.
```

```xml
<member name="T:E_NUM" value="6">
	<tagname value="E_NUM"/>
	<member name="C:A" value="5">
	</member>
	<remarks>  <c>e num</c> - get it?  </remarks> 
</member>
```

Unused enums will also not appear in the XML, but their comments still will, which means those
comments will end up in the `<general>` section with all the unattached comments.  Sometimes their
comments will end up on the wrong symbol, and sometimes they will inherit the comments of another
symbol (the next one, not just a random one).  Finally, it is possible to crash the compiler on
certain enums with a mix of attached and unattached comments.  Why is not currently explored, but
the solution to all these problems are easy - use the enum:

```pawn
// An empty unattached documentation block before the enum.  Prevents previous
// comments leaking to the enum
///

/**
	Documentation on the enum, as normal.
*/
enum E_NUM
{
	E_LEMENT,
}
// Extra constant to use `E_NUM` as a symbol, so it gets in the output.
const E_NUM:UnusedConst = E_NUM;
```

The additional comment before the documentation, and the unused function after do make documenting
enums a little more tricky, but there are macros later to simplify this.

Oh, and you can't document anonymous enums either - sorry.

## XML

The XML file output has the following general structure:

```xml
<?xml version="1.0" encoding="ISO-8859-1"?>
<?xml-stylesheet href="file:///C|/Path/To/Pawno/xml/pawndoc.xsl" type="text/xsl"?>
<doc source="C:\Path\To\Your\File.pwn">
	<assembly>
		<name>File.pwn</name>
	</assembly>

	<!-- general -->
	<general>
		Your general (unattached) comments go here.
	</general>

	<members>
		<!-- enumerations -->
		<member name="T:E_NUM" value="6">
		</member>

		<!-- constants -->
		<member name="C:CONST_NAME" value="55">
		</member>

		<!-- variables -->
		<member name="F:gVariable">
		</member>

		<!-- functions -->
		<member name="M:Function" syntax="Function(parameter)">
		</member>
	</members>
</doc>
```

The names have the following prefixes: `T` - `enum`, `C` - `const`, `F` - variable, `M` - function.
The commented section headers also appear in the XML file.  And yes, both `/` and `\` are used for
path separators at the start.

## Bugs And Solutions

Solving the `enum` bug is easy.  Since unused `const`s do not appear in the XML output, and `enum`s
only appear correctly if they are assigned to something, assign their value to an otherwise unused
`const`:

```pawn
///
/**
 * <remarks>
 *   <c>e num</c> - get it?
 * </remarks>
 */

enum E_NUM
{
	A = 5
}

const E_NUM:_@E_NUM = E_NUM;
```

Basic macros being undocumentable are also easy to solve - define a variable with the same name
before the macro.  This will get the comments and appear in the output, while the macro defined
after it will be used throughout the code:

```pawn
/**
 * <remarks>
 *   But what is the question?
 * </remarks>
 */

static stock THE_ANSWER = 42;
#define THE_ANSWER (42)
```

We can do a similar thing with function-like macros - define a dummy function with the same name
first:

```pawn
/**
 * <summary>
 *   Check if a string is empty, or almost empty.
 * </summary>
 */

static stock IsNull(const string[]) {}
#define IsNull(%0) ((%0[(%0[0])=='\1'])=='\0')
```

Unfortunately, this breaks a common pattern:

```pawn
/**
 * <summary>
 *   Check if a string is empty, or almost empty.
 * </summary>
 */

#if !defined IsNull
	static stock IsNull(const string[]) {}
	#define IsNull(%0) ((%0[(%0[0])=='\1'])=='\0')
#endif
```

Because of how the compiler makes multiple passes to find functions declared later in code and use
them before their declaration (this is why ALS works - the pre-processor can see functions before
you define them) this code will stop working because the `#if !defined IsNull` sees the new fake
function definition, even though it comes first.  The solution is to use a `native` instead of a
normal function, because natives aren't recorded for future passes:

```pawn
#if !defined IsNull
	native IsNull(const str[]);
	#define IsNull(%0) ((%0[(%0[0])=='\1'])=='\0')
#endif
```

But this gives another problem - unused natives aren't included in the XML, so we need another
function to call that native:

```pawn
#if !defined IsNull
	native IsNull(const str[]);
	static stock _@IsNull(const str[]) { IsNull(str); }
	#define IsNull(%0) ((%0[(%0[0])=='\1'])=='\0')
#endif
```

This introduces yet more problems (especially when abstracting this fix to a macro):

* How should `IsNull` be called?  Determining the parameters for a function from its signature is
possible in a generic macro, but very very complicated.
* Now there's a second function in the XML - `_@IsNull`, that is never used and shouldn't be used,
but its in there cluttering things up.
* If the function in question is 30+ characters long the addition of `_@` to make a new unique
function name will give warnings.

The final solution is even more complex, so the explanation is in a later section:

```pawn
#if __COMPILER_FIRST_PASS
	// First compiler pass only.
	#define FUNC_PAWNDOC(%0(%1)); native %0(%1) = __PAWNDOC; stock PAWNDOC _PAWNDOC_BRACKETS <__PAWNDOC:%0> { (%0()); }
#else
	#define FUNC_PAWNDOC(%0(%1));
#endif

#if __COMPILER_FIRST_PASS
	// First compiler pass only.
	#define CONST_PAWNDOC(%0=%1); static stock %0 = %1;
#else
	#define CONST_PAWNDOC(%0);
#endif

#define _FIXES_ENUM_PAWNDOC(%0); stock PAWNDOC PP_BRACKETS<> <__PAWNDOC:%0> { random(_:%0); }

// Some compile-time safety.
#define PAWNDOC() Dont_Call_PAWNDOC()

// Strip tags from states.
#define __PAWNDOC:%0:%1> __PAWNDOC:%1>

// Defer macro expansion.
#define _PAWNDOC_BRACKETS ()
```

Examples:

```pawn
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
```

If you can see what's going on here you'll see that there is still an extra function generated in
the XML - called `PAWNDOC`  However, other solutions give an extra function for every documented
macro, this only gives one total, and that one can be carefully documented to explain *why* it is
there (possibly by linking to this document).

Or, just include this include!

## Pre-Processor Issue

There's one more issue with documentation comments - they aren't affected by the pre-processor.
This code will not work as expected when `IsNull` already exists:

```pawn
#if !defined IsNull
	/**
	 * <summary>
	 *   Check if a string is empty, or almost empty.
	 * </summary>
	 */

	FUNC_PAWNDOC(IsNull(const string[]));
	#define IsNull(%0) ((%0[(%0[0])=='\1'])=='\0')
#endif
```

If `IsNull` already exists this branch will not be entered, but the documentation comments will
still be parsed and output.  Thus, because the function they should be attached to isn't output,
this comment becomes an unattached comment and ends up in `<general>`  So by trying to hide
functions a load of irrelevant comments end up in the XML header.  So we need a fake function to
attach the documentation to in the other case (for which we can use a `native` or `const`, so the
comments don't go in the output at all):

```pawn
#define HIDE_PAWNDOC(%0) const %0 = 0;
```

And put the comments above the directives:

```pawn
/**
 * <summary>
 *   Check if a string is empty, or almost empty.
 * </summary>
 */

#if defined IsNull
	HIDE_PAWNDOC(IsNull);
#else
	FUNC_PAWNDOC(IsNull(const string[]));
	#define IsNull(%0) ((%0[(%0[0])=='\1'])=='\0')
#endif
```

It is also possible to merge multiple documentation comments for multiple functions together.  When
using `///` the comments will go on the next function, unless that function is not compiled.  So
this can be exploited to link many comments together by ensuring that EVERY line ends with `///`:

```pawn
#if SOME_CHECK
	/// The documentation for <c>Func1</c>
	stock Func1() ///
	{             ///
	}             ///
	              ///
	/// The documentation for <c>Func1</c>
	stock Func2() ///
	{             ///
	}             ///
#else             ///
	native UnusedForHidingDocumentation();
#endif
```

If `SOME_CHECK` is `true`, then `Func1` and `Func2` will both get the correct documentation
attached (with a few extra blank lines).  The moment `Func1` ends the comments stop being attached
to it and start getting attached to the next function. If `SOME_CHECK` is `false` then all the `///`
lines will be attached to `UnusedForHidingDocumentation`, which is never used and so doesn't appear
in the output, taking all its documentation with it.  `fixes.inc` has an instance of this spanning
several hundred lines and nearly as many function definitions, because using a fake extra function
for all of them would have been massively excessive.

## Automata Issue

If you are on the old compiler there is
[another bug](https://github.com/pawn-lang/compiler/issues/184) with pawndoc comments that this
library actually makes worse - state transitions are not correctly documented and instead generate
uninitialised rubbish.  So after generation of the XML file, you have to clean it up with the
following RegEx replacement:

    Search: <transition target[^/]+/>
    Replace: (nothing)

This works 99% of the time, though you may get one where the corrupted target includes the character
`/`, in which case you should manually delete them.  Note that YSI now includes manual documentation
for transitions, but these all include the parameter `keep="true"`, which exists simply to not match
that RegEx.  This is fixed in the community compiler.

## `PAWNDOC` Function

The `FUNC_PAWNDOC` macro looks like:

```pawn
#define FUNC_PAWNDOC(%0(%1)); native %0(%1) = __PAWNDOC; stock PAWNDOC() <__PAWNDOC:%0> { (%0()); }
```

The function `PAWNDOC` is used to call the natives to put them in the output XML because there is
then only one superfluous function in the XML, instead of hundreds.  The use of automata means that
this one function can instead be redefined hundreds of times itself.  The use of
`native Func() = __PAWNDOC;` instead of just `native Func();` means that if you do somehow call it
you'll get an error about `__PAWNDOC` being an unknown function, not random macros that do exist.

The call has no parameters, why is that not always an error?  Surely that means that the call
doesn't (usually) match the function declaration?  The answer is simple - `PAWNDOC` is never called,
so its contents are never accurately checked.  The code is syntactically correct and that's all the
compiler cares about in code that is never run (and sometimes not even that).

The use of `:%0` as a state, rather than `_@%0` as a separate symbol means that if `%0` is
under the function name length limit, the `:%0` state always is as well.  Again, sadly that's not
the case on the enums, but that's the only case for now.

The function is declared as `PAWNDOC _PAWNDOC_BRACKETS` instead of `PAWNDOC()` so that the compiler
doesn't match it against the `PAWNDOC()` macro.  This is to do with evaluation order - the compiler
sees the text `PAWNDOC` and, per the definition of the macro, looks to see if this is followed by
`()`.  It isn't, it is followed by `_PAWNDOC_BRACKETS`, so the replacement isn't done.  Then the
compiler moves on to the next symbol and sees `_PAWNDOC_BRACKETS`.  This does exactly match a known
macro and this symbol is replaced by `()`.  So the generated code will contain `PAWNDOC ()`, but by
this point it is too late for the pre-processor; it doesn't backtrack and doesn't retry the earlier
`PAWNDOC` macro.  This allows any calls to `PAWNDOC()` to be replaced by `Dont_Call_PAWNDOC()`,
which doesn't exist and gives a compile-time error; but bypasses this replacement for the
`PAWNDOC()` declarations.

