// brush: "clang" aliases: ["cpp", "c++", "c", "objective-c"]

//	This file is part of the "jQuery.Syntax" project, and is distributed under the MIT License.
//	Copyright (c) 2011 Samuel G. D. Williams. <http://www.oriontransfer.co.nz>
//	See <jquery.syntax.js> for licensing details.

Syntax.register('clang', function(brush) {
	var keywords = ["@interface", "@implementation", "@protocol", "@end", "@try", "@throw", "@catch", "@finally", "@class", "@selector", "@encode", "@synchronized", "@property", "@synthesize", "@dynamic", "struct", "break", "continue", "else", "for", "switch", "case", "default", "enum", "goto", "register", "sizeof", "typedef", "volatile", "do", "extern", "if", "return", "static", "union", "while", "asm", "dynamic_cast", "namespace", "reinterpret_cast", "try", "explicit", "static_cast", "typeid", "catch", "operator", "template", "class", "const_cast", "inline", "throw", "virtual", "IBOutlet"];
	
	var access = ["@private", "@protected", "@public", "@required", "@optional", "private", "protected", "public", "friend", "using"];
	
	var typeModifiers = ["mutable", "auto", "const", "register", "typename", "abstract"];
	var types = ["double", "float", "int", "short", "char", "long", "signed", "unsigned", "bool", "void", "id"];
	
	var operators = ["+", "*", "/", "-", "&", "|", "~", "!", "%", "<", "=", ">", "[", "]", "new", "delete", "in"];
	
	var values = ["this", "true", "false", "NULL", "YES", "NO", "nil"];
	
	brush.push(values, {klass: 'constant'});
	brush.push(typeModifiers, {klass: 'keyword'})
	brush.push(types, {klass: 'type'});
	brush.push(keywords, {klass: 'keyword'});
	brush.push(operators, {klass: 'operator'});
	brush.push(access, {klass: 'access'});
	
	// Objective-C properties
	brush.push({
		pattern: /@property\((.*)\)[^;]+;/gmi,
		klass: 'objective-c-property',
		allow: '*'
	});
	
	var propertyAttributes = ["getter", "setter", "readwrite", "readonly", "assign", "retain", "copy", "nonatomic"];
	
	brush.push(propertyAttributes, {
		klass: 'keyword',
		only: ['objective-c-property']
	});
	
	// Objective-C strings
	
	brush.push({
		pattern: /@(?=")/g,
		klass: 'string'
	});
	
	// Objective-C classes, C++ classes, C types, etc.
	brush.push(Syntax.lib.camelCaseType);
	brush.push(Syntax.lib.cStyleType);
	brush.push({
		pattern: /(?:class|struct|enum|namespace)\s+([^{;\s]+)/gmi,
		matches: Syntax.extractMatches({klass: 'type'})
	});
	
	brush.push({
		pattern: /#.*$/gmi,
		klass: 'preprocessor',
		allow: ['string']
	});
	
	brush.push(Syntax.lib.cStyleComment);
	brush.push(Syntax.lib.cppStyleComment);
	brush.push(Syntax.lib.webLink);
	
	// Objective-C style functions
	brush.push({pattern: /\w+:(?=.*(\]|;|\{))(?!:)/g, klass: 'function'});
	
	brush.push({
		pattern: /[^:\[]\s+(\w+)(?=\])/g,
		matches: Syntax.extractMatches({klass: 'function'})
	});
	
	brush.push({
		pattern: /-\s*(\([^\)]+?\))?\s*(\w+)\s*\{/g,
		matches: Syntax.extractMatches({index: 2, klass: 'function'})
	});
	
	// Strings
	brush.push(Syntax.lib.singleQuotedString);
	brush.push(Syntax.lib.doubleQuotedString);
	brush.push(Syntax.lib.stringEscape);
	
	// Numbers
	brush.push(Syntax.lib.decimalNumber);
	brush.push(Syntax.lib.hexNumber);
	
	brush.push(Syntax.lib.cStyleFunction);
});

