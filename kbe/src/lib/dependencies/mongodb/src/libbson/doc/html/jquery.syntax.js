/* 
	This file is part of the "jQuery.Syntax" project, and is distributed under the MIT License.
	For more information, please see http://www.oriontransfer.co.nz/software/jquery-syntax
	
	Copyright (c) 2011 Samuel G. D. Williams. <http://www.oriontransfer.co.nz>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

/*global Function: true, ResourceLoader: true, Syntax: true, alert: false, jQuery: true */

// ECMAScript 5! Why wasn't this done before!?
if (!Function.prototype.bind) {
	Function.prototype.bind = function (target) {
		var args = Array.prototype.slice.call(arguments, 1), fn = this;

		return function () {
			return fn.apply(target, args);
		};
	};
}

function ResourceLoader (loader) {
	this.dependencies = {};
	this.loading = {};
	this.loader = loader;
}

ResourceLoader.prototype._finish = function (name) {
	var deps = this.dependencies[name];
	
	if (deps) {
		// I'm not sure if this makes me want to cry... or laugh... or kill!?
		var chain = this._loaded.bind(this, name);
		
		for (var i = 0; i < deps.length; i += 1) {
			chain = this.get.bind(this, deps[i], chain);
		}
		
		chain();
	} else {
		this._loaded(name);
	}
};

ResourceLoader.prototype._loaded = function (name) {
	// When the script has been succesfully loaded, we expect the script
	// to register with this loader (i.e. this[name]).
	var resource = this[name], loading = this.loading[name];

	// Clear the loading list
	this.loading[name] = null;

	if (!resource) {
		alert("ResourceLoader: Could not load resource named " + name);
	} else {
		for (var i = 0; i < loading.length; i += 1) {
			loading[i](resource);
		}
	}
};

// This function must ensure that current cannot be completely loaded until next
// is completely loaded.
ResourceLoader.prototype.dependency = function (current, next) {
	// If the resource has completely loaded, then we don't need to queue it
	// as a dependency
	if (this[next] && !this.loading[name]) {
		return;
	}
	
	if (this.dependencies[current]) {
		this.dependencies[current].push(next);
	} else {
		this.dependencies[current] = [next];
	}
};

// This function must be reentrant for the same name and different callbacks.
ResourceLoader.prototype.get = function (name, callback) {
	if (this.loading[name]) {
		this.loading[name].push(callback)
	} else if (this[name]) {
		callback(this[name]);
	} else {
		this.loading[name] = [callback];
		this.loader(name, this._finish.bind(this, name));
	}
};

