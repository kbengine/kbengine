<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">

<head>
<meta content="text/html; charset=utf-8" http-equiv="Content-Type" />
<title>nedtries Readme</title>
<style type="text/css">
<!--
body {
	text-align: justify;
}
h1, h2, h3, h4, h5, h6 {
	margin-bottom: -0.5em;
}
h1 {
	text-align: center;
}
h2 {
	text-decoration: underline;
	margin-bottom: -0.25em;
}
p {
	margin-top: 0.5em;
	margin-bottom: 0.5em;
}
pre {
	background: #eee;
	width: 80%;
	margin-left:auto;
	margin-right:auto;
}
ul li, ol li {
	margin-top: 0.2em;
	margin-bottom: 0.2em;
}
dl {
	margin-left: 2em;
}
dl dt {
	font-weight: bold;
}
dt + dd {
	margin-bottom: 1em;
}
.gitcommit {
	font-family: "Courier New", Courier, monospace;
	font-size: smaller;
}
table {
    border: 0;
}
th {
	text-align: center;
}
td {
    border-color: lightgray;
	border-style: solid;
	border-width: 1px;
}
td ul {
	margin-right: 2em;
}
-->
</style>
</head>

<body>

<div style="text-align: center">
	<h1 style="text-decoration: underline">nedtries v1.02 Final (9th July 2012)</h1>
	<h2 style="text-decoration: none;">by Niall Douglas</h2>
	<p>Web site: <a href="http://www.nedprod.com/programs/portable/nedtries/">http://www.nedprod.com/programs/portable/nedtries/</a></p>
	<hr /></div>
<p>Enclosed is nedtries, an in-place bitwise binary Fredkin trie algorithm which allows for near 
constant time insertions, deletions, finds,
<span style="text-decoration: underline"> <strong>closest fit finds</strong></span> 
and iteration. On modern hardware it is approximately 50-100% faster than 
red-black binary trees, it handily beats even the venerable O(1) hash table for 
less than 3000 objects and it is barely slower than the hash table for 10000 
objects. Past 10000 objects you probably ought to use a hash table though, and 
if you need <a href="#nfindversuscfind">nearest fit rather than close fit</a> 
then red-black trees are still optimal.</p>
<p>It is licensed under the
<a href="http://www.boost.org/LICENSE_1_0.txt" target="_blank">Boost Software License</a> 
which basically means you can do anything you like with it. Commercial support is 
available from <a href="http://www.nedproductions.biz/" target="_blank">ned 
Productions Limited</a>.</p>
<p>Its advantages over other algorithms are sizeable:</p>
<ol>
	<li>It has all the advantages of red-black trees such as close-fit finds 
	(i.e. find an item which is similar but not exact to the search term) without 
	anything like the impact on memory bandwidth as red-black trees because it 
	doesn't have to rebalance itself when adding new items (i.e. it 
	scales far better with memory pressure than red-black trees).</li>
	<li>It doesn&#39;t require dynamic memory like hash tables, so it can be used in 
	a bounded environment such as a bootstrapper or a tiny embedded systems 
	kernel. It is also a lot faster than hash tables for less than a few 
	thousand items.</li>
	<li>Unlike either red-black trees or most hash tables, nedtries can store as 
	many items with identical keys as you like.</li>
	<li>Its performance is nearly perfectly stable over time and number of 
	contents N with a worst case complexity of O(M) following a mostly linear 
	degradation with increasing M average where 1 &lt;= M &lt;= 8*sizeof(void *). M is a measure 
	of the entropy between differing keys, so where keys are very similar at the 
	bit level M is 
	higher than where keys are very dissimilar. This leads to an unusual 
	complexity where it can be like O(log N) for some distributions of key, and 
	like O(1) for other distributions of key. The scaling graphs below are for 
	completely random keys.</li>
	<li>Its complexities for find and insert are identical, whereas for deletion 
	it is slightly more constant. Unlike almost any other algorithm, bitwise binary tries 
	have nearly identical real world speeds for ALL its operations rather than 
	being fast at one thing but slow at the others. In other words, if your code 
	equally inserts, deletes and finds with no preference for which then 	<span style="text-decoration: underline">this algorithm will 
	typically beat all others 
	in the general purpose situation</span>.</li>
