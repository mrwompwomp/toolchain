.. _fontlibc_h:

fontlibc.h
==========

.. code-block:: c

    #include <fontlibc.h>

fontlibc was designed under a "mechanism not policy" sort of philosophy.
Rather than attempting to provide as many fancy features as a programmer could want, fontlibc tries to provide fast, basic routines that can be used to build the additional functionality you want.
For example, word-wrap is not directly provided, but can be implemented using :code:`fontlib_SetAlternateStopCode` and :code:`fontlib_GetStringWidth`.
fontlib hopes to provide enough performance to be usable in games, while providing powerful enough basic features for fancy GUIs and document editors.

.. contents:: :local:
   :depth: 3

Creating Fonts
--------------

Fonts for use with FontLibC can be made with any program that can produce Windows 3.x .FNT resource files.

Editor Software
~~~~~~~~~~~~~~~

`Fony <http://hukka.ncn.fi/?fony>`_ is probably the most-used FNT editor available. It can open both .FNT and .FON files.

`mkwinfont <https://github.com/juanitogan/mkwinfont>`_ provides some Python code for converting a text-based format to and .FON files; it should be trivial for someone with basic Python skills to change to code to skip the .FON packaging stage and directly produce a .FNT resource file. Useful if for some reason you think graphics are best done without the aid of any kind of graphics editor.

`VSoft's FontEdit <http://www.vsoft.nl/software/utils/win/fontedit/>`_ is mostly just the original FontEdit included with the Windows SDK for Windows 3.x, but compiled for 32-bit Windows. A notable addition, however, is the ability to import TrueType fonts, though I haven't tested it. It cannot, however, open .FON files; the FNT resource(s) embedded in a .FON file must be extracted before FontEdit can open the font(s).

`MFE <https://github.com/drdnar/MFE>`_ is DrDnar's own bitmap font editor. It can export .FNT files. It has the useful feature of allowing fully custom mapping from Unicode to your font's 8-bit code page, which makes creating mock-ups with the preview function easier.

Using Fonts in Your Project
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once your .FNT file has been created, use the convfont utility included with the SDK to convert the .FNT resource file into a format usable in your project.

There are two main ways of including a font in your project:

 - directly in your program binary; or,
 - in separate font pack appvar.

Embedding a Font Directly in Your Program
.........................................

Embedding a font directly in your program ensures the font will always be available, but it prevents it from being used by any other program, and bloats your program. However, it is also the easiest way to access a custom font.

Place your .FNT font files in your source code directory. Then, create a :code:`myfonts.h` source code file:

.. code-block:: c

    /* Declare globally-accessible pointers to the font data. */
    extern const fontlib_font_t *my_font_1;
    extern const fontlib_font_t *my_font_2;
    . . .

Then create a :code:`myfonts.c` file:

.. code-block:: c

    /* This contains the raw data for the font. */
    static const uint8_t my_font_1_data[] = {
    	#include "myfont1.inc"
    };
    /* However, C89 does not allow us to typecast a byte array into a
    fontlib_font_t pointer directly, so we have to use a second statement to do it,
    though helpfully we can at least do it in the global scope. */
    const fontlib_font_t *my_font_1 = (fontlib_font_t *)my_font_1_data;
    
    static const uint8_t my_font_2_data[] = {
    	#include "myfont2.inc"
    };
    const fontlib_font_t *my_font_2 = (fontlib_font_t *)my_font_2_data;

Now you should be wondering where the :code:`myfont1.inc` file comes from. This file will get generated by your makefile, which will need to be modified to append the following:

.. code-block:: make

    # This is a roundabout way to tell make that myfonts.c depends on the .inc files.
    # It does it by saying the compiled object code depends on the .inc files.
    $(OBJDIR)/myfonts.src: $(SRCDIR)/myfont1.inc $(SRCDIR)/myfont2.inc
    
    # Convert a .fnt file into a .inc file
    $(SRCDIR)/myfont1.inc: $(SRCDIR)/myfont1.fnt
    	convfont -o carray -f $(SRCDIR)/myfont1.fnt $(SRCDIR)/myfont1.inc
    
    $(SRCDIR)/myfont2.inc: $(SRCDIR)/myfont2.fnt
    	convfont -o carray -f $(SRCDIR)/myfont2.fnt $(SRCDIR)/myfont2.inc

Finally, somewhere else in your program, you can use :code:`fontlib_SetFont`:

.. code-block:: c

    void main() {
        . . .
        fontlib_SetFont(my_font_1, 0);
        . . .
    }

Packaging a Font Pack
.....................

Font packs are an alternative to directly embedding a font in your program binary. They allow multiple related fonts to be packaged together, and FontLibC can select a font from the font pack given a requested size and style. The fonts in a font pack can be used by other programs, reducing the size of your program and saving valuable space on-calculator. They can also be archived, freeing up limited RAM.

