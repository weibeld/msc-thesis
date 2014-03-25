/*
 * famod.h: finite automata (modified)
 *
 * Copyright (C) 2007-2011 David Lutterkort
 * Copyright (C) 2013 Christian Göttel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: David Lutterkort <dlutter@redhat.com>
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

#ifndef FAMOD_H_
#define FAMOD_H_

#include <libefa/efa.h>

#include <regex.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  
/*
 * fa_boltzmann_samplers:
 * An automaton generating algorithm.
 */ 
#if HAVE_MODULE_HASH
int fa_hash_boltzmann_samplers(fa_t *);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_boltzmann_smaplers(fa_t *);
#endif

/*
 * fa_brzozowski:
 * An automaton minimization algorithm.
 */
#if HAVE_MODULE_HASH
int fa_hash_brzozowski(fa_t *);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_brzozowski(fa_t *);
#endif

/* Unless otherwise mentioned, automata passed into routines are never
 * modified. It is the responsibility of the caller to free automata
 * returned by any of these routines when they are no longer needed.
 */

/*
 * Compile the regular expression RE of length SIZE into an automaton. The
 * return value is the same as the return value for the POSIX function
 * regcomp. The syntax for regular expressions is extended POSIX syntax,
 * with the difference that '.' does not match newlines.
 *
 * On success, FA points to the newly allocated automaton constructed for
 * RE, and the function returns REG_NOERROR. Otherwise, FA is NULL, and the
 * return value indicates the error.
 *
 * The FA is case sensitive. Call FA_NOCASE to switch it to
 * case-insensitive.
 */
//int fa_compile(const char *re, size_t size, struct fa **fa);

/* Return a finite automaton that accepts the concatenation of the
 * languages for FA1 and FA2, i.e. L(FA1).L(FA2)
 */
//struct fa *fa_concat(struct fa *fa1, struct fa *fa2);

/* Return a finite automaton that accepts the union of the languages that
 * FA1 and FA2 accept (the '|' operator in regular expressions).
 */
//struct fa *fa_union(struct fa *fa1, struct fa *fa2);

/* Return a finite automaton that accepts the intersection of the languages
 * of FA1 and FA2.
 */
//struct fa *fa_intersect(struct fa *fa1, struct fa *fa2);

/* 
 * fa_collect:
 * Removes all non-reachable states of the finite automaton.
 */
#if HAVE_MODULE_HASH
int fa_hash_collect(fa_t *);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_collect(fa_t *);
#endif

/* 
 * fa_complement:
 * Complements the finite automaton.
 */
#if HAVE_MODULE_HASH
int fa_hash_complement(fa_t *, unsigned char);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_complement(fa_t *, unsigned char);
#endif

/* 
 * fa_complete:
 * Return a complete finite automaton.
 */
#if HAVE_MODULE_HASH
int fa_hash_complete(fa_t *);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_complete(fa_t *);
#endif

/*
 * fa_copy_sos:
 * Phyiscally copy a deterministic set of states.
 */
#if HAVE_MODULE_HASH
hnode_t *fa_hash_copy_sos(hnode_t *, unsigned int);
#endif
#if HAVE_MODULE_RBTREE
rbnode_t *fa_rbtree_copy_sos(rbnode_t *, unsigned int);
#endif

/*
 * fa_create_insert_sos:
 * Creates and inserts a set of states from a list of states.
 */
#if HAVE_MODULE_HASH
int fa_hash_create_insert_sos(hnlst_t *, hash_t *, hnode_t **);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_create_insert_sos(rbnlst_t *, rbtree_t *, rbnode_t **);
#endif

/*
 * fa_create_sos:
 * Creates a set of states from a list of states.
 */
#if HAVE_MODULE_HASH
hnode_t *fa_hash_create_sos(hnlst_t *);
#endif
#if HAVE_MODULE_RBTREE
rbnode_t *fa_rbtree_create_sos(rbtree_t *);
#endif

/*
 * fa_create_sos_label:
 * Creates a label for a set of states from a list of states.
 */
#if HAVE_MODULE_HASH
const char *fa_hash_create_sos_label(hnlst_t *);
#endif
#if HAVE_MODULE_RBTREE
const char *fa_rbtree_create_sos_label(rbnlst_t *);
#endif

/*
 * fa_equivalence:
 * Tests two finite automata for equivalence.
 */
#if HAVE_MODULE_HASH
int fa_hash_equivalence(fa_t *, fa_t *);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_equivalence(fa_t *, fa_t *);
#endif

/* Return a finite automaton that accepts the set difference of the
 * languages of FA1 and FA2, i.e. L(FA1)\L(FA2)
 */
//struct fa *fa_minus(struct fa *fa1, struct fa *fa2);