</ol>
<p>Its two primary disadvantages are that (i) it can only key upon a size_t (i.e. the size 
of a void *), so it cannot make use of an arbitrarily large key like a hash 
table can (though of course one could hash the large key into a size_t sized 
key) and (ii) it is lousy at guaranteed nearest fit finds. It also runs fastest when the key is as unique 
from other keys as possible, so if you wish 
to replace a red-black tree which has a complex left-right comparison function 
which cannot be converted into a stable size_t value then you will need to stick 
with red-black trees. In other words, it is ideal when you are keying on pointer 
sized keys where each item has a definitive non-changing key.</p>
<p>Have a look at the scaling graphs below to decide if your software could 
benefit. Note for non-random key distributions you may get significantly better 
or worse performance than shown. If you're interested, read on for how to add them to your software.</p>
<div><center>
<img alt="Bitwise Trees Scaling" height="25%" src="images/BitwiseTreesScaling.png" width="25%" /><img alt="Red Black Trees Scaling" height="25%" src="images/RedBlackTreesScaling.png" width="25%" /><img alt="Hash Table Scaling" height="25%" src="images/HashTableScaling.png" width="25%" /></center></div>
<div><center><img alt="Bitwise Trees Scaling" height="25%" src="images/LogLogBitwiseTreesScaling.png" width="25%" /><img alt="Red Black Trees Scaling" height="25%" src="images/LogLogRedBlackTreesScaling.png" width="25%" /><img alt="Hash Table Scaling" height="25%" src="images/LogLogHashTableScaling.png" width="25%" /></center></div>
<h2><a name="breakingchanges">A. Breaking changes since previous versions:</a></h2>
<p>In v1.02 a breaking change in what NEDTRIE_NFIND means and does was 
introduced, and the version was bumped to v1.02 to warn users of the 
change. The problem in v1.01 and earlier was that I had incorrectly 
documented what NEDTRIE_NFIND does: I said that it found the nearest item to the 
search term when this was patently untrue (thanks to smilingthax for reporting 
this). In fact, NEDTRIE_NFIND used to return a matching item, if there was one, 
but if there was no matching item, it returned <em>any</em> larger item rather 
than <em>the</em> next largest item. I do apologise to the users of nedtries for 
this documentation error, and for the lost productivity it surely must have 
caused some of you.</p>
<p>The good news is that NEDTRIE_NFIND now guarantees to return the next largest 
item, and it therefore now matches BSD's red-black Nfind. The bad news is that 
bitwise tries are really not ideal for guaranteed nearest matching, and 
performance is terrible as you can see via the purple line in the graphs above. 
If you can put up with non-guaranteed nearest matching, NEDTRIE_CFIND offers 
much better performance. NEDTRIE_CFIND takes a <em>rounds</em> parameter which 
indicates how hard the routine should try to return a close item: rounds=0 means 
to return the first item encountered which is equal or larger to search key, 
rounds=1 means try one level down, rounds=2 means try two levels down and so on. 
rounds=INT_MAX means try hardest, and guarantees that any item with a matching 
key will be found and that if not matching, any item returned will have a very 
close key (those not necessarily the closest).</p>
<p>For a summary of the differences between Nfind and Cfind,
<a href="#nfindversuscfind">see this useful table</a>. Note that if you just 
want any item with a key larger or equal to the search key, 
NEDTRIE_CFIND(rounds=0|1|2) is extremely swift and has O(1) complexity as shown 
in the graphs above.</p>
<h2><a name="implementation">B. Implementation:</a></h2>
<p>The source makes use of C macros on C and C++ templates on C++ - therefore, 
unlike typical C-macro-based algorithms it is easy to debug and in fact, the improved 
metadata specified by the templates lets a modern C++ compiler produce 5-15% 
faster code through PGO guided selective inlining. The code is 100% standard C and C++, so it should run on any 
platform or architecture though you <em>may</em> need to implement your own nedtriebitscanr() 
function if you&#39;re not using GCC nor MSVC and want to keep performance high. If 
you are building debug, NEDTRIEDEBUG is by default turned on: this causes a 
complete state validation check to be performed after each and every change to 
the trie which tends to be very good at catching bugs early, but can make debug 
builds a little slow.</p>
<p>So what is &quot;an in-place bitwise binary Fredkin trie algorithm&quot; then? 
Well you ought to start by reading and fully digesting
<a href="http://en.wikipedia.org/wiki/Trie" target="_blank">the Wikipedia page 
on Fredkin tries</a> as what comes won&#39;t make much sense otherwise. The 
Wikipedia page describes a non-inplace trie which uses dynamic memory to store 
each consecutive non-differing section of a string, and indeed this is how tries 
are normally described in algorithm theory and classes. nedtries obviously 
enough selects on individual bits rather than substrings, and it uses an inplace 
instead of dynamically allocated implementation.</p>
<p>Here is how nedtries performs an indexation: firstly, the most significant 
set bit X is found using nedtriebitscanr() which is no more than one to three 
CPU cycles on modern processors. This is used to index an array of 
bins. Each bin X contains a binary tree of items whose keys are (1&lt;&lt;X) &lt;= key &lt; 
(1&lt;&lt;(X+1)), so what one does is to follow the tree downwards selecting left or 
right based on whether the next bit downwards is 0 or 1. If an item has 
children, its key is only guaranteed to be constrained to that of its bin, 
whereas if an item does not have children then its key is guaranteed to match as 
closely as possible its position in the tree.</p>
<p>If you insert an item, nedtries indexes as far as it can down the existing 
tree where the new item ought to be and inserts it there. If you remove an item, 
if that item has no children it is simply removed. If it has a child then a 
nobble function is called to select the bias for how to select the childless 
item to be nobbled and used as the replacement i.e. one either traverses down 
preferentially 1 or preferentially 0 until you find a childless item, then you 
delink it from there and link it in to replace the item being removed.</p>
<p>If you think about this hard enough, you realise that you will get a &quot;nearly 
sorted&quot; binary tree i.e. one whose node keys are very nearly in order. In fact, 
the more randomised the key, the more in order the tree becomes. The tree is 
usually sufficiently ordered that one can assume it to be so for most 
operations, but if you need to guarantee order then you can bubble sort per MSB 
bin as bubble sort performs <em>very</em> well on nearly sorted data (as does 
smooth sort if you have a very large set of data) .</p>
<p>The enclosed benchmark.cpp will run a series of scalability tests comparing the bitwise 
binary trie implementation from nedtries with others outputting its results in 
CSV format:</p>
<ul>
	<li>If compiled as C not C++, the C macro version of nedtries is compared to the red-black binary tree 