A font pack should contain related fonts, namely different sizes and styles of a typeface. It is legal for a font pack to contain only one font. Metadata fields in a font pack, such as the description, should be *short.*

Font packs are easiest to make as a separate project. Create a new folder, place your :code:`.fnt` files in it, and then create a :code:`makefile` with the following contents:

.. code-block:: make

    # Put each of your .fnt files on this next line.
    # Look at the documentation for convfont for more information on font properties
    temp.bin: font1.fnt font2.fnt font3.fnt
        convfont -o fontpack -N "Font Name" -P "ASCII" -V "Some version or date" -A "Your Name" \
        -D "A SHORT description of your font" \
        -f font1.fnt -a 1 -b 1 -w bold -s sans-serif -s upright -s proportional \
        -f font2.fnt -a 2 -b 2 -w normal -s serif -s italic \
        -f font3.fnt -a 0 -b 3 -w light -s monospaced \
        temp.bin

    # Don't forget to change font_pack_file_name on both these lines.
    # Set PACKNAME to the on-calculator appvar name you want
    font_pack_file_name.8xv: temp.bin
        convhex -a -v -n PACKNAME temp.bin font_pack_file_name.8xv

    all: font_pack_file_name.8xv

Using Font Packs
----------------

While using an embedded font is easy—just call :code:`fontlib_SetFont` directly on the pointer to the font data—, using a font pack is a bit more involved.

**WARNING: FontLibC caches a pointer to the font's data when you use** :code:`SetFont`. **If you do something that causes the font's data to move, that pointer becomes invalid and FontLibC will start displaying garbage!** For example, if a font appvar is in RAM, any operation that creates or resizes a file may invalidate the cached pointer. Simply calling :code:`SetFont` again will not suffice to fix this; you must also lookup the font's location again. This also applies if a font pack is archived, and you do something that causes a garbage collection cycle.

(The above warning does not apply to fonts embedded into your program, as data embedded in your program cannot get moved.)

Opening a Font Pack the Simple Way
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you require a specific font pack with a specific appvar name, then opening a font is straight-forward:

.. code-block:: c

    #include <graphx.h>
    #include <fontlibc.h>

    void main(void) {
        fontlib_font_t *my_font;
        . . .
        /* Get the first font present in the font pack */
        my_font = fontlib_GetFontByIndex("MYFONT", 0);
        /* This check is important! If fetching the font fails, trying to use the font will go . . . poorly. */
        if (!my_font) {
            gfx_PrintStringXY("MYFONT appvar not found or invalid", 0, 0);
            return;
        }
        /* Use font for whatever */
        fontlib_SetFont(my_font, 0);
        . . .
    }

Caching a Font Pack's Address
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

However, accessing fonts this way triggers a slow VAT lookup for the appvar every time you call a :code:`GetFont` routine. You can avoid this overhead—provided you keep in mind the above warning about moving data around—by using the FileIOC library to get a direct pointer to the appvar's data.

.. code-block:: c

    ti_var_t file;
    fontlib_font_pack_t *my_font_pack;
    fontlib_font_t *my_font;
    . . .
    /* Open file */
    file = ti_open("MYFONT", "r");
    /* Throw an error if the file was not found */
    if (!my_font_pack) {
        gfx_PrintStringXY("MYFONT appvar not found", 0, 0);
        return;
    }
    my_font_pack = ti_GetDataPtr(file);
    /* Once we have the pointer, we don't need the file handle any more. */
    ti_Close(file);
    /* Just because the file exists doesn't mean it's actually a font pack */
    my_font = fontlib_GetFontByIndexRaw(my_font_pack, 0);
    if (!my_font) {
        gfx_PrintStringXY("MYFONT appvar is invalid", 0, 0);
        return;
    }

Finding a Font Pack by Typeface Name
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In addition to opening a font pack by appvar name, FontLibC also provides the special routine :code:`fontlib_GetFontPackName` to make it easier to search for a font pack by typeface name:

.. code-block:: c

    char *var_name;
    char *typeface_name;
    fontlib_font_pack_t *my_font_pack;
    uint8_t *search_pos = NULL;
    while ((var_name = ti_DetectVar(&search_pos, "FONTPACK", TI_APPVAR_TYPE)) != NULL) {
        typeface_name = fontlib_GetFontPackName(var_name);
        /* Font packs can omit the name metadata property, so check for that. */
        if (!typeface_name)
            continue;
        /* Do something interesting with the typeface_name here.
           Note that a direct pointer to the name is returned, which may be archived,
           so you cannot write to the string. */
        if (!strcmp(typeface_name, "My Font")) {
            fontlib_SetFont(fontlib_GetFontByIndex(var_name, 0), 0);
            break;
        }
    }

