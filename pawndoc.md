# Pawndoc, Macros, And Compiler Passes

## Introduction

This tutorial covers some deep and complex interactions between three different parts of the
compiler; interactions and parts that most people will probably never think about (and, honestly,
will never need to).  It covers when functions and macros are defined (thus how the compiler knows a
function exists before you define it), the basics of pawndoc (the in-built documentation system) and
what can and can't be documented, and work-arounds to document those things that "can't" be
documented without breaking anything else.

If you only want to learn about documenting your code, just read the first section.  At the end of
that section some bugs are explained, and solutions provided wholesale.  If you don't care about why
they work, just take the solutions and use them.  If you do care, read the later sections.

## Pawndoc

### Documentation Comments.

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

### Syntax And Tags.

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

### Attached And Unattached Comments.

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

### Documented Symbols

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
// Extra function to use `E_NUM` as a symbol, so it gets in the output.
static stock E_NUM:UnusedFunction() { return E_NUM; }
```

The additional comment before the documentation, and the unused function after do make documenting
enums a little more tricky, but there are macros later to simplify this.

Oh, and you can't document anonymous enums either - sorry.

### XML

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

### Bugs And Solutions

Solving the `enum` bug is easy.  Since unused `const`s do not appear in the XML output, and `enum`s
only appear correctly if they are assigned to something, assign their value to an otherwise unused
`const`:

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

const E_NUM:E_NUM__ = E_NUM;
```

Basic macros are also easy to solve - define a variable with the same name before the macro.  This
will get the comments and appear in the output, while the macro defined after it will be used
throughout the code:

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

Because of the magic of the compiler (making multiple passes, documented later), this code will stop
working because the `#if !defined IsNull` sees the function definition, even though it comes first
(this is why ALS works - the pre-processor can see functions before you define them).  The solution
is a more complex 