implementation from FreeBSD and the O(1) hash table implementation from
	<a href="http://uthash.sourceforge.net/">http://uthash.sourceforge.net/</a>.</li>
	<li>If compiled as C++, the C++ template version of nedtries is compared to 
	all of the C tests above as well as the STL associative container classes 
	std::map&lt;&gt; and std::unordered_map&lt;&gt;. NOTE THAT YOU NEED A TR1 SUPPORTING 
	COMPILER FOR std::unordered_map&lt;&gt; SUPPORT!</li>
</ul>
<p>You will also find enclosed a set of precomputed 
Microsoft Excel spreadsheets which were generated on a 2.67Ghz Intel Core 2 Quad 
Windows 7 x64 machine. They should be representative of performance on modern 
hardware - though note that the Intel Atom has a 17 cycle nedtriebitscanr() 
which is the only modern CPU to be so slow. See
<a href="http://gmplib.org/~tege/x86-timing.pdf">http://gmplib.org/~tege/x86-timing.pdf</a> for x86 and x64 instruction timings.</p>
<h2><a name="c-usage">C. C Macro Usage:</a></h2>
<p>Usage via C macros follows the FreeBSD <a href="rbtree.h">rbtree.h</a> 
format. See the enclosed <a href="nedtries.chm">nedtries.chm</a>  for detailed 
API documentation. Here is some sample code which can be compiled cleanly using <tt>gcc -Wall -pedantic 
-std=c99 test.c</tt> (or as C++ via <tt>g++ -Wall -pedantic test.c</tt>):</p>
<pre>#include &lt;stdio.h&gt;
#include &lt;string.h&gt;
#include &quot;nedtrie.h&quot;