Looking at Other Font Metadata
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are no other routines for processing the other metadata fields in a font pack. However, it is not hard to process the metadata fields yourself:

.. code-block:: c

    ti_var_t file;
    fontlib_font_pack_t *font_pack;
    int metadata_offset;
    fontlib_metadata_t *font_pack_metadata;
    char *font_pack_author;
    . . .
    /* Assume that file is the already-opened appvar of the font pack. */
    font_pack = ti_GetDataPtr(file);
    metadata_offset = font_pack->metadata;
    if (!metadata_offset)
        return; /* No metadata in the font pack, do something else here */
    ti_Seek(metadata_offset, SEEK_SET, file);
    font_pack_metadata = ti_GetDataPtr(file);
    metadata_offset = font_pack_metadata->font_author;
    if (!metadata_offset)
        return; /* No author specified */
    ti_Seek(metadata_offset, SEEK_SET, file);
    font_pack_author = ti_GetDataPtr(file);
    /* Now you have the author name string */

Selecting a Font by Style
~~~~~~~~~~~~~~~~~~~~~~~~~

The :code:`GetFontByStyle` routines help you automatically select a font given a set of size and style criteria.

.. code-block:: c

    fontlib_font_pack_t *font_pack;
    fontlib_font_t *font;
    . . .
    /* Assume font_pack already points to a valid font pack. */
    /* Get a 9 or 10 pixel tall bold, serif font that isn't monospaced and isn't italic. */
    font = fontlib_GetFontByStyleRaw(font_pack, 9, 10, FONTLIB_BOLD, FONTLIB_BOLD, FONTLIB_SERIF, FONTLIB_MONOSPACED | FONTLIB_ITALIC);
    if (font)
        fontlib_SetFont(font, 0);

API Usage Notes
---------------

Text Windowing
~~~~~~~~~~~~~~

To assist in text layout, fontlibc provides for a text window, which automatically confines text to appear in a specific rectangular area of the screen. This feature may be useful for dialogs and scrolling large blocks of text. Use :code:`fontlib_SetWindow` to set the current window bounds. Use :code:`fontlib_SetNewlineOptions` to control how :code:`fontlib_DrawString` behaves when it reaches the right edge of the text window.

Aligning Text
~~~~~~~~~~~~~

Implementing centered text, right-aligned text, and word wrap requires being able to compute the width of a word or string of text. The routine :code:`fontlib_GetStringWidth` provides this functionality.

If you call :code:`fontlib_SetAlternateStopCode(' ')`, :code:`fontlib_GetStringWidth` and :code:`fontlib_DrawString` will stop drawing on spaces, giving you a chance to check if the next word will fit on screen. You can use :code:`fontlib_GetLastCharacterRead()` to find out where :code:`fontlib_GetStringWidth` or :code:`fontlib_DrawString` stopped, and, after handling the space, then pass that address (plus one) again to :code:`fontlib_GetStringWidth` or :code:`fontlib_DrawString` to resume processing at where it left off before.

Text Control Codes
~~~~~~~~~~~~~~~~~~

Embedded control codes are a popular way of managing style and formatting information in string.
fontlibc only natively recognizes two types of control codes: NULL (0) as a stop code and a user-specified alternate stop code, and a user-specified newline code (defaults to 0x0A---ASCII LF and standard Linux style).
However, you can add your own control codes with :code:`fontlib_SetFirstPrintableCodePoint`.
When any code point less than the first printable code point is encountered, fontlibc stops string processing and returns to allow you to handle the control code yourself using :code:`fontlib_GetLastCharacterRead`.

Transparent Text
~~~~~~~~~~~~~~~~

Part of providing high-performance is not painting a single pixel more than once.
To assist with this goal, fontlibc provides for both transparent and opaque text backgrounds.
Use :code:`fontlib_SetTransparency(true)` if you need to paint text over a background other than a solid color.
If you turn transparency off, however, fontlibc will paint both background and foreground pixels for you, eliminating the time needed to erase those pixels before painting over that area.

Line Spacing
~~~~~~~~~~~~

Since a block of text may not always be the same size, fontlibc provides :code:`fontlib_ClearEOL` for erasing the remainder of a line of text without needing to pad it with spaces.
This action can also be performed automatically after embedded newlines in text and on normal wrapping with :code:`fontlib_SetNewlineOptions`.

Additional blank vertical space around text can improve readability in large blocks of text.
:code:`fontlib_SetLineSpacing` allows you to set this behavior.
Fonts may specify default additional spacing that is automatically applied when calling :code:`fontlib_SetFont`.
In GUIs and games where the benefits of legibility are outweighed by more aggressive use of vertical space, you can force the default spacing to zero after using :code:`fontlib_SetFont` with :code:`fontlib_SetLineSpacing`.

API Documentation
-----------------

.. doxygenfile:: fontlibc.h
   :project: CE C/C++ Toolchain
