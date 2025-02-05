# Text-Renderer
 OpenGL Text Renderer

## Core Flow
1. **`Font`** loads one or more **MiniFont** objects by scanning for files like `TimesNewRoman0.fnt`, `TimesNewRoman1.fnt`, etc.  
2. **`MiniFont`** parses each `.fnt` file, collecting character metrics (positions, offsets) and kerning info.  
3. **`Font`** then combines all `.tga` pages into a single **array texture**, so one texture object can store multiple pages (and multiple sizes).  
4. **`TextBox`** is an object that can display a string inside a bounding rectangle, handling:
   - Word-wrap, forced line breaks (`\n`), and hyphenation if a single word exceeds the width.
   - Selection of the mini-font that best fits the bounding box (trying smaller indexes if needed).
   - Truncation from the bottom if it still exceeds the height.
   - Generating vertex data (quads) for rendering.  
5. **`TextRenderer`** holds references to all `TextBox` objects and a shader (`2d.vsh`, `2d.fsh`) that does the final draw call.  
6. **`TextTable`** provides a simple CSV-based system for localization and placeholder substitution (e.g. `"{playerName}" -> "Alice"`).  

At runtime, the application:
- Initializes the Wolf window/input/GL environment.
- Loads fonts and text boxes, hooking up text via either direct strings or IDs from a CSV file.
- Renders them in an orthographic projection.
- Dynamically re-wraps/truncates text if the content or bounding box changes.

---

## How to add more fonts?
- Open BmFont
- Chooose a font [Since you're going to be repeating these steps start with the largest font you want]
- Export it as a .fnt file and choose .txt font descriptor & .tga for textures 
- Ctrl + S to save it and make sure to place it within "data/Fonts/"
- Name the file a simple name for example "Arial0"
- Repeat the process for each font size and name it accordingly (Arial1, Arial2, etc.)

- Finally, in main write the following line of code to load the font:

    Font* font = m_textRenderer->createFont("TimesNewRoman");

Make sure to use the name only without the number afterwards.
ex. Arial0 would be written as createFont("Arial")

Everything else is done automatically

---

## What has been done

1. **Unified Array Texture**  
   - All pages of a given font are merged into **one** texture array. This greatly simplifies binding in OpenGL and avoids multiple texture switches per letter.

2. **Multiple Font Sizes**  
   - By scanning filenames (`"MyFont0.fnt"`, `"MyFont1.fnt"`, etc.), you take care of all sizes
   - The program automatically picks smaller fonts if text won’t fit the bounding box at size=1.

3. **Wrapping & Truncation**  
   - Single-word overflow triggers **hyphenation**.
   - Forced breaks `\n` are accounted for.  
   - If final text still exceeds the box height, it’s truncated from the bottom with an ellipsis (`"..."`).

4. **TextTable Integration**  
   - Simple CSV-based approach to store multiple languages or dynamic placeholders (e.g., `{scoreValue}`).
   - Automatic re-substitution if the table changes language or placeholder data.

5. **Decoupled Rendering**  
   - `TextRenderer` only draws all boxes; each `TextBox` manages its own geometry.  
   - This keeps it flexible for adding or removing text boxes at runtime.

You get a **scalable** text-rendering system that can handle multiple fonts, multiple languages, on-the-fly changes, and robust 
word-wrapping/truncation — **all** driven by array textures for efficiency.
