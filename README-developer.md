# Developer documentation

Notes:

Coordinate system:
```
^
|
|
|
|
|
|
+----------------------------------------------|->
```

Language tags are first looking up main tag, if matching, use it, otherwise find sumtag, and if still not present, use `en`.

Locales: First ISO-639 language specifier (such as "en" for English, "de" for German, etc). Then country strings in the format YY, where "YY" is an ISO-3166 country code

From discord:

It's generally recommended to const-ref objects of which size extends 2 * sizeof(std::uintmax_t) aka 16 bytes

https://stackoverflow.com/questions/2071621/how-to-do-opengl-live-text-rendering-for-a-gui

## Web
I need to decrease the file size...

You can profile the code with Firefox

You can't enable lto for the linking step, only when compiling or libs break