/* Return a finite automaton that accepts a repetition of the language that
 * FA accepts. If MAX == -1, the returned automaton accepts arbitrarily
 * long repetitions. MIN must be 0 or bigger, and unless MAX == -1, MIN
 * must be less or equal to MAX. If MIN is greater than 0, the returned
 * automaton accepts only words that have at least MIN repetitions of words
 * from L(FA).
 *
 * The following common regexp repetitios are achieved by the following
 * calls (using a lose notation equating automata and their languages):
 *
 * - FA* = FA_ITER(FA, 0, -1)
 * - FA+ = FA_ITER(FA, 1, -1)
 * - FA? = FA_ITER(FA, 0, 1)
 * - FA{n,m} = FA_ITER(FA, n, m) with 0 <= n and m = -1 or n <= m
 */
//struct fa *fa_iter(struct fa *fa, int min, int max);

/* Return 1 if the language of FA1 is contained in the language of FA2, 0
 * otherwise.
 */
//int fa_contains(struct fa *fa1, struct fa *fa2);

/* Return 1 if the language of FA1 equals the language of FA2 */
//int fa_equals(struct fa *fa1, struct fa *fa2);

/* Return a finite automaton that accepts the overlap of the languages of
 * FA1 and FA2. The overlap of two languages is the set of strings that can
 * be split in more than one way into a left part accepted by FA1 and a
 * right part accepted by FA2.
 */
//struct fa *fa_overlap(struct fa *fa1, struct fa *fa2);

/* Convert the finite automaton FA into a regular expression and set REGEXP
 * to point to that. When REGEXP is compiled into another automaton, it is
 * guaranteed that that automaton and FA accept the same language.
 *
 * The code tries to be semi-clever about keeping the generated regular
 * expression short; to guarantee reasonably short regexps, the automaton
 * should be minimized before passing it to this routine.
 *
 * On success, REGEXP_LEN is set to the length of REGEXP
 *
 * Return 0 on success, and a negative number on failure. The only reason
 * to fail for FA_AS_REGEXP is running out of memory.
 */
//int fa_as_regexp(struct fa *fa, char **regexp, size_t *regexp_len);

/* Given the regular expression REGEXP construct a new regular expression
 * NEWREGEXP that does not match strings containing any of the characters
 * in the range FROM to TO, with the endpoints included.
 *
 * The new regular expression is constructed by removing the range FROM to
 * TO from all character sets in REGEXP; if any of the characters [FROM,
 * TO] appear outside a character set in REGEXP, return -2.
 *
 * Return 0 if NEWREGEXP was constructed successfully, -1 if an internal
 * error happened (e.g., an allocation failed) and -2 if NEWREGEXP can not
 * be constructed because any character in the range [FROM, TO] appears
 * outside of a character set.
 *
 * Return a positive value if REGEXP is not syntactically valid; the value
 * returned is one of the REG_ERRCODE_T POSIX error codes. Return 0 on
 * success and -1 if an allocation fails.
 */
/*int fa_restrict_alphabet(const char *regexp, size_t regexp_len,
                         char **newregexp, size_t *newregexp_len,
                         char from, char to);*/

/* Generate up to LIMIT words from the language of FA, which is assumed to
 * be finite. The words are returned in WORDS, which is allocated by this
 * function and must be freed by the caller.
 *
 * If FA accepts the empty word, the empty string will be included in
 * WORDS.
 *
 * Return the number of generated words on success, -1 if we run out of
 * memory, and -2 if FA has more than LIMIT words.
 */
//int fa_enumerate(struct fa *fa, int limit, char ***words);

/*
 * fa_hopcroft:
 * A minimization algorithm for finite automata.
 */
#if HAVE_MODULE_HASH
int fa_hash_hopcroft(fa_t *);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_hopcroft(fa_t *);
#endif

/*
 * fa_nfa_move:
 * Move from a given set of states to a new set of states with a given symbol.
 * Returns 0 on success and 1 on failure.
 * Note: The set of states returned by this function can be the empty set, e.g. 
 * OUT_SET will be NULL and return value of the function is 0.
 */
#if HAVE_MODULE_HASH
int fa_hash_nfa_move(hash_t *, hnode_t *, hnode_t **, unsigned int);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_nfa_move(rbtree_t *, rbnode_t *, rbnode_t **, unsigned int);
#endif

/*
 * fa_run:
 * An algorithm to test a finite automaton for a word.
 */
#if HAVE_MODULE_HASH
int fa_hash_run(fa_t *, const char *);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_run(fa_t *, const char *);
#endif

/*
 * fa_recursive_method:
 * A recursive method to generate finite automata.
 */
#if HAVE_MODULE_HASH
int fa_hash_recursive_method(fa_t *);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_recursive_method(fa_t *);
#endif

/*
 * fa_subset_construction:
 * Subset construction algorithm to transform non-deterministic finite automata 
 * into deterministic finite automata.
 */
#if HAVE_MODULE_HASH
int fa_hash_subset_construction(fa_t *);
#endif
#if HAVE_MODULE_RBTREE
int fa_rbtree_subset_construction(fa_t *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FAMOD_H_ */