typedef struct foo_s foo_t;
struct foo_s {
  NEDTRIE_ENTRY(foo_s) link;
  size_t key;
};
typedef struct foo_tree_s foo_tree_t;
NEDTRIE_HEAD(foo_tree_s, foo_s);
static foo_tree_t footree;

size_t fookeyfunct(const foo_t *r)
{
  return r-&gt;key;
}

NEDTRIE_GENERATE(static, foo_tree_s, foo_s, link, fookeyfunct, NEDTRIE_NOBBLEZEROS(foo_tree_s))

int main(void)
{
  foo_t a, b, c, *r;
  NEDTRIE_INIT(&amp;footree);
  a.key=2;
  NEDTRIE_INSERT(foo_tree_s, &amp;footree, &amp;a);
  b.key=6;
  NEDTRIE_INSERT(foo_tree_s, &amp;footree, &amp;b);
  r=NEDTRIE_FIND(foo_tree_s, &amp;footree, &amp;b);
  assert(r==&amp;b);
  c.key=5;
  r=NEDTRIE_NFIND(foo_tree_s, &amp;footree, &amp;c);
  assert(r==&amp;b); /* NFIND finds next largest. Invert the key function (i.e. 1-key) to find next smallest. */
  NEDTRIE_REMOVE(foo_tree_s, &amp;footree, &amp;a);
  NEDTRIE_FOREACH(r, foo_tree_s, &amp;footree)
  {
    printf(&quot;%p, %u\n&quot;, (void *) r, (unsigned) r-&gt;key);
  }
  assert(!NEDTRIE_PREV(foo_tree_s, &amp;footree, &amp;b));
  assert(!NEDTRIE_NEXT(foo_tree_s, &amp;footree, &amp;b));
  return 0;
}</pre>
<p>There isn&#39;t really much more to it - if you want to throw away the trie, 
simply NEDTRIE_INIT() its head. As no dynamic memory is involved, nothing is 
lost.</p>
<h3><a name="choosingthenobblefunction">Choosing The Nobble Function</a></h3>
<p>I should mention what the nobble function is for: you have three default 
choices, NEDTRIE_NOBBLEZEROS, NEDTRIE_NOBBLEONES and NEDTRIE_NOBBLEEQUALLY 
though you can of course also define your own. The nobble function contributes 
to tree balance by working <em>against</em> bit bias in your keys, so if your 
keys contain an excess of<em> non-leading </em>zeros then you should 
preferentially nobble zeros. Equally if your keys contain an excess of ones, 
then you should preferentially nobble ones and, as you might have guessed, if 
your bits <em>after the first set bit</em> are completely random (which is rare) 
then you should nobble equally.</p>
<p>Sounds complicated? In fact it&#39;s very easy if you simply use trial &amp; error. 
Start with nobble zeroes which tends to be right in most situations, and then 
use benchmarking your code to determine the correct setting.</p>
<h3><a name="nfindversuscfind">Nfind versus Cfind</a></h3>
<p>Where the BSD red-black tree implementation has RB_NFIND() for finding items 
which are nearest to the search term, nedtries provides NEDTRIE_CFIND() and 
NEDTRIE_NFIND(). What's the difference? Here's a quick table:</p>
<table style="width: 80%; position:relative; margin-left:auto; margin-right:auto;">
	<tr>
		<th style="width: 33%" >Nfind</th>
		<th style="width: 33%" >Cfind(rounds=INT_MAX)</th>
		<th style="width: 33%" >Cfind(rounds=0|1)</th>
	</tr>
	<tr>
		<td style="width: 33%" valign="top">
		<ul>
			<li>Returns an exact match if there is an exact match in the trie</li>
			<li>If there is not an exact match, guarantees that the item 
			returned is the next largest</li>
			<li>Complexity: Somewhat worse than O(log N), as must perform a O(log N) 
			search of the subtree returned by Cfind(rounds=INT_MAX).</li>
		</ul>
		</td>
		<td style="width: 33%" valign="top">
		<ul>
			<li>Returns an exact match if there is an exact match in the trie</li>
			<li>If there is not an exact match, item returned will be very close 
			to the next largest</li>
			<li>Complexity: Identical to Find i.e. O(1/D<sub>KL</sub>(key||average key)). 
			However because it does much more work, it is approximately four 
			times slower than a straight find.</li>
		</ul>
		</td>
		<td style="width: 33%" valign="top">
		<ul>
			<li>Returns an item with a key no larger than the next power of two 
			multiple higher than the search key</li>
			<li>Complexity: approximately O(2^rounds) for small numbers of 
			rounds, so approaches O(1).</li>
		</ul>
		</td>
	</tr>
