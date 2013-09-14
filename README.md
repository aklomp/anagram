Anagram is a Linux commandline utility to find anagrams of words and sentences.
An anagram a word or set of words made up of the same letters as the input. For
example, an anagram of the string `hello world` is `oh well, lord`, because it
contains the same letters as the input.

Simple usage:

```sh
./anagram "hello world"
```

This will print a list of anagrams of the sentence `hello world` to standard
output. Or read from standard input:

```sh
echo "hello world" | ./anagram
```

The program has a couple of switches which place restrictions on the output.
For instance, running:

```sh
./anagram -m 3 -l 5 "hello world"
```

will print a list of anagrams of the string `hello world` where all words are
at least three characters long, and the anagram contains at least one word of
length five. Playing with these options tends to weed out less interesting
anagrams made of all short words. See "Options" for more details.

Anagram is currently not aware of anything other than ASCII characters. It also
doesn't know the difference between uppercase and lowercase, or any punctuation
other than spaces and tabs. It will strip out spaces, but looks for exact
matches for all other characters.

## Options

- `-f|--wordfile <dictfile>`: use the given file instead of the default system
  dictionary file for the input words. Dictionary files have one word per line.
  The default dictionary file is `/usr/share/dict/words`.

- `-m|--minlength <length>`: all words in the anagram must be at least this
  long. Defaults to 1. Set to larger values if you want to skip short words
  like 'an', 'do', and so on. By passing this option, anagram can skip these
  words at the dictionary file parsing stage, which saves processing time.

- `-l|--haslength <length>`: the output anagram must contain a word at least
  this long. The default case tends to produce anagrams made up of a lot of
  two- or three-letter words. Set this to something higher than the default of
  1 to get more interesting anagrams. 

## Internals

Anagram is written in C (specifically, C89), and compiles with the compiler set
to its most pedantic. It has been profiled with Valgrind and with LLVM's
`scan-build` tool.

The code tries its hardest to do the least amount of work. It first transforms
the input string into a histogram, by which we mean an alphabetized array of
characters and their frequency. The string "hello" would be parsed into
something like "(e,1) (h,1) (l,2) (o,1)". Then the code parses the dictionary
file. For each word, it checks if it's longer than the input string. If so, it
can't possibly be (part of) an anagram of the input string, so it's ignored.
Same if the word's highest letter frequency is higher than that of the input.
If the word contains any letters not in the input, or at a higher frequency
than in the input, it's also ignored. Words that remain are added to a linked
list.

During the search phase, the quest to do as little as possible continues. The
code uses a recursive search to find sequences of words whose combined
histograms fit exactly into the input sequence's histogram. If a prospective
word is shorter than the minimum length, is too long, has a too high max
frequency in its histogram or contains superfluous letters, the word is
ignored. The code walks the word list recursively. When it finds a word whose
histogram "fits" into that of the input, it subtracts its histogram from the
input histogram and recurses. If after the subtraction the input histogram is
empty, a full anagram was found and the sequence of words is printed in order.

The result is code that is fairly fast for what it does, but still does not
scale well for even small inputs (say 15 characters or so) because of its naive
approach. For production purposes, you might prefer something based on
perturbation algorithms.

## License

This code is licensed under the
[GPL version 3](https://www.gnu.org/licenses/gpl-3.0.html).
