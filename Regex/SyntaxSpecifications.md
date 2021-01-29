https://www.cplusplus.com/reference/regex/ECMAScript/

# ECMAScript syntax

## ECMAScript regular expressions pattern syntax
The following syntax is used to construct regex objects.
A regular expression pattern is formed by a sequence of characters.
Regular expression operations look sequentially for matches between the characters of the pattern and the characters in the target sequence: In principle, each character in the pattern is matched against the corresponding character in the target sequence, one by one. But the regex syntax allows for special characters and expressions in the pattern:

### * Special pattern characters
Special pattern characters are characters (or sequences of characters) that have a special meaning when they appear in a regular expression pattern, either to represent a character that is difficult to express in a string, or to represent a category of characters. Each of these special pattern characters is matched in the target sequence against a single character (unless a quantifier specifies otherwise).


### * Quantifiers
Quantifiers follow a character or a special pattern character. They can modify the amount of times that character is repeated in the match:

| characters    | times         | effects |
| ------------- | ------------- | ------- |
| `*`           | 0 or more     | The preceding atom is matched 0 or more times |
| `+`           | 1 or more     | The preceding atom is matched 1 or more times |
| `?`           | 0 or 1        | The preceding atom is optional (matched either 0 times or once) |

By default, all these quantifiers are greedy (i.e., they take as many characters that meet the condition as possible).

### * Groups
Groups allow to apply quantifiers to a sequence of characters (instead of a single character). There are two kinds of groups:

| characters        | times         | effects |
| ----------------- | ------------- | ------- |
| `(`subpattern`)`  | Separator     | Separates two alternative patterns or subpatterns |


### * Alternatives
A pattern can include different alternatives:

| characters    | times         | effects |
| ------------- | ------------- | ------- |
| `|`           | Separator     | Separates two alternative patterns or subpatterns |

A regular expression can contain multiple alternative patterns simply by separating them with the separator operator `|`: The regular expression will match if any of the alternatives match, and as soon as one does.
Subpatterns (in groups or assertions) can also use the separator operator to separate different alternatives.