var Syntax = {
	root: null, 
	aliases: {},
	styles: {},
	themes: {},
	lib: {},
	defaultOptions: {
		cacheScripts: true,
		cacheStyleSheets: true,
		theme: "base"
	},
	
	brushes: new ResourceLoader(function (name, callback) {
		name = Syntax.aliases[name] || name;
		
		Syntax.getResource('jquery.syntax.brush', name, callback);
	}),
	
	layouts: new ResourceLoader(function (name, callback) {
		Syntax.getResource('jquery.syntax.layout', name, callback);
	}),
	
	loader: new ResourceLoader(function (name, callback) {
		Syntax.getResource('jquery.syntax', name, callback);
	}),
	
	getStyles: function (path) {
		var link = jQuery('<link>');
		jQuery("head").append(link);

		if (!Syntax.defaultOptions.cacheStyleSheets) {
			path = path + "?" + Math.random()
		}
		
		link.attr({
			rel: "stylesheet",
			type: "text/css",
			href: path
		});
	},
	
	getScript: function (path, callback) {
		var script = document.createElement('script');
		
		// Internet Exploder
		script.onreadystatechange = function() {
			if (this.onload && (this.readyState == 'loaded' || this.readyState == 'complete')) {
				this.onload();
				
				// Ensure the function is only called once.
				this.onload = null;
			}
		};
		
		// Every other modern browser
		script.onload = callback;
		script.type = "text/javascript";
		
		if (!Syntax.defaultOptions.cacheScripts)
			path = path + '?' + Math.random()
		
		script.src = path;
		
		document.getElementsByTagName('head')[0].appendChild(script);
	},
	
	getResource: function (prefix, name, callback) {
		var basename = prefix + "." + name;
		var styles = this.styles[basename];
		
		if (styles) {
			for (var i = 0; i < styles.length; i += 1) {
				this.getStyles(this.root + styles[i]);
			}
		}
		
		Syntax.getScript(this.root + basename + '.js', callback);
	},
	
	alias: function (name, aliases) {
		Syntax.aliases[name] = name;
		
		for (var i = 0; i < aliases.length; i += 1) {
			Syntax.aliases[aliases[i]] = name;
		}
	},
	
	brushAliases: function (brush) {
		var aliases = [];
		
		for (var name in Syntax.aliases) {
			if (Syntax.aliases[name] === brush) {
				aliases.push(name);
			}
		}
		
		return aliases;
	},
	
	brushNames: function () {
		var names = [];
		
		for (var name in Syntax.aliases) {
			if (name === Syntax.aliases[name]) {
				names.push(name);
			}
		}
		
		return names;
	},
	
	extractBrushName: function (className) {
		// brush names are by default lower case - normalize so we can detect it.
		className = className.toLowerCase();
		
		var match = className.match(/brush-([\S]+)/);
		
		if (match) {
			return match[1];
		} else {
			var classes = className.split(/ /);
			
			if (jQuery.inArray("syntax", classes) !== -1) {
				for (var i = 0; i < classes.length; i += 1) {
					var name = Syntax.aliases[classes[i]];
					
					if (name) {
						return name;
					}
				}
			}
		}
		
		return null;
	},
	
	detectRoot: function () {
		if (Syntax.root == null) {
			// Initialize root based on current script path.
			var scripts = jQuery('script').filter(function(){
				return this.src.match(/jquery\.syntax/);
			});

			var first = scripts.get(0);

			if (first) {
				// Calculate the basename for the given script src.
				var root = first.src.match(/.*\//);

				if (root) {
					Syntax.root = root[0];
				}
			}
		}
	},
	
	log: function() {
		if (typeof(console) != "undefined" && console.log) {
			console.log.apply(console, arguments);
		} else if (window.console && window.console.log) {
			window.console.log.apply(window.console, arguments);
		}
	}
};

jQuery.fn.syntax = function (options, callback) {
	Syntax.detectRoot();
	
	var elements = this;
	
	Syntax.loader.get('core', function () {
		Syntax.highlight(elements, options, callback);
	});
};

jQuery.syntax = function (options, callback) {
	options = options || {};
	var context = options.context;
	
	if (options.root) {
		Syntax.root = options.root;
	} else {
		Syntax.detectRoot();
	}
	
	options = jQuery.extend(Syntax.defaultOptions, options)
	
	options.blockSelector = options.blockSelector || 'pre.syntax:not(.highlighted)';
	options.inlineSelector = options.inlineSelector || 'code.syntax:not(.highlighted)';
	
	options.blockLayout = options.blockLayout || 'list';
	options.inlineLayout = options.inlineLayout || 'inline';
	
	// Allow the user to specify callbacks without replacement.
	if (typeof options.replace == "undefined")
		options.replace = true;
	
	jQuery(options.blockSelector, context).each(function () {
		jQuery(this).syntax(jQuery.extend({}, options, {
			brush: Syntax.extractBrushName(this.className),
			layout: options.blockLayout
		}), callback);
	});
	
	jQuery(options.inlineSelector, context).each(function () {
		jQuery(this).syntax(jQuery.extend({}, options, {
			brush: Syntax.extractBrushName(this.className),
			layout: options.inlineLayout
		}), callback);
	});
};