</table>
<h2><a name="cpp-usage">D. C++ Usage:</a></h2>
<p>C++ usage is even easier than the C macro usage thanks to nedtries::trie_map&lt;&gt; 
and nedtries::trie_multimap&lt;&gt; 
which is API compatible with the std::map&lt;&gt;, std::multimap&lt;&gt; and std::unordered_map&lt;&gt; 
STL associative containers. trie_map&lt;&gt; and trie_multimap&lt;&gt; 
makes full use of rvalue construction if either you are running on C++0x 
according to the value of __cplusplus, or have 
defined HAVE_CPP0X. In the general case, simply drop trie_map&lt;&gt; or 
trie_multimap&lt;&gt; in where 
your STL associative container used to be and enjoy the speed benefits!</p>
<p>Note that insertion and deletion speed in <strong>any</strong> STL container 
is heavily bound by the speed of your memory allocator. You may wish to consider 
employing
<a href="http://www.nedprod.com/programs/portables/nedmalloc/" target="_blank">
nedmalloc</a> which can deliver some unholy speed benefits if you run it as 
root, otherwise it will need some small source changes to employ its advanced v2 
malloc API.</p>
<p>In case you are not familiar with STL associative containers, they are very 
simple e.g.:</p>
<pre>nedtries::trie_map&lt;size_t, Foo&gt; foomap;
foomap[5]=Foo();
foomap.erase(foomap.find(5));</pre>
<p>You can of course iterate through them and do all the normal things you can 
do with any STL container.</p>
<h3>The trie_map&lt;&gt; and trie_multimap&lt;&gt; Implementation</h3>
<p>trie_map&lt;&gt; and trie_multimap&lt;&gt; are actually a STL container <em>wrappers</em> rather than a proper STL 
container in its own right i.e. it subclasses an existing STL container passing 
through most of its API, but selectively overrides certain members. Its default 
parameters point at std::list&lt;&gt; which is its most likely usage model for most 
people.</p>
<p>The advantages are mainly that it is quick to implement and can be 
theoretically applied to any arbitrary STL container, thus taking advantage of 
that STL container&#39;s optimisations and customisations. The big disadvantage is 
that it is hacky, dirty and prone to getting bugs into it, and if you look at 
the source you&#39;ll see what I mean. There is after all a number of places where I 
am doing a number of very illegal things in C++ which just happen to usually 
work.</p>
<p>The chances are that this implementation will be good enough for most people. 
If however you might like to sponsor the development of a full bitwise trie STL 
associative container for submission to
<a href="http://www.boost.org/" target="_blank">the Boost C++ peer reviewed 
libraries</a> (and thereafter into the standard C++ language itself), I would be 
very pleased to oblige. <a href="http://www.nedproductions.biz/" target="_blank">
Please contact ned Productions Consulting Ltd. for further details</a>.</p>
<h2><a name="changelog">E. ChangeLog:</a></h2>
<h3>v1.02 Final (9th July 2012):</h3>
<ul>
	<li>Due to the breaking 
	change in what NEDTRIE_NFIND means and does, bumped version number.</li>
	<li><span class="gitcommit">[master 910ef60]</span> Fixed the C++ 
	implementation causing memory corruption when built as 64 bit.</li>
	<li><span class="gitcommit">[master 0ea1327]</span> Added some compile time 
	checks to ensure C++ 
	implementation will never again cause memory corruption when built as 64 bit.</li>
