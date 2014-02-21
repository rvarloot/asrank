asrank
======

Implementation of CAIDA AS ranking algorithm released under the CeCILL license.

=====
Usage

  asrank [--ixp ixpFile] [--rel relationshipFile] [--clique cliqueFile] file1 [file2 ...]

Description

  Computes the types of relationships between all the ASs visible on the Internet.

  Output: asrank outputs the relationships (CAIDA format) on the standard output.

Options
  
  --ixp ixpFile
    ixpFile contains a list of AS numbers corresponding to Internet Exchange Points.
    Two AS numbers can be separated by a blank or newline character.
    Multiple files may be given (each must be precede by --ixp).
    The '#' character comments the rest of the line it is on.
  
  --rel relationshipFile
    relationshipFile contains a list of AS relationships, using CAIDA format.
    Each relationship must be on a separate line.
    Multiple files may be given (each must be precede by --rel).
    The '#' character comments the rest of the line it is on.
  
  --clique cliqueFile
    cliqueFile contains a list of AS numbers corresponding to Tier 1 providers.
    Two AS numbers can be separated by a blank or newline character.
    Only one file may be given (in case multiple files are given, the last one is used).
    The '#' character comments the rest of the line it is on.
  
  file1 file2 ...
    These files contain AS paths.
    The format is one AS path per line, with each AS separated by a space (no prefix).
    At least one file must be provided.
    The '#' character comments the rest of the line it is on.
    A list of AS path can be retrieved from the bgpdump tool output.

CAIDA format

  a|b|r

  a and b : AS numbers
  r : relationship

  -1 if a is a provider of b
  0  if a is a peer of b
  1  if a is a customer of b
  2  if a is a sibling of b
  3  if the type of relationship could not be inferred.

TODO

  Include options for controlling the output (e.g. clique only, customer cones, etc.).
  Check for failure when opening files.
