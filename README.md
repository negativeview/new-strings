# new-strings

An experiment into a "better" way to do strings. Might eventually make its way into one of my language experiments, if they prove themselves useful here.

Note that the API for these strings aren't designed to necessarily be nice to use. I don't expect a "normal user" to use these -- I expect to eventually use a compiler to generate usage of this API. It has to be composable, not necessarily nice.

## What is a template string?

You've seen template strings in fancy dynamic/scripting languages. For instance: `$foo = "World"; $bar = "Hello $foo";`

The point of this project is to create a type/api for a string that is composed of pieces. Some of those pieces are static -- they are the "Hello" above. Some of those pieces are able to be dynamically replaced -- the "$foo" part above.