</ul>
<h3>v1.01 RC2 (unreleased):</h3>
<ul>
	<li><span class="gitcommit">[master bd6f3e5]</span> Fixed misc documentation errors.</li>
	<li><span class="gitcommit">[master 79efb3a]</span> Fixed really obvious 
	documentation bug in the example usage. Thanks to Fabian Holler for 
	reporting this.</li>
	<li><span class="gitcommit">[master 99e67e3]</span> Fixed that the example 
	usage in the documentation spews warnings on GCC. Now compiles totally 
	cleanly. Thanks to Fabian Holler for 
	reporting this.</li>
	<li><span class="gitcommit">[master 537c27b]</span> Added NEDTRIE_FOREACH_SAFE
	and NEDTRIE_FOREACH_REVERSE_SAFE. Thanks to Stephen Hemminger for contributing this.</li>
	<li><span class="gitcommit">[master 4b12a3c]</span> Fixed the fact that I 
	had forgotten to implement iterators for trie_map&lt;&gt;. Also added 
	trie_multimap&lt;&gt;. Thanks to Ned for pointing out the problem.</li>
	<li><span class="gitcommit">[master 8b53224]</span> Renamed Nfind to Cfind.</li>
</ul>
<h3>v1.01 RC1 (19th June 2011):</h3>
<ul>
	<li><span class="gitcommit">[master 30a440a]</span> Fixed misc documentation errors.</li>
	<li><span class="gitcommit">[master 2103969]</span> Fixed misoperation when trie key is
zero. Thanks to Andrea for reporting this.</li>
	<li><span class="gitcommit">[master 083d94b]</span> Added support for MSVC's as old as 7.1.</li>
	<li><span class="gitcommit">[master f836319]</span> Added Microsoft CLR target support.</li>
	<li><span class="gitcommit">[master 85abf67]</span> I, being a muppet of the highest
order, was actually benchmarking the speed of the timing routines rather than much
else. Performance is now approx. 10x higher in the graphs ... I am a fool!</li>
	<li><span class="gitcommit">[master 6aa344e]</span> Added check for key uniqueness
in benchmark test (hash tables suffer is key isn't unique). Added cube root averaging
to results output.</li>
	<li><span class="gitcommit">[master feb4f56]</span> Replaced the use of rand()
with the Mersenne Twister (<a href="http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/index.html">
http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/index.html</a>).</li>
</ul>
<h3>v1.00 beta 1 (18th June 2010):</h3>
<ul>
	<li><span class="gitcommit">[master e4d1245]</span> First release.</li>
</ul>

</body>

</html>
