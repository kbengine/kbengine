//	This file is part of the "jQuery.Syntax" project, and is distributed under the MIT License.
//	Copyright (c) 2011 Samuel G. D. Williams. <http://www.oriontransfer.co.nz>
//	See <jquery.syntax.js> for licensing details.

if (!RegExp.prototype.indexOf) {
	RegExp.indexOf = function (match, index) {
		return match[0].indexOf(match[index]) + match.index;
	};
}

if (!RegExp.prototype.escape) {
	RegExp.escape = function (pattern) {
		return pattern.replace(/[\-\[\]{}()*+?.\\\^$|,#\s]/g, "\\$&");
	};
}

if (!String.prototype.repeat) {
	String.prototype.repeat = function(l) {
		return new Array(l+1).join(this);
	};
}

// Return the inner text of an element - must preserve whitespace.
// Avoid returning \r characters.
Syntax.innerText = function(element) {
	var text;
	
	if (!element) {
		return "";
	}
	
	if (element.nodeName == 'BR') {
		return '\n';
	} else if (element.textContent) {
		// W3C: FF, Safari, Chrome, etc.
		text = element.textContent;
	} else if (document.body.innerText) {
		// IE, other older browsers.
		text = element.innerText;
	}
	
	return text.replace(/\r\n?/g, '\n');
}

// Convert to stack based implementation
Syntax.extractElementMatches = function (elems, offset, tabWidth) {
	var matches = [], current = [elems];
	offset = offset || 0;
	tabWidth = tabWidth || 4;
	
	(function (elems) {
		for (var i = 0; elems[i]; i++) {
			var text = null, elem = elems[i];
			
			if (elem.nodeType === 3 || elem.nodeType === 4) {
				offset += elem.nodeValue.length;
			
			} else if (elem.nodeType === 1) {
				var text = Syntax.innerText(elem);
				
				matches.push(new Syntax.Match(offset, text.length, {
					klass: elem.className,
					force: true,
					element: elem,
					allow: '*'
				}, text));
			}
			
			// Traverse everything, except comment nodes
			if (elem.nodeType !== 8 && elem.children) {
				arguments.callee(elem.childNodes, offset);
			}
		}
	})(elems);
	
	// Remove the top level element, since this will be recreated based on the supplied configuration.
	// Maybe there is a better way to achieve this?
	matches.shift();
	
	return matches;
}

// Basic layout doesn't do anything e.g. identity layout.
Syntax.layouts.preformatted = function (options, html, container) {
	return html;
};

Syntax.modeLineOptions = {
	'tab-width': function(name, value, options) { options.tabWidth = parseInt(value, 10); }
};

// Should be obvious right?
Syntax.convertTabsToSpaces = function (text, tabSize) {
	var space = [], pattern = /\r|\n|\t/g, tabOffset = 0, offsets = [], totalOffset = 0;
	tabSize = tabSize || 4
	
	for (var i = ""; i.length <= tabSize; i = i + " ") {
		space.push(i);
	}

	text = text.replace(pattern, function(match) {
		var offset = arguments[arguments.length - 2];
		if (match === "\r" || match === "\n") {
			tabOffset = -(offset + 1);
			return match;
		} else {
			var width = tabSize - ((tabOffset + offset) % tabSize);
			tabOffset += width - 1;
			
			// Any match after this offset has been shifted right by totalOffset
			totalOffset += width - 1
			offsets.push([offset, width, totalOffset]);
			
			return space[width];
		}
	});
	
	return {text: text, offsets: offsets};
};

// This function converts from a compressed set of offsets of the form:
//	[
//		[offset, width, totalOffset],
//		...
//	]
// This means that at a $offset, a tab (single character) was expanded to $width
// single space characters.
// This function produces a lookup table of offsets, where a given character offset
// is mapped to how far the character has been offset.
Syntax.convertToLinearOffsets = function (offsets, length) {
	var current = 0, changes = [];
	
	// Anything with offset after offset[current][0] but smaller than offset[current+1][0]
	// has been shifted right by offset[current][2]
	for (var i = 0; i < length; i++) {
		if (offsets[current] && i > offsets[current][0]) {
			// Is there a next offset?
			if (offsets[current+1]) {
				// Is the index less than the start of the next offset?
				if (i <= offsets[current+1][0]) {
					changes.push(offsets[current][2]);
				} else {
					// If so, move to the next offset.
					current += 1;
					i -= 1;
				}
			} else {
				// If there is no next offset we assume this one to the end.
				changes.push(offsets[current][2]);
			}
		} else {
			changes.push(changes[changes.length-1] || 0);
		}
	}
	
	return changes;
}

// Used for tab expansion process, by shifting matches when tab charaters were converted to
// spaces.
Syntax.updateMatchesWithOffsets = function (matches, linearOffsets, text) {
	(function (matches) {
		for (var i = 0; i < matches.length; i++) {
			var match = matches[i];
			
			// Calculate the new start and end points
			var offset = match.offset + linearOffsets[match.offset];
			var end = match.offset + match.length;
			end += linearOffsets[end];
			
			// Start, Length, Text
			match.adjust(linearOffsets[match.offset], end - offset, text);
			
			if (match.children.length > 0)
				arguments.callee(match.children);
		}
	})(matches);
	
	return matches;
};

// A helper function which automatically matches expressions with capture groups from the regular expression match.
// Each argument position corresponds to the same index regular expression group.
// Or, override by providing rule.index
Syntax.extractMatches = function() {
	var rules = arguments;
	
	return function(match, expr) {
		var matches = [];
		
		for (var i = 0; i < rules.length; i += 1) {
			var rule = rules[i], index = i+1;
			
			if (rule == null) {
				continue;
			}
			
			if (typeof(rule.index) != 'undefined') {
				index = rule.index;
			}
			
			if (rule.debug) {
				Syntax.log("extractMatches", rule, index, match[index], match);
			}
			
			if (match[index].length > 0) {
				if (rule.brush) {
					matches.push(Syntax.Brush.buildTree(rule, match[index], RegExp.indexOf(match, index)));
				} else {
					var expression = jQuery.extend({owner: expr.owner}, rule);
					
					matches.push(new Syntax.Match(RegExp.indexOf(match, index), match[index].length, expression, match[index]));
				}
			}
		}
		
		return matches;
	};
};

// Used to create processing functions that automatically link to remote documentation.
Syntax.lib.webLinkProcess = function (queryURI, lucky) {
	if (lucky) {
		queryURI = "http://www.google.com/search?btnI=I&q=" + encodeURIComponent(queryURI + " ");
	}
	
	return function (element, match, options) {
		// Per-code block linkification control.
		if (options.linkify === false)
			return element;
		
		var a = document.createElement('a');
		a.href = queryURI + encodeURIComponent(Syntax.innerText(element));
		a.className = element.className;
		
		// Move children from <element> to <a>
		while (element.childNodes.length > 0)
			a.appendChild(element.childNodes[0]);
		
		return a;
	};
};

// Global brush registration function.
Syntax.register = function (name, callback) {
	var brush = Syntax.brushes[name] = new Syntax.Brush();
	brush.klass = name;
	
	callback(brush);
};

// Library of helper patterns
Syntax.lib.cStyleComment = {pattern: /\/\*[\s\S]*?\*\//gm, klass: 'comment', allow: ['href']};
Syntax.lib.cppStyleComment = {pattern: /\/\/.*$/gm, klass: 'comment', allow: ['href']};
Syntax.lib.perlStyleComment = {pattern: /#.*$/gm, klass: 'comment', allow: ['href']};

Syntax.lib.perlStyleRegularExpression = {pattern: /\B\/([^\/]|\\\/)*?\/[a-z]*(?=\s*($|[^\w\s'"\(]))/gm, klass: 'constant', incremental: true};

Syntax.lib.cStyleFunction = {pattern: /([a-z_][a-z0-9_]*)\s*\(/gi, matches: Syntax.extractMatches({klass: 'function'})};
Syntax.lib.camelCaseType = {pattern: /\b_*[A-Z][\w]*\b/g, klass: 'type'};
Syntax.lib.cStyleType = {pattern: /\b[_a-z][_\w]*_t\b/gi, klass: 'type'};

Syntax.lib.xmlComment = {pattern: /(&lt;|<)!--[\s\S]*?--(&gt;|>)/gm, klass: 'comment'};
Syntax.lib.webLink = {pattern: /\w+:\/\/[\w\-.\/?%&=@:;#]*/g, klass: 'href'};

Syntax.lib.hexNumber = {pattern: /\b0x[0-9a-fA-F]+/g, klass: 'constant'};
Syntax.lib.decimalNumber = {pattern: /\b[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?/g, klass: 'constant'};

Syntax.lib.doubleQuotedString = {pattern: /"([^\\"\n]|\\.)*"/g, klass: 'string'};
Syntax.lib.singleQuotedString = {pattern: /'([^\\'\n]|\\.)*'/g, klass: 'string'};
Syntax.lib.multiLineDoubleQuotedString = {pattern: /"([^\\"]|\\.)*"/g, klass: 'string'};
Syntax.lib.multiLineSingleQuotedString = {pattern: /'([^\\']|\\.)*'/g, klass: 'string'};
Syntax.lib.stringEscape = {pattern: /\\./g, klass: 'escape', only: ['string']};

// Main match constructor. Make sure value is the correct size.
Syntax.Match = function (offset, length, expression, value) {
	this.offset = offset;
	this.endOffset = offset + length;
	this.length = length;
	this.expression = expression;
	this.value = value;
	this.children = [];
	this.parent = null;
	
	// When a node is bisected, this points to the next part.
	this.next = null;
};

// Shifts an entire tree forward or backwards.
Syntax.Match.prototype.shift = function (offset, text) {
	this.adjust(offset, null, text);
	
	for (var i = 0; i < this.children.length; i++) {
		this.children[i].shift(offset, text)
	}
};

// C the current match to have different offset and length.
Syntax.Match.prototype.adjust = function (offset, length, text) {
	this.offset += offset;
	this.endOffset += offset;
	
	if (length) {
		this.length = length;
		this.endOffset = this.offset + length;
	}
	
	if (text) {
		this.value = text.substr(this.offset, this.length);
	}
};

// Sort helper for sorting matches in forward order (e.g. same as the text that they were extracted from)
Syntax.Match.sort = function (a,b) {
	return (a.offset - b.offset) || (b.length - a.length);
};

// Is the given match contained in the range of the parent match?
Syntax.Match.prototype.contains = function (match) {
	return (match.offset >= this.offset) && (match.endOffset <= this.endOffset);
};

// Reduce a givent tree node into an html node.
Syntax.Match.defaultReduceCallback = function (node, container) {
	// We avoid using jQuery in this function since it is incredibly performance sensitive.
	// Using jQuery jQuery.fn.append() can reduce performance by as much as 1/3rd.
	if (typeof(node) === 'string') {
		node = document.createTextNode(node);
	}
	
	container.appendChild(node);
};

// Convert a tree of matches into some flat form (typically HTML nodes).
Syntax.Match.prototype.reduce = function (append, process) {
	var start = this.offset;
	var container = document.createElement('span');
	
	append = append || Syntax.Match.defaultReduceCallback;
	
	if (this.expression && this.expression.klass) {
		if (container.className.length > 0)
			container.className += ' ';
		
		container.className += this.expression.klass;
	}
	
	for (var i = 0; i < this.children.length; i += 1) {
		var child = this.children[i], end = child.offset;
		
		if (child.offset < this.offset) {
			Syntax.log("Syntax Warning: Offset of child", child, "is before offset of parent", this);
		}
		
		var text = this.value.substr(start - this.offset, end - start);
		
		append(text, container);
		append(child.reduce(append, process), container);
		
		start = child.endOffset;
	}
	
	if (start === this.offset) {
		append(this.value, container);
	} else if (start < this.endOffset) {
		append(this.value.substr(start - this.offset, this.endOffset - start), container);
	} else if (start > this.endOffset) {
		Syntax.log("Syntax Warning: Start position " + start + " exceeds end of value " + this.endOffset);
	}
	
	if (process) {
		container = process(container, this);
	}
	
	return container;
};

// Main nesting check - can a match contain the given match?
Syntax.Match.prototype.canContain = function (match) {
	// This is a special conditional for explicitly added ranges by the user.
	// Since user added it, we honour it no matter what.
	if (match.expression.force) {
		return true;
	}
	
	// Can't add anything into complete trees.
	if (this.complete) {
		return false;
	}
	
	// match.expression.only will be checked on insertion using this.canHaveChild(match)
	if (match.expression.only) {
		return true;
	}
	
	// If allow is undefined, default behaviour is no children.
	if (typeof(this.expression.allow) === 'undefined') {
		return false;
	}
	
	// false if {disallow: [..., klass, ...]}
	if (jQuery.isArray(this.expression.disallow) && jQuery.inArray(match.expression.klass, this.expression.disallow) !== -1) {
		return false;
	}
	
	// true if {allow: '*'}
	if (this.expression.allow === '*') {
		return true;
	}
	
	// true if {allow: [..., klass, ...]}
	if (jQuery.isArray(this.expression.allow) && jQuery.inArray(match.expression.klass, this.expression.allow) !== -1) {
		return true;
	}
	
	return false;
};

// Return true if the given match can be spliced in as a child.
// Checked automatically when calling _splice.
Syntax.Match.prototype.canHaveChild = function(match) {
	var only = match.expression.only;
	
	// This condition is fairly slow
	if (only) {
		var cur = this;
		
		while (cur !== null) {
			if (jQuery.inArray(cur.expression.klass, only) !== -1) {
				return true;
			}
			
			cur = cur.parent;
			
			// We don't traverse into other trees.
			if (cur && cur.complete) {
				break;
			}
		}
		
		return false;
	}
	
	return true;
};

// Add a child into the list of children for a given match, if it is acceptable to do so.
// Updates the owner of the match.
// Returns null if splice failed.
Syntax.Match.prototype._splice = function(i, match) {
	if (this.canHaveChild(match)) {
		this.children.splice(i, 0, match);
		match.parent = this;
		
		// For matches added using tags.
		if (!match.expression.owner) {
			match.expression.owner = this.expression.owner;
		}
		
		return this;
	} else {
		return null;
	}
};

// This function implements a full insertion procedure, and will break up the match to fit.
// This operation is potentially very expensive, but is used to insert custom ranges into
// the tree, if they are specified by the user. A custom <span> may cover multiple leafs in
// the tree, thus some parts of the tree may need to be split. This behavior is controlled
// by whole - if true, the tree is split, if false, the match is split.
// You should avoid using this function except in very specific cases.
Syntax.Match.prototype.insert = function(match, whole) {
	if (!this.contains(match))
		return null;
	
	if (whole) {
		var top = this, i = 0;
		while (i < top.children.length) {
			if (top.children[i].contains(match)) {
				top = top.children[i];
				i = 0;
			} else {
				i += 1;
			}
		}
		
		return top._insertWhole(match);
	} else {
		return this._insert(match);
	}
}

Syntax.Match.prototype._insertWhole = function(match) {
	var parts = this.bisectAtOffsets([match.offset, match.endOffset])
	this.children = [];
	
	if (parts[0]) {
		this.children = this.children.concat(parts[0].children);
	}
	
	if (parts[1]) {
		match.children = [];
		
		// Update the match's expression based on the current position in the tree:
		if (this.expression && this.expression.owner) {
			match.expression = this.expression.owner.getRuleForKlass(match.expression.klass) || match.expression;
		}
		
		// This probably isn't ideal, it would be better to convert all children and children-of-children
		// into a linear array and reinsert - it would be slightly more accurate in many cases.
		for (var i = 0; i < parts[1].children.length; i += 1) {
			var child = parts[1].children[i];
			
			if (match.canContain(child)) {
				match.children.push(child);
			}
		}
		
		this.children.push(match);
	}
	
	if (parts[2]) {
		this.children = this.children.concat(parts[2].children);
	}
	
	return this;
}

// This is not a general tree insertion function. It is optimised to run in almost constant
// time, but data must be inserted in sorted order, otherwise you will have problems.
// This function also ensures that matches won't be broken up unless absolutely necessary.
Syntax.Match.prototype.insertAtEnd = function(match) {
	if (!this.contains(match)) {
		Syntax.log("Syntax Error: Child is not contained in parent node!");
		return null;
	}
	
	if (!this.canContain(match)) {
		return null;
	}
	
	if (this.children.length > 0) {
		var i = this.children.length-1;
		var child = this.children[i];
		
		if (match.offset < child.offset) {
			// Displacement: Before or LHS Overlap
			// This means that the match has actually occurred before the last child.
			// This is a bit of an unusual situation because the matches SHOULD be in
			// sorted order.
			// However, we are sure that the match is contained in this node. This situation
			// sometimes occurs when sorting existing branches with matches that are supposed
			// to be within that branch. When we insert the match into the branch, there are
			// matches that technically should have been inserted afterwards.
			// Normal usage should avoid this case, and this is best for performance.
			if (match.force) {
				return this._insert(match);
			} else {
				return null;
			}
		} else if (match.offset < child.endOffset) {
			if (match.endOffset <= child.endOffset) { 
				// Displacement: Contains
				//console.log("displacement => contains");
				var result = child.insertAtEnd(match);
				return result;
			} else {
				// Displacement: RHS Overlap
				if (match.force) {
					return this._insert(match);
				} else {
					return null;
				}
			}
		} else {
			// Displacement: After
			return this._splice(i+1, match);
		}
		
		// Could not find a suitable placement: this is probably an error.
		return null;
	} else {
		// Displacement: Contains [but currently no children]
		return this._splice(0, match);
	}
};

// This insertion function is relatively complex because it is required to split the match over
// several children. This function is used infrequently and is mostly for completeness. However,
// it might be possible to remove it to reduce code.
Syntax.Match.prototype._insert = function(match) {
	if (this.children.length == 0)
		return this._splice(0, match);
	
	for (var i = 0; i < this.children.length; i += 1) {
		var child = this.children[i];
		
		// If the match ends before this child, it must be before it.
		if (match.endOffset <= child.offset)
			return this._splice(i, match);
		
		// If the match starts after this child, we continue.
		if (match.offset >= child.endOffset)
			continue;
		
		// There are four possibilities... 
		// ... with the possibility of overlapping children on the RHS.
		//           {------child------}   {---possibly some other child---}
		//   |----------complete overlap---------|
		//   |--lhs overlap--|
		//             |--contains--|
		//                       |--rhs overlap--|
		
		// First, the easiest case:
		if (child.contains(match)) {
			return child._insert(match);
		}
		
		// console.log("Bisect at offsets", match, child.offset, child.endOffset);
		var parts = match.bisectAtOffsets([child.offset, child.endOffset]);
		// console.log("parts =", parts);
		// We now have at most three parts
		//           {------child------}   {---possibly some other child---}
		//   |--[0]--|-------[1]-------|--[2]--|
		
		// console.log("parts", parts);
		
		if (parts[0]) {
			this._splice(i, parts[0])
		}
		
		if (parts[1]) {
			child.insert(parts[1])
		}
		
		// Continue insertion at this level with remainder.
		if (parts[2]) {
			match = parts[2]
		} else {
			return this;
		}
	}
	
	// If we got this far, the match wasn't [completely] inserted into the list of existing children, so it must be on the end.
	this._splice(this.children.length, match);
}

// This algorithm recursively bisects the tree at a given offset, but it does this efficiently by folding multiple bisections
// at a time.
// Splits:            /                /                   /
// Tree:       |-------------------------Top-------------------------|
//             |------------A--------------------|  |------C-------|
//                         |-------B----------|
// Step (1):
// Split Top into 4 parts:
//             |------/----------------/-------------------/---------|
// For each part, check if there are any children that cover this part.
// If there is a child, recursively call bisect with all splits.
// Step (1-1):
// Split A into parts:
//             |------/-----A----------/---------|
// For each part, check if there are any children that cover this part.
// If there is a child, recursively call bisect with all splits.
// Step (1-1-1):
// Split B into parts:
//                         |-------B---/------|
// No children covered by split. Return array of two parts, B1, B2.
// Step (1-2):
// Enumerate the results of splitting the child and merge piece-wise into own parts
//             |------/-----A----------/---------|
//                         |------B1---|--B2--|
// Finished merging children, return array of three parts, A1, A2, A3
// Step (2):
// Enumerate the results of splitting the child and merge piece-wise into own parts.
//             |------/----------------/-------------------/---------|
//             |--A1--|-------A2-------|----A3---|
//                         |------B1---|--B2--|
// Continue by splitting next child, C.
// Once all children have been split and merged, return all parts, T1, T2, T3, T4.
// The new tree:
//             |--T1--|-------T2-------|--------T3---------|---T4---|
//             |--A1--|-------A2-------|----A3---|  |--C1--|---C2--|
//                         |------B1---|--B2--|
//
// The new structure is as follows:
//		T1 <-	A1
//		T2 <-	A2	<- B1
//		T3 <-	A3	<-	B2
//		   \-	C1
//		T4 <- C2
//
Syntax.Match.prototype.bisectAtOffsets = function(splits) {
	var parts = [], start = this.offset, prev = null, children = jQuery.merge([], this.children);
	
	// Copy the array so we can modify it.
	splits = splits.slice(0);
	
	// We need to split including the last part.
	splits.push(this.endOffset);
	
	splits.sort(function (a,b) {
		return a-b;
	});
	
	// We build a set of top level matches by looking at each split point and
	// creating a new match from the end of the previous match to the split point.
	for (var i = 0; i < splits.length; i += 1) {
		var offset = splits[i];
		
		// The split offset is past the end of the match, so there are no more possible
		// splits.
		if (offset > this.endOffset) {
			break;
		}
		
		// We keep track of null parts if the offset is less than the start
		// so that things align up as expected with the requested splits.
		if (
			offset < this.offset // If the split point is less than the start of the match.
			|| (offset - start) == 0 // If the match would have effectively zero length.
		) {
			parts.push(null); // Preserve alignment with splits.
			start = offset;
			continue;
		}
		
		// Even if the previous split was out to the left, we align up the start
		// to be at the start of the match we are bisecting.
		if (start < this.offset)
			start = this.offset;
		
		var match = new Syntax.Match(start, offset - start, this.expression);
		match.value = this.value.substr(start - this.offset, match.length);
		
		if (prev) {
			prev.next = match;
		}
		
		prev = match;
		
		start = match.endOffset;
		parts.push(match);
	}
	
	// We only need to split to produce the number of parts we have.
	splits.length = parts.length;
	
	for (var i = 0; i < parts.length; i += 1) {
		if (parts[i] == null)
			continue;
		
		var offset = splits[0];
		
		while (children.length > 0) {
			if (children[0].endOffset <= parts[i].endOffset) {
				parts[i].children.push(children.shift());
			} else {
				break;
			}
		}
		
		if (children.length) {
			// We may have an intersection
			if (children[0].offset < parts[i].endOffset) {
				var children_parts = children.shift().bisectAtOffsets(splits), j = 0;
			
				// children_parts are the bisected children which need to be merged with parts
				// in a linear fashion
				for (; j < children_parts.length; j += 1) {
					if (children_parts[j] == null) continue; // Preserve alignment with splits.
					
					parts[i+j].children.push(children_parts[j]);
				}
				
				// Skip any parts which have been populated already
				// (i is incremented at the start of the loop, splits shifted at the end)
				i += (children_parts.length-2);
				splits.splice(0, children_parts.length-2);
			}
		}
		
		splits.shift();
	}
	
	if (children.length) {
		Syntax.log("Syntax Error: Children nodes not consumed", children.length, " remaining!");
	}
	
	return parts;
};

// Split a match at points in the tree that match a specific regular expression pattern.
// Uses the fast tree bisection algorithm, performance should be bounded O(S log N) where N is
// the total number of matches and S is the number of splits (?).
Syntax.Match.prototype.split = function(pattern) {
	var splits = [], match;
	
	while ((match = pattern.exec(this.value)) !== null) {
		splits.push(pattern.lastIndex);
	}
	
	var matches = this.bisectAtOffsets(splits);
	
	// Remove any null placeholders.
	return jQuery.grep(matches, function(n,i){
		return n;
	});
};

Syntax.Brush = function () {
	// The primary class of this brush. Must be unique.
	this.klass = null;
	
	// A sequential list of rules for extracting matches.
	this.rules = [];
	
	// A list of all parents that this brush derives from.
	this.parents = [];
	
	// A list of processes that may be run after extracting matches.
	this.processes = {};
};

// Add a parent to the brush. This brush should be loaded as a dependency.
Syntax.Brush.prototype.derives = function (name) {
	this.parents.push(name);
	this.rules.push({
		apply: function(text, expr) {
			return Syntax.brushes[name].getMatches(text);
		}
	});
}

// Return an array of all classes that the brush consists of.
// A derivied brush is its own klass + the klass of any and all parents.
Syntax.Brush.prototype.allKlasses = function () {
	var klasses = [this.klass];
	
	for (var i = 0; i < this.parents.length; i += 1) {
		klasses = klasses.concat(Syntax.brushes[this.parents[i]].allKlasses());
	}
	
	return klasses;
}

Syntax.Brush.convertStringToTokenPattern = function (pattern, escape) {
	var prefix = "\\b", postfix = "\\b";
	
	if (!pattern.match(/^\w/)) {
		if (!pattern.match(/\w$/)) {
			prefix = postfix = "";
		} else {
			prefix = "\\B";
		}
	} else {
		if (!pattern.match(/\w$/)) {
			postfix = "\\B";
		}
	}
	
	if (escape)
		pattern = RegExp.escape(pattern)
	
	return prefix + pattern + postfix;
}

Syntax.Brush.MatchPattern = function (text, rule) {
	if (!rule.pattern)
		return [];
	
	// Duplicate the pattern so that the function is reentrant.
	var matches = [], pattern = new RegExp;
	pattern.compile(rule.pattern);
	
	while((match = pattern.exec(text)) !== null) {
		if (rule.matches) {
			matches = matches.concat(rule.matches(match, rule));
		} else if (rule.brush) {
			matches.push(Syntax.Brush.buildTree(rule, match[0], match.index));
		} else {
			matches.push(new Syntax.Match(match.index, match[0].length, rule, match[0]));
		}
		
		if (rule.incremental) {
			// Don't start scanning from the end of the match..
			pattern.lastIndex = match.index + 1;
		}
	}
	
	return matches;
}

Syntax.Brush.prototype.push = function () {
	if (jQuery.isArray(arguments[0])) {
		var patterns = arguments[0], rule = arguments[1];
		
		var all = "(";
		
		for (var i = 0; i < patterns.length; i += 1) {
			if (i > 0) all += "|";
			
			var p = patterns[i];
			
			if (p instanceof RegExp) {
				all += p.source;
			} else {
				all += Syntax.Brush.convertStringToTokenPattern(p, true);
			}
		}
		
		all += ")";
		
		this.push(jQuery.extend({
			pattern: new RegExp(all, rule.options || 'g')
		}, rule));
	} else {
		var rule = arguments[0];
		
		if (typeof(rule.pattern) === 'string') {
			rule.string = rule.pattern;
			rule.pattern = new RegExp(Syntax.Brush.convertStringToTokenPattern(rule.string, true), rule.options || 'g')
		}

		if (typeof(XRegExp) !== 'undefined') {
			rule.pattern = new XRegExp(rule.pattern);
		}
		
		// Default pattern extraction algorithm
		rule.apply = rule.apply || Syntax.Brush.MatchPattern;

		if (rule.pattern && rule.pattern.global || typeof(rule.pattern) == 'undefined') {
			this.rules.push(jQuery.extend({owner: this}, rule));
		} else {
			Syntax.log("Syntax Error: Malformed rule: ", rule);
		}
	}
};

Syntax.Brush.prototype.getMatchesForRule = function (text, rule) {
	var matches = [], match = null;
	
	// Short circuit (user defined) function:
	if (typeof(rule.apply) != 'undefined') {
		matches = rule.apply(text, rule);
	}
	
	if (rule.debug) {
		Syntax.log("Syntax matches:", rule, text, matches);
	}
	
	return matches;
};

Syntax.Brush.prototype.getRuleForKlass = function (klass) {
	for (var i = 0; i < this.rules.length; i += 1) {
		if (this.rules[i].klass == klass) {
			return this.rules[i];
		}
	}
	
	return null;
}

// Get all matches from a given block of text.
Syntax.Brush.prototype.getMatches = function(text) {
	var matches = [];
	
	for (var i = 0; i < this.rules.length; i += 1) {
		matches = matches.concat(this.getMatchesForRule(text, this.rules[i]));
	}
	
	return matches;
};

// A helper function for building a tree from a specific rule.
// Typically used where sub-trees are required, e.g. CSS brush in HTML brush.
Syntax.Brush.buildTree = function(rule, text, offset, additionalMatches) {
	var match = Syntax.brushes[rule.brush].buildTree(text, offset, additionalMatches);
	
	jQuery.extend(match.expression, rule);
	
	return match;
}

// This function builds a tree from a given block of text.
// This is done by applying all rules to the text to get a complete list of matches,
// sorting them in order, and inserting them into a syntax tree data structure.
// Additional matches are forcefully inserted into the tree.
// Provide an offset if the text is offset in a larger block of text. Matches
// will be shifted along appropriately.
Syntax.Brush.prototype.buildTree = function(text, offset, additionalMatches) {
	offset = offset || 0;
	
	// Fixes code that uses \r\n for line endings. /$/ matches both \r\n, which is a problem..
	text = text.replace(/\r/g, '');
	
	var matches = this.getMatches(text);
	
	// Shift matches if offset is provided.
	if (offset && offset > 0) {
		for (var i = 0; i < matches.length; i += 1) {
			matches[i].shift(offset);
		}
	}
	
	var top = new Syntax.Match(offset, text.length, {klass: this.allKlasses().join(" "), allow: '*', owner: this}, text);

	// This sort is absolutely key to the functioning of the tree insertion algorithm.
	matches.sort(Syntax.Match.sort);

	for (var i = 0; i < matches.length; i += 1) {
		top.insertAtEnd(matches[i]);
	}
	
	if (additionalMatches) {
		for (var i = 0; i < additionalMatches.length; i += 1) {
			top.insert(additionalMatches[i], true);
		}
	}
	
	top.complete = true;
	
	return top;
};

// This function builds a syntax tree from the given text and matches (optional).
// The syntax tree is then flattened into html using a variety of functions.
//
// By default, you can't control reduction process through this function, but
// it is possible to control the element conversion process by replace
// .reduce(null, ...) with  .reduce(reduceCallback, ...)
// See Syntax.Match.defaultReduceCallback for more details about interface.
//
// Matches is optional, and provides a set of pre-existing matches to add
// to the tree.
// Options are passed to element level processing functions.
Syntax.Brush.prototype.process = function(text, matches, options) {
	var top = this.buildTree(text, 0, matches);
	
	var lines = top.split(/\n/g);
	
	var html = document.createElement('pre');
	html.className = 'syntax';
	
	for (var i = 0; i < lines.length; i += 1) {
		var line = lines[i].reduce(null, function (container, match) {
			if (match.expression) {
				if (match.expression.process) {
					container = match.expression.process(container, match, options);
				}
				
				if (match.expression.owner) {
					var process = match.expression.owner.processes[match.expression.klass];
					if (process) {
						container = process(container, match, options);
					}
				}
			}
			return container;
		});
		
		html.appendChild(line);
	}
	
	return html;
};

// Highlights a given block of text with a given set of options.
// options.brush should specify the brush to use, either by direct reference
// or name.
// Callback will be called with (highlighted_html, brush_used, original_text, options)
Syntax.highlightText = function(text, options, callback) {
	var brushName = (options.brush || 'plain').toLowerCase();
	
	brushName = Syntax.aliases[brushName] || brushName;
	
	Syntax.brushes.get(brushName, function(brush) {
		if (options.tabWidth) {
			// Calculate the tab expansion and offsets
			replacement = Syntax.convertTabsToSpaces(text, options.tabWidth);
			
			// Update any existing matches
			if (options.matches && options.matches.length) {
				var linearOffsets = Syntax.convertToLinearOffsets(replacement.offsets, text.length);
				options.matches = Syntax.updateMatchesWithOffsets(options.matches, linearOffsets, replacement.text);
			}
			
			text = replacement.text;
		}
		
		var html = brush.process(text, options.matches, options);
		
		if (options.linkify !== false) {
			jQuery('span.href', html).each(function(){
				jQuery(this).replaceWith(jQuery('<a>').attr('href', this.innerHTML).text(this.innerHTML));
			});
		}
		
		callback(html, brush, text, options);
	});
}

// Highlight a given set of elements with a set of options.
// Callback will be called once per element with (options, highlighted_html, original_container)
Syntax.highlight = function (elements, options, callback) {
	if (typeof(options) === 'function') {
		callback = options;
		options = {};
	}
	
	options.layout = options.layout || 'preformatted';
	options.matches = [];
	
	if (typeof(options.tabWidth) === 'undefined') {
		options.tabWidth = 4;
	}
	
	elements.each(function () {
		var container = jQuery(this);
		
		// We can augment the plain text to extract existing annotations (e.g. <span class="foo">...</span>).
		options.matches = options.matches.concat(Syntax.extractElementMatches(container));
		
		var text = Syntax.innerText(this);
		
		var match = text.match(/-\*- mode: (.+?);(.*?)-\*-/i);
		var endOfSecondLine = text.indexOf("\n", text.indexOf("\n") + 1);

		if (match && match.index < endOfSecondLine) {
			options.brush = options.brush || match[1];
			var modeline = match[2];

			var mode = /([a-z\-]+)\:(.*?)\;/gi;

			while((match = mode.exec(modeline)) !== null) {
				var setter = Syntax.modeLineOptions[match[1]];

				if (setter) {
					setter(match[1], match[2], options);
				}
			}
		}
		
		Syntax.highlightText(text, options, function(html, brush/*, text, options*/) {
			Syntax.layouts.get(options.layout, function(layout) {
				html = layout(options, $(html), $(container));

				// If there is a theme specified, ensure it is added to the top level class.
				if (options.theme) {
					// Load dependencies
					var themes = Syntax.themes[options.theme];
					for (var i = 0; i < themes.length; i += 1) {
						html.addClass("syntax-theme-" + themes[i]);
					}

					// Add the base theme
					html.addClass("syntax-theme-" + options.theme);
				}

				if (brush.postprocess) {
					html = brush.postprocess(options, html, container);
				}

				if (callback) {
					html = callback(options, html, container);
				}

				if (html && options.replace === true) {
					container.replaceWith(html);
				}
			});
		});
	});
};

// Register the file as being loaded
Syntax.loader.core = true;
